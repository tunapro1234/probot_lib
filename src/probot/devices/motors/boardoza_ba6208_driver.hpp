#pragma once
#include <Arduino.h>
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::motor {

class BoardozaBA6208Driver : public IMotorDriver {
public:
  BoardozaBA6208Driver(int pwmPin,
                       int in1Pin,
                       int in2Pin,
                       uint8_t pwmChannel = 0,
                       uint32_t pwmFrequency = 10000,
                       uint8_t pwmResolution = 8);

  void begin();
  void setBrakeMode(bool enabled);
  bool brakeMode() const { return brake_mode_; }

  // IMotorDriver overrides
  bool claim(void* owner) override;
  void release(void* owner) override;
  bool setPower(float power, void* owner) override;
  bool isClaimed() const override { return owner_ != nullptr; }
  void* currentOwner() const override { return owner_; }
  void setInverted(bool inverted) override { inverted_ = inverted; }
  bool getInverted() const override { return inverted_; }

private:
  void ensureInitialized();
  void applyStop();
  void applyDirection(bool forward);
  void writeDuty(float magnitude);

  int      pwm_pin_;
  int      in1_pin_;
  int      in2_pin_;
  uint8_t  pwm_channel_;
  uint32_t pwm_frequency_;
  uint8_t  pwm_resolution_;

  void*  owner_        = nullptr;
  bool   inverted_     = false;
  bool   initialized_  = false;
  bool   brake_mode_   = true;
};

} // namespace probot::motor
