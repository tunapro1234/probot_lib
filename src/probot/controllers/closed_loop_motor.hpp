#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <probot/core/scheduler.hpp>
#include <probot/controllers/pid.hpp>
#include <probot/sensors/encoder.hpp>
#include <probot/devices/motors/motor.hpp>

namespace probot::controllers {
  enum class ControlType { kVelocity = 0, kPosition = 1 };

  class ClosedLoopMotor : public ::control::IUpdatable, public motor::IMotor {
  public:
    ClosedLoopMotor(sensors::IEncoder* encoder,
                    control::PID* pid,
                    motor::IMotor* motor,
                    float vel_ticks_per_s_to_units = 1.0f,
                    float pos_ticks_to_units = 1.0f)
    : encoder_(encoder), pid_(pid), motor_(motor),
      vel_ticks_to_units_(vel_ticks_per_s_to_units),
      pos_ticks_to_units_(pos_ticks_to_units),
      ref_value_(0.0f), last_ref_ms_(0), timeout_ms_(500),
      active_mode_(ControlType::kVelocity),
      default_slot_velocity_(0),
      default_slot_position_(1),
      owner_token_(this),
      external_owner_(nullptr),
      inverted_(false)
    {
      for (int i=0;i<4;i++){
        slot_cfg_[i] = {0,0,0,-1000,1000};
      }
      if (motor_) motor_->claim(owner_token_);
    }

    ~ClosedLoopMotor(){
      if (motor_) {
        motor_->setPower(0, owner_token_);
        motor_->release(owner_token_);
      }
    }

    void setSetpoint(float value, ControlType mode, int slot = -1){
      ref_value_ = value;
      last_ref_ms_ = millis();
      active_mode_ = mode;
      if (slot >= 0) selected_slot_override_ = clampSlot(slot); else selected_slot_override_ = -1;
    }

    void setPidSlotConfig(int slot, const control::PidConfig& cfg){
      slot = clampSlot(slot);
      slot_cfg_[slot] = cfg;
    }

    void selectDefaultSlot(ControlType mode, int slot){
      slot = clampSlot(slot);
      if (mode == ControlType::kVelocity) default_slot_velocity_ = slot;
      else                                 default_slot_position_ = slot;
      pid_->reset();
    }

    void setTimeoutMs(uint32_t ms){ timeout_ms_ = ms; }

    bool setPowerDirect(int16_t power){
      if (!motor_) return false;
      return motor_->setPower(power, owner_token_);
    }

    // IMotor interface (external ownership control). External owner must claim/release.
    bool claim(void* owner) override {
      if (external_owner_ && external_owner_ != owner) return false;
      if (!motor_) return false;
      if (!motor_->claim(owner)) return false;
      external_owner_ = owner;
      return true;
    }
    void release(void* owner) override {
      if (!motor_ || external_owner_ != owner) return;
      motor_->release(owner);
      external_owner_ = nullptr;
    }
    bool setPower(int16_t power, void* owner) override {
      if (!motor_ || external_owner_ != owner) return false;
      int16_t p = inverted_ ? (int16_t)-power : power;
      return motor_->setPower(p, owner);
    }
    bool isClaimed() const override { return external_owner_ != nullptr; }
    void* currentOwner() const override { return external_owner_; }

    void setInverted(bool inverted) override {
      inverted_ = inverted;
      if (motor_) motor_->setInverted(inverted);
    }
    bool getInverted() const override { return inverted_; }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (!encoder_ || !pid_ || !motor_) return;

      if (timeout_ms_ > 0 && (now_ms - last_ref_ms_) > timeout_ms_){
        motor_->setPower(0, owner_token_);
        return;
      }

      int slot = (selected_slot_override_ >= 0)
               ? selected_slot_override_
               : (active_mode_ == ControlType::kVelocity ? default_slot_velocity_ : default_slot_position_);
      pid_->setConfig(slot_cfg_[slot]);

      float ref = ref_value_;
      float meas = 0.0f;
      if (active_mode_ == ControlType::kVelocity){
        int32_t tps = encoder_->readTicksPerSecond();
        meas = vel_ticks_to_units_ * static_cast<float>(tps);
      } else {
        int32_t ticks = encoder_->readTicks();
        meas = pos_ticks_to_units_ * static_cast<float>(ticks);
      }

      float error = ref - meas;
      float dt_s = dt_ms * 0.001f;
      float cmd = pid_->step(error, dt_s);

      int16_t pwm = static_cast<int16_t>(cmd);
      motor_->setPower(pwm, owner_token_);

#ifndef PROBOT_CLM_NOLOG
      Serial.printf("[CLM ] mode=%s ref=%.2f meas=%.2f err=%.2f pwm=%d slot=%d\n",
                    (active_mode_==ControlType::kVelocity?"VEL":"POS"),
                    ref, meas, error, (int)pwm, slot);
#endif
    }

  private:
    static int clampSlot(int s){ return s < 0 ? 0 : (s > 3 ? 3 : s); }

    sensors::IEncoder* encoder_;
    control::PID*     pid_;
    motor::IMotor*    motor_;

    float    vel_ticks_to_units_;
    float    pos_ticks_to_units_;
    volatile float ref_value_;
    uint32_t last_ref_ms_;
    uint32_t timeout_ms_;

    ControlType active_mode_;
    int         default_slot_velocity_;
    int         default_slot_position_;
    int         selected_slot_override_ = -1;
    control::PidConfig slot_cfg_[4];

    void* owner_token_;
    void* external_owner_;
    bool  inverted_;
  };
} // namespace probot::controllers 