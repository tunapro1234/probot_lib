#include <cmath>
#include <Arduino.h>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>

namespace probot::motor {
namespace {
  constexpr float kDeadband = 1e-3f;

  inline float clampUnit(float v){
    if (v > 1.0f) return 1.0f;
    if (v < -1.0f) return -1.0f;
    return v;
  }

  inline float clamp01(float v){
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
  }
}

namespace {
  // Datasheet (ST VNH5019A-E, Table 12) suggests PWM switching up to 20 kHz.
  // 10-bit resolution keeps duty control granular while staying well within the device's timing budget.
  constexpr uint8_t  kPwmResolutionBits = 10;
  constexpr uint32_t kPwmFrequencyHz    = 20000;
}

BoardozaVNHMotorDriver::BoardozaVNHMotorDriver(int inaPin,
                                               int inbPin,
                                               int pwmPin,
                                               int enaPin,
                                               int enbPin)
: ina_pin_(inaPin),
  inb_pin_(inbPin),
  pwm_pin_(pwmPin),
  ena_pin_(enaPin),
  enb_pin_(enbPin) {}

void BoardozaVNHMotorDriver::configurePins(){
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

void BoardozaVNHMotorDriver::begin(){
  if (initialized_) return;

  configurePins();

  analogWriteResolution(pwm_pin_, kPwmResolutionBits);
  analogWriteFrequency(pwm_pin_, kPwmFrequencyHz);
  pwm_max_value_ = (kPwmResolutionBits >= 31)
    ? 0x7FFFFFFFu
    : ((1u << kPwmResolutionBits) - 1u);

  analogWrite(pwm_pin_, 0);
  applyStop();
  initialized_ = true;
}

void BoardozaVNHMotorDriver::ensureInitialized(){
  if (!initialized_) begin();
}

void BoardozaVNHMotorDriver::setBrakeMode(bool enabled){
  brake_mode_ = enabled;
  if (!initialized_) return;
  applyStop();
}

void BoardozaVNHMotorDriver::setBrakeStrength(float dutyFraction){
  brake_strength_ = clamp01(dutyFraction);
  if (!initialized_) return;
  if (brake_mode_){
    applyStop();
  }
}

bool BoardozaVNHMotorDriver::claim(void* owner){
  if (owner_ && owner_ != owner) return false;
  owner_ = owner;
  return true;
}

void BoardozaVNHMotorDriver::release(void* owner){
  if (owner_ != owner) return;
  owner_ = nullptr;
  applyStop();
}

bool BoardozaVNHMotorDriver::setPower(float power, void* owner){
  if (owner_ && owner_ != owner) return false;

  ensureInitialized();
  if (!initialized_) return false;

  float cmd = clampUnit(inverted_ ? -power : power);
  if (std::fabs(cmd) <= kDeadband){
    applyStop();
    return true;
  }

  bool forward = cmd >= 0.0f;
  applyDirection(forward);
  writeDuty(std::fabs(cmd));
  return true;
}

void BoardozaVNHMotorDriver::applyStop(){
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

void BoardozaVNHMotorDriver::applyDirection(bool forward){
  digitalWrite(ina_pin_, forward ? HIGH : LOW);
  digitalWrite(inb_pin_, forward ? LOW : HIGH);
}

void BoardozaVNHMotorDriver::writeDuty(float magnitude){
  float mag = clamp01(magnitude);
  uint32_t duty = static_cast<uint32_t>(std::round(mag * static_cast<float>(pwm_max_value_)));
  if (duty > pwm_max_value_) duty = pwm_max_value_;
  analogWrite(pwm_pin_, static_cast<int>(duty));
}

} // namespace probot::motor
