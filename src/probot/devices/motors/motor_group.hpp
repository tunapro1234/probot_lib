#pragma once
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::motor {
  class MotorGroup : public IMotorDriver {
  public:
    MotorGroup(IMotorDriver* a, IMotorDriver* b)
    : a_(a), b_(b), inverted_(false) {}

    bool setPower(float power) override {
      if (!a_ || !b_) return false;
      float p = inverted_ ? -power : power;
      bool ok1 = a_->setPower(p);
      bool ok2 = b_->setPower(p);
      return ok1 && ok2;
    }

    void setInverted(bool inverted) override {
      inverted_ = inverted;
      if (a_) a_->setInverted(inverted);
      if (b_) b_->setInverted(inverted);
    }
    bool getInverted() const override { return inverted_; }

  private:
    IMotorDriver* a_;
    IMotorDriver* b_;
    bool    inverted_;
  };
} // namespace probot::motor 
