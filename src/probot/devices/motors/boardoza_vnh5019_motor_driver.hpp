#pragma once
#include <Arduino.h>
#include <atomic>
#include <cmath>
#include <probot/control/control_types.hpp>
#include <probot/control/pid.hpp>
#include <probot/devices/motors/imotor_controller.hpp>
#include <probot/devices/sensors/encoder.hpp>

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
  static float clampUnit(float v);
  static float clamp01(float v);
  static float deadband();
  static uint8_t pwmResolutionBits();
  static uint32_t pwmFrequencyHz();

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

inline BoardozaVNH5019MotorDriver::BoardozaVNH5019MotorDriver(int inaPin,
                                                             int inbPin,
                                                             int pwmPin,
                                                             int enaPin,
                                                             int enbPin)
: ina_pin_(inaPin),
  inb_pin_(inbPin),
  pwm_pin_(pwmPin),
  ena_pin_(enaPin),
  enb_pin_(enbPin),
  velocity_cfg_(defaultPidConfig()),
  position_cfg_(defaultPidConfig()),
  velocity_pid_(velocity_cfg_),
  position_pid_(position_cfg_) {}

inline probot::control::PidConfig BoardozaVNH5019MotorDriver::defaultPidConfig(){
  return {0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f};
}

inline float BoardozaVNH5019MotorDriver::clampUnit(float v){
  if (v > 1.0f) return 1.0f;
  if (v < -1.0f) return -1.0f;
  return v;
}

inline float BoardozaVNH5019MotorDriver::clamp01(float v){
  if (v < 0.0f) return 0.0f;
  if (v > 1.0f) return 1.0f;
  return v;
}

inline float BoardozaVNH5019MotorDriver::deadband(){
  return 1e-3f;
}

inline uint8_t BoardozaVNH5019MotorDriver::pwmResolutionBits(){
  return 10;
}

inline uint32_t BoardozaVNH5019MotorDriver::pwmFrequencyHz(){
  return 20000;
}

inline void BoardozaVNH5019MotorDriver::attachEncoder(probot::sensors::IEncoder* encoder,
                                                      float vel_ticks_per_s_to_units,
                                                      float pos_ticks_to_units){
  encoder_ = encoder;
  vel_ticks_to_units_ = vel_ticks_per_s_to_units;
  pos_ticks_to_units_ = pos_ticks_to_units;
}

inline void BoardozaVNH5019MotorDriver::setVelocityPidConfig(const probot::control::PidConfig& cfg){
  velocity_cfg_ = cfg;
  velocity_pid_.setConfig(cfg);
  velocity_pid_.reset();
}

inline void BoardozaVNH5019MotorDriver::setPositionPidConfig(const probot::control::PidConfig& cfg){
  position_cfg_ = cfg;
  position_pid_.setConfig(cfg);
  position_pid_.reset();
}

inline void BoardozaVNH5019MotorDriver::configurePins(){
  pinMode(ina_pin_, OUTPUT);
  pinMode(inb_pin_, OUTPUT);
  pinMode(pwm_pin_, OUTPUT);
  if (ena_pin_ >= 0){
    pinMode(ena_pin_, OUTPUT);
    digitalWrite(ena_pin_, HIGH);
  }
  if (enb_pin_ >= 0){
    pinMode(enb_pin_, OUTPUT);
    digitalWrite(enb_pin_, HIGH);
  }
}

inline void BoardozaVNH5019MotorDriver::begin(){
  if (initialized_) return;

  configurePins();

  analogWriteResolution(pwm_pin_, pwmResolutionBits());
  analogWriteFrequency(pwm_pin_, pwmFrequencyHz());
  uint8_t bits = pwmResolutionBits();
  pwm_max_value_ = (bits >= 31)
    ? 0x7FFFFFFFu
    : ((1u << bits) - 1u);

  analogWrite(pwm_pin_, 0);
  applyStop();
  initialized_ = true;
}

inline void BoardozaVNH5019MotorDriver::ensureInitialized(){
  if (!initialized_) begin();
}

inline void BoardozaVNH5019MotorDriver::setBrakeMode(bool enabled){
  brake_mode_ = enabled;
  if (!initialized_) return;
  applyStop();
}

inline void BoardozaVNH5019MotorDriver::setBrakeStrength(float dutyFraction){
  brake_strength_ = clamp01(dutyFraction);
  if (!initialized_) return;
  if (brake_mode_){
    applyStop();
  }
}

inline bool BoardozaVNH5019MotorDriver::applyPowerRaw(float power){
  ensureInitialized();
  if (!initialized_) return false;

  float cmd = clampUnit(inverted_ ? -power : power);
  if (std::fabs(cmd) <= deadband()){
    last_cmd_ = 0.0f;
    applyStop();
    return true;
  }

  last_cmd_ = cmd;
  bool forward = cmd >= 0.0f;
  applyDirection(forward);
  writeDuty(std::fabs(cmd));
  return true;
}

inline bool BoardozaVNH5019MotorDriver::setPower(float power){
  float clamped = clampUnit(power);
  active_mode_ = probot::control::ControlType::kPercent;
  target_power_.store(clamped);
  last_ref_ms_.store(millis());
  bool ok = applyPowerRaw(clamped);
  if (ok){
    last_measurement_ = clamped;
    last_output_ = last_cmd_;
  }
  return ok;
}

inline float BoardozaVNH5019MotorDriver::getPower() const {
  return last_cmd_;
}

inline bool BoardozaVNH5019MotorDriver::setVelocity(float units_per_s){
  if (!encoder_) return false;
  target_velocity_.store(units_per_s);
  bool mode_changed = active_mode_ != probot::control::ControlType::kVelocity;
  active_mode_ = probot::control::ControlType::kVelocity;
  last_ref_ms_.store(millis());
  if (mode_changed){
    velocity_pid_.reset();
  }
  return true;
}

inline bool BoardozaVNH5019MotorDriver::setPosition(float units){
  if (!encoder_) return false;
  target_position_.store(units);
  bool mode_changed = active_mode_ != probot::control::ControlType::kPosition;
  active_mode_ = probot::control::ControlType::kPosition;
  last_ref_ms_.store(millis());
  if (mode_changed){
    position_pid_.reset();
  }
  return true;
}

inline void BoardozaVNH5019MotorDriver::update(uint32_t now_ms, uint32_t dt_ms){
  ensureInitialized();
  if (!initialized_) return;

  if (timeout_ms_ > 0 && (now_ms - last_ref_ms_.load()) > timeout_ms_){
    applyPowerRaw(0.0f);
    last_output_ = last_cmd_;
    return;
  }

  if (active_mode_ == probot::control::ControlType::kPercent){
    float target_val = target_power_.load();
    applyPowerRaw(target_val);
    last_measurement_ = target_val;
    last_output_ = last_cmd_;
    return;
  }

  if (!encoder_) return;

  float meas = 0.0f;
  float target = 0.0f;
  probot::control::PidConfig cfg;
  probot::control::PID* pid = nullptr;
  if (active_mode_ == probot::control::ControlType::kVelocity){
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
  cmd = clampUnit(cmd);
  if (cmd < cfg.out_min) cmd = cfg.out_min;
  if (cmd > cfg.out_max) cmd = cfg.out_max;

  applyPowerRaw(cmd);
  last_measurement_ = meas;
  last_output_ = last_cmd_;
}

inline float BoardozaVNH5019MotorDriver::lastSetpoint() const {
  if (active_mode_ == probot::control::ControlType::kVelocity) return target_velocity_.load();
  if (active_mode_ == probot::control::ControlType::kPosition) return target_position_.load();
  return target_power_.load();
}

inline bool BoardozaVNH5019MotorDriver::isAtTarget(float tolerance) const {
  if (active_mode_ == probot::control::ControlType::kVelocity){
    return std::fabs(target_velocity_.load() - last_measurement_) <= tolerance;
  }
  if (active_mode_ == probot::control::ControlType::kPosition){
    return std::fabs(target_position_.load() - last_measurement_) <= tolerance;
  }
  return false;
}

inline void BoardozaVNH5019MotorDriver::applyStop(){
  if (!initialized_){
    configurePins();
  }

  if (brake_mode_){
    digitalWrite(ina_pin_, HIGH);
    digitalWrite(inb_pin_, HIGH);
    writeDuty(brake_strength_);
  } else {
    writeDuty(0.0f);
    digitalWrite(ina_pin_, LOW);
    digitalWrite(inb_pin_, LOW);
  }
}

inline void BoardozaVNH5019MotorDriver::applyDirection(bool forward){
  digitalWrite(ina_pin_, forward ? HIGH : LOW);
  digitalWrite(inb_pin_, forward ? LOW : HIGH);
}

inline void BoardozaVNH5019MotorDriver::writeDuty(float magnitude){
  float mag = clamp01(magnitude);
  uint32_t duty = static_cast<uint32_t>(std::round(mag * static_cast<float>(pwm_max_value_)));
  if (duty > pwm_max_value_) duty = pwm_max_value_;
  analogWrite(pwm_pin_, static_cast<int>(duty));
}

} // namespace probot::motor
