#include <cmath>
#include <Arduino.h>
#include <probot/devices/motors/bts7960b_motor_driver.hpp>

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
  constexpr uint8_t  kPwmResolutionBits = 10;
  constexpr uint32_t kPwmFrequencyHz    = 20000;
}

BTS7960BMotorDriver::BTS7960BMotorDriver(int rpwmPin, int lpwmPin, int renPin, int lenPin)
: rpwm_pin_(rpwmPin),
  lpwm_pin_(lpwmPin),
  ren_pin_(renPin),
  len_pin_(lenPin) {}

void BTS7960BMotorDriver::configurePins(){
  pinMode(rpwm_pin_, OUTPUT);
  pinMode(lpwm_pin_, OUTPUT);
  pinMode(ren_pin_, OUTPUT);
  pinMode(len_pin_, OUTPUT);
  digitalWrite(rpwm_pin_, LOW);
  digitalWrite(lpwm_pin_, LOW);
}

void BTS7960BMotorDriver::begin(){
  if (initialized_) return;
  configurePins();

  analogWriteResolution(ren_pin_, kPwmResolutionBits);
  analogWriteResolution(len_pin_, kPwmResolutionBits);
  analogWriteFrequency(ren_pin_, kPwmFrequencyHz);
  analogWriteFrequency(len_pin_, kPwmFrequencyHz);
  pwm_max_value_ = (kPwmResolutionBits >= 31)
    ? 0x7FFFFFFFu
    : ((1u << kPwmResolutionBits) - 1u);

  applyStop();
  initialized_ = true;
}

void BTS7960BMotorDriver::ensureInitialized(){
  if (!initialized_) begin();
}

void BTS7960BMotorDriver::applyStop(){
  writeEnableDuty(0.0f);
  digitalWrite(rpwm_pin_, LOW);
  digitalWrite(lpwm_pin_, LOW);
  last_cmd_ = 0.0f;
}

void BTS7960BMotorDriver::setDirection(bool forward){
  digitalWrite(rpwm_pin_, forward ? HIGH : LOW);
  digitalWrite(lpwm_pin_, forward ? LOW : HIGH);
}

void BTS7960BMotorDriver::writeEnableDuty(float magnitude){
  float mag = clamp01(magnitude);
  uint32_t duty = static_cast<uint32_t>(std::round(mag * static_cast<float>(pwm_max_value_)));
  if (duty > pwm_max_value_) duty = pwm_max_value_;
  analogWrite(ren_pin_, static_cast<int>(duty));
  analogWrite(len_pin_, static_cast<int>(duty));
}

bool BTS7960BMotorDriver::setPower(float power){
  ensureInitialized();
  if (!initialized_) return false;

  float cmd = clampUnit(inverted_ ? -power : power);
  if (std::fabs(cmd) <= kDeadband){
    applyStop();
    return true;
  }

  last_cmd_ = cmd;
  setDirection(cmd >= 0.0f);
  writeEnableDuty(std::fabs(cmd));
  return true;
}

} // namespace probot::motor
