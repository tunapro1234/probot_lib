#pragma once
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::test {
  class TestMotor : public probot::motor::IMotorDriver {
  public:
    bool setPower(float power) override {
      if (power < -1.0f) power = -1.0f; else if (power > 1.0f) power = 1.0f;
      commanded_ = power;
      last_cmd_ = inverted_ ? -power : power;
      return true;
    }

    void setInverted(bool inverted) override { inverted_ = inverted; }
    bool getInverted() const override { return inverted_; }

    float appliedPower() const { return last_cmd_; }
    float commandedPower() const { return commanded_; }

  private:
    bool    inverted_= false;
    float   last_cmd_= 0.0f;
    float   commanded_= 0.0f;
  };
} // namespace probot::test

// Backward compatibility alias
namespace probot::motor {
  using NullMotor = probot::test::TestMotor;
}
