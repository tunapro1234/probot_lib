#pragma once
#include <Arduino.h>
#include <atomic>
#include <probot/control/control_types.hpp>
#include <probot/control/pid.hpp>
#include <probot/devices/motors/imotor_controller.hpp>
#include <probot/sensors/encoder.hpp>

namespace probot::motor {

class BoardozaVNH5019MotorDriver : public IMotorController {
public:
  BoardozaVNH5019MotorDriver(int inaPin,
                             int inbPin,
                             int pwmPin,
                             int enaPin = -1,
                             int enbPin = -1);

  void begin();
  void setBrakeMode(bool enabled);
  bool brakeMode() const { return brake_mode_; }

  void setBrakeStrength(float dutyFraction); // 0.0 .. 1.0
  float brakeStrength() const { return brake_strength_; }

  void attachEncoder(probot::sensors::IEncoder* encoder,
                     float vel_ticks_per_s_to_units = 1.0f,
                     float pos_ticks_to_units = 1.0f);

  void setVelocityPidConfig(const probot::control::PidConfig& cfg);
  void setPositionPidConfig(const probot::control::PidConfig& cfg);
  const probot::control::PidConfig& velocityPidConfig() const { return velocity_cfg_; }
  const probot::control::PidConfig& positionPidConfig() const { return position_cfg_; }
  void setTimeoutMs(uint32_t ms) { timeout_ms_ = ms; }

  // IMotorController
  bool setPower(float power) override;
  float getPower() const override;
  void setInverted(bool inverted) override { inverted_ = inverted; }
  bool getInverted() const override { return inverted_; }

  bool supportsVelocity() const override { return encoder_ != nullptr; }
  bool supportsPosition() const override { return encoder_ != nullptr; }
  bool setVelocity(float units_per_s) override;
  bool setPosition(float units) override;
  float getVelocity() const override { return target_velocity_.load(); }
  float getPosition() const override { return target_position_.load(); }

  void update(uint32_t now_ms, uint32_t dt_ms) override;

  float lastSetpoint() const;
  float lastMeasurement() const { return last_measurement_; }
  float lastOutput() const { return last_output_; }
  probot::control::ControlType activeMode() const { return active_mode_; }
  bool isAtTarget(float tolerance) const;

private:
  static probot::control::PidConfig defaultPidConfig();
  bool applyPowerRaw(float power);
  void ensureInitialized();
  void configurePins();
  void applyStop();
  void applyDirection(bool forward);
  void writeDuty(float magnitude);

  int      ina_pin_;
  int      inb_pin_;
  int      pwm_pin_;
  int      ena_pin_;
  int      enb_pin_;
  bool   inverted_       = false;
  bool   initialized_    = false;
  bool   brake_mode_     = true;
  float  brake_strength_ = 1.0f;
  float  last_cmd_       = 0.0f;
  uint32_t pwm_max_value_ = 1023;

  probot::sensors::IEncoder* encoder_ = nullptr;
  float vel_ticks_to_units_ = 1.0f;
  float pos_ticks_to_units_ = 1.0f;

  probot::control::PidConfig velocity_cfg_;
  probot::control::PidConfig position_cfg_;
  probot::control::PID velocity_pid_;
  probot::control::PID position_pid_;

  std::atomic<float> target_velocity_{0.0f};
  std::atomic<float> target_position_{0.0f};
  std::atomic<float> target_power_{0.0f};
  std::atomic<uint32_t> last_ref_ms_{0};
  uint32_t timeout_ms_ = 500;

  probot::control::ControlType active_mode_ = probot::control::ControlType::kPercent;
  float last_measurement_ = 0.0f;
  float last_output_ = 0.0f;
};

} // namespace probot::motor
