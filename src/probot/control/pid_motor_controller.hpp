#pragma once
#include <stdint.h>
#include <algorithm>
#include <atomic>
#include <Arduino.h>
#include <math.h>
#include <probot/control/control_types.hpp>
#include <probot/control/pid.hpp>
#include <probot/devices/motors/imotor_controller.hpp>
#include <probot/devices/sensors/encoder.hpp>

namespace probot::control {
  class PidMotorController : public motor::IMotorController {
  public:
    PidMotorController(sensors::IEncoder* encoder,
                       motor::IMotorController* driver,
                       float vel_ticks_per_s_to_units = 1.0f,
                       float pos_ticks_to_units = 1.0f)
    : encoder_(encoder),
      driver_(driver),
      vel_ticks_to_units_(vel_ticks_per_s_to_units),
      pos_ticks_to_units_(pos_ticks_to_units),
      velocity_cfg_(defaultPidConfig()),
      position_cfg_(defaultPidConfig()),
      velocity_pid_(velocity_cfg_),
      position_pid_(position_cfg_),
      target_velocity_(0.0f),
      target_position_(0.0f),
      target_power_(0.0f),
      last_ref_ms_(0),
      timeout_ms_(500),
      active_mode_(ControlType::kPercent),
      inverted_(false)
    {
    }

    ~PidMotorController(){
      if (driver_) {
        driver_->setPower(0.0f);
      }
    }

    void setVelocityPidConfig(const probot::control::PidConfig& cfg){
      velocity_cfg_ = cfg;
      velocity_pid_.setConfig(cfg);
      velocity_pid_.reset();
    }
    void setPositionPidConfig(const probot::control::PidConfig& cfg){
      position_cfg_ = cfg;
      position_pid_.setConfig(cfg);
      position_pid_.reset();
    }
    const probot::control::PidConfig& velocityPidConfig() const { return velocity_cfg_; }
    const probot::control::PidConfig& positionPidConfig() const { return position_cfg_; }

    void setTimeoutMs(uint32_t ms){ timeout_ms_ = ms; }

    bool setPower(float power) override {
      if (!driver_) return false;
      float clamped = std::clamp(power, -1.0f, 1.0f);
      active_mode_ = ControlType::kPercent;
      target_power_.store(clamped);
      last_ref_ms_.store(millis());
      bool ok = driver_->setPower(clamped);
      if (ok) {
        last_output_ = inverted_ ? -clamped : clamped;
      }
      return ok;
    }

    float getPower() const override { return last_output_; }

    bool supportsVelocity() const override { return true; }
    bool supportsPosition() const override { return true; }

    bool setVelocity(float units_per_s) override {
      target_velocity_.store(units_per_s);
      active_mode_ = ControlType::kVelocity;
      last_ref_ms_.store(millis());
      velocity_pid_.reset();
      return true;
    }

    bool setPosition(float units) override {
      target_position_.store(units);
      active_mode_ = ControlType::kPosition;
      last_ref_ms_.store(millis());
      position_pid_.reset();
      return true;
    }

    float getVelocity() const override { return target_velocity_.load(); }
    float getPosition() const override { return target_position_.load(); }

    void setInverted(bool inverted) override {
      inverted_ = inverted;
      if (driver_) driver_->setInverted(inverted);
    }
    bool getInverted() const override { return inverted_; }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (!encoder_ || !driver_) return;

      if (timeout_ms_ > 0 && (now_ms - last_ref_ms_.load()) > timeout_ms_){
        driver_->setPower(0.0f);
        last_output_ = 0.0f;
        return;
      }

      if (active_mode_ == ControlType::kPercent){
        float target_val = target_power_.load();
        float applied = inverted_ ? -target_val : target_val;
        driver_->setPower(target_val);
        last_measurement_ = target_val;
        last_output_ = applied;
        return;
      }

      float meas = 0.0f;
      float target = 0.0f;
      probot::control::PidConfig cfg;
      probot::control::PID* pid = nullptr;
      if (active_mode_ == ControlType::kVelocity){
        int32_t tps = encoder_->readTicksPerSecond();
        meas = vel_ticks_to_units_ * static_cast<float>(tps);
        target = target_velocity_.load();
        cfg = velocity_cfg_;
        pid = &velocity_pid_;
      } else {
        int32_t ticks = encoder_->readTicks();
        meas = pos_ticks_to_units_ * static_cast<float>(ticks);
        target = target_position_.load();
        cfg = position_cfg_;
        pid = &position_pid_;
      }

      float dt_s = dt_ms * 0.001f;
      if (dt_s <= 0.0f) dt_s = 0.001f;

      float error = target - meas;
      float pid_out = pid->step(error, dt_s);
      float ff = cfg.kf * target;
      float cmd = pid_out + ff;
      cmd = std::clamp(cmd, cfg.out_min, cfg.out_max);

      driver_->setPower(cmd);
      last_measurement_ = meas;
      last_output_ = inverted_ ? -cmd : cmd;
    }

    float lastSetpoint() const {
      if (active_mode_ == ControlType::kVelocity) return target_velocity_.load();
      if (active_mode_ == ControlType::kPosition) return target_position_.load();
      return target_power_.load();
    }
    float lastMeasurement() const { return last_measurement_; }
    float lastOutput() const { return last_output_; }
    ControlType activeMode() const { return active_mode_; }
    bool isAtTarget(float tolerance) const {
      if (active_mode_ == ControlType::kVelocity){
        return fabsf(target_velocity_.load() - last_measurement_) <= tolerance;
      }
      if (active_mode_ == ControlType::kPosition){
        return fabsf(target_position_.load() - last_measurement_) <= tolerance;
      }
      return false;
    }

  private:
    static probot::control::PidConfig defaultPidConfig(){
      return {0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f};
    }

    sensors::IEncoder* encoder_;
    motor::IMotorController* driver_;

    float vel_ticks_to_units_;
    float pos_ticks_to_units_;

    probot::control::PidConfig velocity_cfg_;
    probot::control::PidConfig position_cfg_;
    probot::control::PID velocity_pid_;
    probot::control::PID position_pid_;

    std::atomic<float> target_velocity_;
    std::atomic<float> target_position_;
    std::atomic<float> target_power_;
    std::atomic<uint32_t> last_ref_ms_;
    uint32_t timeout_ms_;

    ControlType active_mode_;
    bool inverted_;

    float last_measurement_ = 0.0f;
    float last_output_ = 0.0f;
  };
} // namespace probot::control
