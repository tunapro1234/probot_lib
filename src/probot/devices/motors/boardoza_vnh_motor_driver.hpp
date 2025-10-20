#pragma once
#include <Arduino.h>
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::motor {

class BoardozaVNHMotorDriver : public IMotorDriver {
public:
  BoardozaVNHMotorDriver(int inaPin,
                         int inbPin,
                         int pwmPin,
                         int enaPin = -1,
                         int enbPin = -1);

  void begin();
  void setBrakeMode(bool enabled);
  bool brakeMode() const { return brake_mode_; }

  void setBrakeStrength(float dutyFraction); // 0.0 .. 1.0
  float brakeStrength() const { return brake_strength_; }

  // IMotorDriver
  bool setPower(float power) override;
  void setInverted(bool inverted) override { inverted_ = inverted; }
  bool getInverted() const override { return inverted_; }

private:
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
  uint32_t pwm_max_value_ = 1023;
};

} // namespace probot::motor
