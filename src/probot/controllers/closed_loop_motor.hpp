#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <probot/core/scheduler.hpp>
#include <probot/controllers/pid.hpp>
#include <probot/controllers/motor_controller.hpp>
#include <probot/sensors/encoder.hpp>

namespace probot::controllers {
  class ClosedLoopMotor : public IMotorController {
  public:
    ClosedLoopMotor(sensors::IEncoder* encoder,
                    control::PID* pid,
                    motor::IMotorDriver* driver,
                    float vel_ticks_per_s_to_units = 1.0f,
                    float pos_ticks_to_units = 1.0f)
    : encoder_(encoder), pid_(pid), driver_(driver),
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
        slot_cfg_[i] = {0.0f, 0.0f, 0.0f, -1.0f, 1.0f};
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

    void setTimeoutMs(uint32_t ms) override { timeout_ms_ = ms; }

    bool setPowerDirect(float power){
      if (!driver_) return false;
      return driver_->setPower(power, owner_token_);
    }

    // IMotor interface (external ownership control). External owner must claim/release.
    bool claim(void* owner) override {
      if (external_owner_ && external_owner_ != owner) return false;
      if (!driver_) return false;
      if (!driver_->claim(owner)) return false;
      external_owner_ = owner;
      return true;
    }
    void release(void* owner) override {
      if (!driver_ || external_owner_ != owner) return;
      driver_->release(owner);
      external_owner_ = nullptr;
    }
    bool setPower(float power, void* owner) override {
      if (!driver_ || external_owner_ != owner) return false;
      float p = inverted_ ? -power : power;
      return driver_->setPower(p, owner);
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

      if (timeout_ms_ > 0 && (now_ms - last_ref_ms_) > timeout_ms_){
        driver_->setPower(0.0f, owner_token_);
        return;
      }

      if (active_mode_ == ControlType::kPercent){
        driver_->setPower(ref_value_, owner_token_);
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

      driver_->setPower(cmd, owner_token_);

#ifndef PROBOT_CLM_NOLOG
      Serial.printf("[CLM ] mode=%s ref=%.3f meas=%.3f err=%.3f out=%.3f slot=%d\n",
                    (active_mode_==ControlType::kVelocity?"VEL":"POS"),
                    ref, meas, error, cmd, slot);
#endif
    }

  private:
    static int clampSlot(int s){ return s < 0 ? 0 : (s > 3 ? 3 : s); }

    sensors::IEncoder* encoder_;
    control::PID*     pid_;
    motor::IMotorDriver* driver_;

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
