#pragma once
#include <Arduino.h>
#include <cmath>
#include <probot/devices/motors/imotor_controller.hpp>

namespace probot::motor {

// Uses REN/LEN as shared PWM enable, RPWM/LPWM as direction selects.
class BTS7960BMotorController : public IMotorController {
public:
  BTS7960BMotorController(int rpwmPin, int lpwmPin, int renPin, int lenPin);

  void begin();

  bool setPower(float power) override;
  float getPower() const override { return last_cmd_; }
  void setInverted(bool inverted) override { inverted_ = inverted; }
  bool getInverted() const override { return inverted_; }

private:
  static float clampUnit(float v);
  static float clamp01(float v);
  static float deadband();
  static uint8_t pwmResolutionBits();
  static uint32_t pwmFrequencyHz();

  void ensureInitialized();
  void configurePins();
  void applyStop();
  void setDirection(bool forward);
  void writeEnableDuty(float magnitude);

  int rpwm_pin_;
  int lpwm_pin_;
  int ren_pin_;
  int len_pin_;
  bool inverted_ = false;
  bool initialized_ = false;
  float last_cmd_ = 0.0f;
  uint32_t pwm_max_value_ = 1023;
};

inline BTS7960BMotorController::BTS7960BMotorController(int rpwmPin, int lpwmPin, int renPin, int lenPin)
: rpwm_pin_(rpwmPin),
  lpwm_pin_(lpwmPin),
  ren_pin_(renPin),
  len_pin_(lenPin) {}

inline float BTS7960BMotorController::clampUnit(float v){
  if (v > 1.0f) return 1.0f;
  if (v < -1.0f) return -1.0f;
  return v;
}

inline float BTS7960BMotorController::clamp01(float v){
  if (v < 0.0f) return 0.0f;
  if (v > 1.0f) return 1.0f;
  return v;
}

inline float BTS7960BMotorController::deadband(){
  return 1e-3f;
}

inline uint8_t BTS7960BMotorController::pwmResolutionBits(){
  return 10;
}

inline uint32_t BTS7960BMotorController::pwmFrequencyHz(){
  return 20000;
}

inline void BTS7960BMotorController::configurePins(){
  pinMode(rpwm_pin_, OUTPUT);
  pinMode(lpwm_pin_, OUTPUT);
  pinMode(ren_pin_, OUTPUT);
  pinMode(len_pin_, OUTPUT);
  digitalWrite(rpwm_pin_, LOW);
  digitalWrite(lpwm_pin_, LOW);
}

inline void BTS7960BMotorController::begin(){
  if (initialized_) return;
  configurePins();

  analogWriteResolution(ren_pin_, pwmResolutionBits());
  analogWriteResolution(len_pin_, pwmResolutionBits());
  analogWriteFrequency(ren_pin_, pwmFrequencyHz());
  analogWriteFrequency(len_pin_, pwmFrequencyHz());
  uint8_t bits = pwmResolutionBits();
  pwm_max_value_ = (bits >= 31)
    ? 0x7FFFFFFFu
    : ((1u << bits) - 1u);

  applyStop();
  initialized_ = true;
}

inline void BTS7960BMotorController::ensureInitialized(){
  if (!initialized_) begin();
}

inline void BTS7960BMotorController::applyStop(){
  writeEnableDuty(0.0f);
  digitalWrite(rpwm_pin_, LOW);
  digitalWrite(lpwm_pin_, LOW);
  last_cmd_ = 0.0f;
}

inline void BTS7960BMotorController::setDirection(bool forward){
  digitalWrite(rpwm_pin_, forward ? HIGH : LOW);
  digitalWrite(lpwm_pin_, forward ? LOW : HIGH);
}

inline void BTS7960BMotorController::writeEnableDuty(float magnitude){
  float mag = clamp01(magnitude);
  uint32_t duty = static_cast<uint32_t>(std::round(mag * static_cast<float>(pwm_max_value_)));
  if (duty > pwm_max_value_) duty = pwm_max_value_;
  analogWrite(ren_pin_, static_cast<int>(duty));
  analogWrite(len_pin_, static_cast<int>(duty));
}

inline bool BTS7960BMotorController::setPower(float power){
  ensureInitialized();
  if (!initialized_) return false;

  float cmd = clampUnit(inverted_ ? -power : power);
  if (std::fabs(cmd) <= deadband()){
    applyStop();
    return true;
  }

  last_cmd_ = cmd;
  setDirection(cmd >= 0.0f);
  writeEnableDuty(std::fabs(cmd));
  return true;
}

} // namespace probot::motor
