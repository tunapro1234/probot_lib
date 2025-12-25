#pragma once
#include <Arduino.h>
#include <probot/devices/motors/imotor_controller.hpp>

namespace probot::motor {

// Uses REN/LEN as shared PWM enable, RPWM/LPWM as direction selects.
class BTS7960BMotorDriver : public IMotorController {
public:
  BTS7960BMotorDriver(int rpwmPin, int lpwmPin, int renPin, int lenPin);

  void begin();

  bool setPower(float power) override;
  float getPower() const override { return last_cmd_; }
  void setInverted(bool inverted) override { inverted_ = inverted; }
  bool getInverted() const override { return inverted_; }

private:
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

} // namespace probot::motor
