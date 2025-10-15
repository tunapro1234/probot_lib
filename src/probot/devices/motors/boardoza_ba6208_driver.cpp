#include <cmath>
#include <Arduino.h>
#include <probot/devices/motors/boardoza_ba6208_driver.hpp>
#ifdef ARDUINO_ARCH_ESP32
#include <esp32-hal-ledc.h>
#endif

namespace probot::motor {
namespace {
  constexpr float kDeadband = 1e-3f;
  inline float clampUnit(float v){
    if (v > 1.0f) return 1.0f;
    if (v < -1.0f) return -1.0f;
    return v;
  }
}

BoardozaBA6208Driver::BoardozaBA6208Driver(int pwmPin,
                                           int in1Pin,
                                           int in2Pin,
                                           uint8_t pwmChannel,
                                           uint32_t pwmFrequency,
                                           uint8_t pwmResolution)
: pwm_pin_(pwmPin), in1_pin_(in1Pin), in2_pin_(in2Pin),
  pwm_channel_(pwmChannel), pwm_frequency_(pwmFrequency), pwm_resolution_(pwmResolution) {}

void BoardozaBA6208Driver::begin(){
  if (initialized_) return;
  pinMode(in1_pin_, OUTPUT);
  pinMode(in2_pin_, OUTPUT);

#ifdef ARDUINO_ARCH_ESP32
  ledcAttachChannel(pwm_pin_, pwm_frequency_, pwm_resolution_, pwm_channel_);
  ledcWriteChannel(pwm_channel_, 0);
#else
  pinMode(pwm_pin_, OUTPUT);
  analogWrite(pwm_pin_, 0);
#endif
  applyStop();
  initialized_ = true;
}

void BoardozaBA6208Driver::ensureInitialized(){
  if (!initialized_) begin();
}

void BoardozaBA6208Driver::setBrakeMode(bool enabled){
  brake_mode_ = enabled;
  if (!initialized_) return;
  applyStop();
}

bool BoardozaBA6208Driver::claim(void* owner){
  if (owner_ && owner_ != owner) return false;
  owner_ = owner;
  return true;
}

void BoardozaBA6208Driver::release(void* owner){
  if (owner_ != owner) return;
  owner_ = nullptr;
  applyStop();
}

bool BoardozaBA6208Driver::setPower(float power, void* owner){
  if (owner_ && owner_ != owner) return false;
  ensureInitialized();
  if (!initialized_) return false;

  float cmd = clampUnit(inverted_ ? -power : power);

  if (std::fabs(cmd) <= kDeadband){
    applyStop();
    return true;
  }

  bool forward = cmd > 0.0f;
  applyDirection(forward);
  writeDuty(std::fabs(cmd));
  return true;
}

void BoardozaBA6208Driver::applyStop(){
  if (!initialized_) return;
#ifdef ARDUINO_ARCH_ESP32
  ledcWriteChannel(pwm_channel_, 0);
#else
  analogWrite(pwm_pin_, 0);
#endif
  if (brake_mode_){
    digitalWrite(in1_pin_, HIGH);
    digitalWrite(in2_pin_, HIGH);
  } else {
    digitalWrite(in1_pin_, LOW);
    digitalWrite(in2_pin_, LOW);
  }
}

void BoardozaBA6208Driver::applyDirection(bool forward){
  digitalWrite(in1_pin_, forward ? HIGH : LOW);
  digitalWrite(in2_pin_, forward ? LOW : HIGH);
}

void BoardozaBA6208Driver::writeDuty(float magnitude){
  float mag = magnitude;
  if (mag < 0.0f) mag = 0.0f;
  if (mag > 1.0f) mag = 1.0f;
#ifdef ARDUINO_ARCH_ESP32
  uint32_t maxDuty = (1u << pwm_resolution_) - 1u;
  uint32_t duty = static_cast<uint32_t>(std::round(mag * static_cast<float>(maxDuty)));
  ledcWriteChannel(pwm_channel_, duty);
#else
  uint32_t duty = static_cast<uint32_t>(std::round(mag * 255.0f));
  if (duty > 255u) duty = 255u;
  analogWrite(pwm_pin_, static_cast<uint8_t>(duty));
#endif
}

} // namespace probot::motor
