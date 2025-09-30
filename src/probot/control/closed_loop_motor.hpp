#pragma once
#include <stdint.h>
#include <algorithm>
#include <memory>
#include <atomic>
#include <Arduino.h>
#include <math.h>
#include <probot/core/scheduler.hpp>
#include <probot/control/pid.hpp>
#include <probot/control/imotor_controller.hpp>
#include <probot/control/motion_profile/trapezoid_profile.hpp>
// #include <probot/control/motion_profile/s_curve_profile.hpp>  // Disabled for now
#include <probot/sensors/encoder.hpp>

namespace probot::control {
  class ClosedLoopMotor : public IMotorController {
  public:
    ClosedLoopMotor(sensors::IEncoder* encoder,
                    probot::control::PID* pid,
                    motor::IMotorDriver* driver,
                    float vel_ticks_per_s_to_units = 1.0f,
                    float pos_ticks_to_units = 1.0f)
    : encoder_(encoder), pid_(pid), driver_(driver),
      vel_ticks_to_units_(vel_ticks_per_s_to_units),
      pos_ticks_to_units_(pos_ticks_to_units),
      ref_value_(0.0f), target_value_(0.0f), last_ref_ms_(0), timeout_ms_(500),
      active_mode_(ControlType::kVelocity),
      default_slot_velocity_(0),
      default_slot_position_(1),
      owner_token_(this),
      external_owner_(nullptr),
      inverted_(false)
    {
      for (int i=0;i<4;i++){
        slot_cfg_[i] = {0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f};
      }
      if (driver_) driver_->claim(owner_token_);
    }

    ~ClosedLoopMotor(){
      if (driver_) {
        driver_->setPower(0.0f, owner_token_);
        driver_->release(owner_token_);
      }
    }

    void setSetpoint(float value, ControlType mode, int slot = -1) override {
      bool mode_changed = (mode != active_mode_);
      target_value_ = value;
      last_ref_ms_.store(millis());
      if (mode_changed) {
        pid_->reset();
        selected_slot_override_ = -1;
      }
      active_mode_ = mode;
      if (slot >= 0) selected_slot_override_ = clampSlot(slot); else selected_slot_override_ = -1;

      bool profileAllowed = (motion_profile_type_ != probot::control::MotionProfileType::kNone) &&
                            (mode == ControlType::kPosition || mode == ControlType::kVelocity) &&
                            profileConstraintsValid(mode);
      if (mode == ControlType::kPercent) {
        profile_active_ = false;
        profile_pending_ = false;
        trapezoid_profile_.reset();
        // scurve_profile_.reset();
        ref_value_ = value;
      } else if (profileAllowed) {
        profile_pending_ = true;
        profile_active_ = false;
        trapezoid_profile_.reset();
        // scurve_profile_.reset();
      } else {
        profile_pending_ = false;
        profile_active_ = false;
        trapezoid_profile_.reset();
        // scurve_profile_.reset();
        ref_value_ = value;
      }
    }

    void setPidSlotConfig(int slot, const probot::control::PidConfig& cfg) override {
      slot = clampSlot(slot);
      slot_cfg_[slot] = cfg;
      pid_->reset();
    }

    void selectDefaultSlot(ControlType mode, int slot) override {
      slot = clampSlot(slot);
      if (mode == ControlType::kVelocity) default_slot_velocity_ = slot;
      else                                 default_slot_position_ = slot;
      pid_->reset();
    }

    int defaultSlot(ControlType mode) const override {
      return (mode == ControlType::kVelocity) ? default_slot_velocity_ : default_slot_position_;
    }

    float lastSetpoint() const override { return target_value_; }
    float lastMeasurement() const override { return last_measurement_; }
    float lastOutput() const override { return last_output_; }
    ControlType activeMode() const override { return active_mode_; }
    bool isAtTarget(float tolerance) const override {
      return fabsf(target_value_ - last_measurement_) <= tolerance;
    }

    void setTimeoutMs(uint32_t ms) override { timeout_ms_ = ms; }

    void setMotionProfile(probot::control::MotionProfileType type) override {
      motion_profile_type_ = type;
      profile_active_ = false;
      profile_pending_ = false;
      profile_elapsed_ = 0.0f;
      trapezoid_profile_.reset();
      // scurve_profile_.reset();
    }
    probot::control::MotionProfileType motionProfile() const override {
      return motion_profile_type_;
    }
    void setMotionProfileConfig(const probot::control::MotionProfileConfig& cfg) override {
      motion_profile_cfg_ = cfg;
      profile_active_ = false;
      profile_pending_ = false;
      profile_elapsed_ = 0.0f;
      trapezoid_profile_.reset();
      // scurve_profile_.reset();
    }
    probot::control::MotionProfileConfig motionProfileConfig() const override {
      return motion_profile_cfg_;
    }

    bool setPowerDirect(float power){
      if (!driver_) return false;
      if (external_owner_) return false;  // Respect external ownership
      return driver_->setPower(power, owner_token_);
    }

    // IMotor interface (external ownership control). External owner must claim/release.
    bool claim(void* owner) override {
      if (external_owner_ && external_owner_ != owner) return false;
      if (!driver_) return false;
      external_owner_ = owner;
      return true;
    }
    void release(void* owner) override {
      if (!driver_ || external_owner_ != owner) return;
      driver_->setPower(0.0f, owner_token_);
      external_owner_ = nullptr;
    }
    bool setPower(float power, void* owner) override {
      if (!driver_ || external_owner_ != owner) return false;
      float p = inverted_ ? -power : power;
      return driver_->setPower(p, owner_token_);
    }
    bool isClaimed() const override { return external_owner_ != nullptr; }
    void* currentOwner() const override { return external_owner_; }

    void setInverted(bool inverted) override {
      inverted_ = inverted;
      if (driver_) driver_->setInverted(inverted);
    }
    bool getInverted() const override { return inverted_; }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (!encoder_ || !pid_ || !driver_) return;

      if (timeout_ms_ > 0 && (now_ms - last_ref_ms_.load()) > timeout_ms_){
        driver_->setPower(0.0f, owner_token_);
        last_output_ = 0.0f;
        return;
      }

      if (active_mode_ == ControlType::kPercent){
        float target_val = target_value_.load();
        float output = inverted_ ? -target_val : target_val;
        ref_value_.store(target_val);
        driver_->setPower(output, owner_token_);
        last_measurement_ = target_val;
        last_output_ = output;
        return;
      }

      int slot = (selected_slot_override_ >= 0)
               ? selected_slot_override_
               : (active_mode_ == ControlType::kVelocity ? default_slot_velocity_ : default_slot_position_);
      pid_->setConfig(slot_cfg_[slot]);

      float ref = ref_value_.load();
      float meas = 0.0f;
      if (active_mode_ == ControlType::kVelocity){
        int32_t tps = encoder_->readTicksPerSecond();
        meas = vel_ticks_to_units_ * static_cast<float>(tps);
      } else {
        int32_t ticks = encoder_->readTicks();
        meas = pos_ticks_to_units_ * static_cast<float>(ticks);
      }

      float dt_s = dt_ms * 0.001f;
      if (dt_s <= 0.0f) dt_s = 0.001f;

      if (motion_profile_type_ != probot::control::MotionProfileType::kNone &&
          (active_mode_ == ControlType::kVelocity || active_mode_ == ControlType::kPosition) &&
          profileConstraintsValid(active_mode_)){
        if (profile_pending_){
          initializeProfile(meas);
        }
        if (profile_active_){
          ref = sampleProfile(meas, dt_s);
        } else {
          ref = target_value_.load();
        }
      } else {
        profile_active_ = false;
        profile_pending_ = false;
        ref = target_value_.load();
      }

      ref_value_.store(ref);

      float error = ref - meas;
      float pid_out = pid_->step(error, dt_s);
      float ff = slot_cfg_[slot].kf * ref;
      float cmd = pid_out + ff;
      cmd = std::clamp(cmd, slot_cfg_[slot].out_min, slot_cfg_[slot].out_max);

      driver_->setPower(cmd, owner_token_);
      last_measurement_ = meas;
      last_output_ = cmd;

#ifndef PROBOT_CLM_NOLOG
      Serial.printf("[CLM ] mode=%s ref=%.3f meas=%.3f err=%.3f out=%.3f slot=%d\n",
                    (active_mode_==ControlType::kVelocity?"VEL":"POS"),
                    ref, meas, error, cmd, slot);
#endif
    }

  private:
    static int clampSlot(int s){ return s < 0 ? 0 : (s > 3 ? 3 : s); }

    bool profileConstraintsValid(ControlType mode) const {
      if (motion_profile_type_ == probot::control::MotionProfileType::kNone) return false;
      if (mode == ControlType::kPosition){
        return motion_profile_cfg_.maxVelocity > 0.0f && motion_profile_cfg_.maxAcceleration > 0.0f;
      }
      if (mode == ControlType::kVelocity){
        return motion_profile_cfg_.maxAcceleration > 0.0f;
      }
      return false;
    }

    void initializeProfile(float currentMeasurement){
      profile_pending_ = false;
      profile_elapsed_ = 0.0f;
      profile_mode_ = active_mode_;
      profile_active_ = false;

      if (profile_mode_ == ControlType::kPosition){
        float maxVel = motion_profile_cfg_.maxVelocity;
        float maxAcc = motion_profile_cfg_.maxAcceleration;
        if (maxVel <= 0.0f || maxAcc <= 0.0f){
          trapezoid_profile_.reset();
          // scurve_profile_.reset();
          return;
        }
        if (motion_profile_type_ == probot::control::MotionProfileType::kTrapezoid){
          probot::control::motion_profile::TrapezoidProfile::Constraints constraints(maxVel, maxAcc);
          probot::control::motion_profile::TrapezoidProfile::State goal(target_value_.load(), 0.0f);
          probot::control::motion_profile::TrapezoidProfile::State initial(currentMeasurement, 0.0f);
          trapezoid_profile_ = std::make_unique<probot::control::motion_profile::TrapezoidProfile>(constraints, goal, initial);
          // scurve_profile_.reset();
        }
        // S-Curve support disabled (high memory usage)
        // else {
        //   float maxJ = motion_profile_cfg_.maxJerk;
        //   if (maxJ <= 0.0f){
        //     trapezoid_profile_.reset();
        //     scurve_profile_.reset();
        //     return;
        //   }
        //   probot::control::motion_profile::SCurveProfile::Constraints constraints(maxVel, maxAcc, maxJ);
        //   probot::control::motion_profile::SCurveProfile::State goal(target_value_.load(), 0.0f, 0.0f);
        //   probot::control::motion_profile::SCurveProfile::State initial(currentMeasurement, 0.0f, 0.0f);
        //   scurve_profile_ = std::make_unique<probot::control::motion_profile::SCurveProfile>(constraints, goal, initial);
        //   trapezoid_profile_.reset();
        // }
        profile_active_ = true;
      } else if (profile_mode_ == ControlType::kVelocity){
        if (motion_profile_cfg_.maxAcceleration <= 0.0f){
          profile_active_ = false;
        } else {
          profile_active_ = true;
        }
        trapezoid_profile_.reset();
        // scurve_profile_.reset();
      }
    }

    float sampleProfile(float currentMeasurement, float dt_s){
      (void)currentMeasurement;
      float ref = target_value_.load();
      profile_elapsed_ += dt_s;

      if (profile_mode_ == ControlType::kPosition){
        if (motion_profile_type_ == probot::control::MotionProfileType::kTrapezoid && trapezoid_profile_){
          auto state = trapezoid_profile_->calculate(profile_elapsed_);
          ref = state.position;
          if (profile_elapsed_ >= trapezoid_profile_->totalTime()){
            ref = target_value_.load();
            profile_active_ = false;
            trapezoid_profile_.reset();
          }
        }
        // S-Curve support disabled
        // else if (motion_profile_type_ == probot::control::MotionProfileType::kSCurve && scurve_profile_){
        //   auto state = scurve_profile_->calculate(profile_elapsed_);
        //   ref = state.position;
        //   if (profile_elapsed_ >= scurve_profile_->totalTime()){
        //     ref = target_value_.load();
        //     profile_active_ = false;
        //     scurve_profile_.reset();
        //   }
        // }
        else {
          ref = target_value_.load();
          profile_active_ = false;
          trapezoid_profile_.reset();
          // scurve_profile_.reset();
        }
      } else if (profile_mode_ == ControlType::kVelocity){
        float maxAcc = motion_profile_cfg_.maxAcceleration;
        float target_val = target_value_.load();
        if (maxAcc <= 0.0f){
          ref = target_val;
          profile_active_ = false;
        } else {
          float lastRef = ref_value_.load();
          float maxDelta = maxAcc * dt_s;
          if (maxDelta <= 0.0f) maxDelta = maxAcc * 0.001f;
          float delta = target_val - lastRef;
          if (delta > maxDelta) delta = maxDelta;
          else if (delta < -maxDelta) delta = -maxDelta;
          ref = lastRef + delta;
          float maxVel = motion_profile_cfg_.maxVelocity;
          if (maxVel > 0.0f){
            if (ref > maxVel) ref = maxVel;
            if (ref < -maxVel) ref = -maxVel;
          }
          if (fabsf(ref - target_val) <= 1e-4f){
            ref = target_val;
            profile_active_ = false;
          }
        }
      } else {
        ref = target_value_.load();
        profile_active_ = false;
      }

      return ref;
    }

    sensors::IEncoder* encoder_;
    probot::control::PID*     pid_;
    motor::IMotorDriver* driver_;

    float    vel_ticks_to_units_;
    float    pos_ticks_to_units_;
    std::atomic<float> ref_value_;
    std::atomic<float> target_value_;
    std::atomic<uint32_t> last_ref_ms_;
    uint32_t timeout_ms_;

    ControlType active_mode_;
    int         default_slot_velocity_;
    int         default_slot_position_;
    int         selected_slot_override_ = -1;
    probot::control::PidConfig slot_cfg_[4];

    probot::control::MotionProfileType motion_profile_type_{probot::control::MotionProfileType::kNone};
    probot::control::MotionProfileConfig motion_profile_cfg_{};
    bool profile_pending_ = false;
    bool profile_active_ = false;
    float profile_elapsed_ = 0.0f;
    ControlType profile_mode_ = ControlType::kPercent;
    std::unique_ptr<probot::control::motion_profile::TrapezoidProfile> trapezoid_profile_;
    // std::unique_ptr<probot::control::motion_profile::SCurveProfile> scurve_profile_;  // Disabled

    void* owner_token_;
    void* external_owner_;
    bool  inverted_;
    float last_measurement_ = 0.0f;
    float last_output_      = 0.0f;
  };
} // namespace probot::control 
