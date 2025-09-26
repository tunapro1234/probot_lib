#pragma once
#include <probot/devices/motors/motor.hpp>

namespace probot::motor {
  class MotorGroup : public IMotor {
  public:
    MotorGroup(IMotor* a, IMotor* b)
    : a_(a), b_(b), owner_(nullptr), inverted_(false) {}

    bool claim(void* owner) override {
      if (!a_ || !b_) return false;
      if (owner_) return owner_ == owner;
      if (!a_->claim(owner)) return false;
      if (!b_->claim(owner)) { a_->release(owner); return false; }
      owner_ = owner;
      return true;
    }

    void release(void* owner) override {
      if (!a_ || !b_) return;
      if (owner_ != owner) return;
      b_->release(owner);
      a_->release(owner);
      owner_ = nullptr;
    }

    bool setPower(float power, void* owner) override {
      if (!a_ || !b_) return false;
      if (owner_ != owner) return false;
      float p = inverted_ ? -power : power;
      bool ok1 = a_->setPower(p, owner);
      bool ok2 = b_->setPower(p, owner);
      return ok1 && ok2;
    }

    bool isClaimed() const override { return owner_ != nullptr; }
    void* currentOwner() const override { return owner_; }

    void setInverted(bool inverted) override {
      inverted_ = inverted;
      if (a_) a_->setInverted(inverted);
      if (b_) b_->setInverted(inverted);
    }
    bool getInverted() const override { return inverted_; }

  private:
    IMotor* a_;
    IMotor* b_;
    void*   owner_;
    bool    inverted_;
  };
} // namespace probot::motor 
