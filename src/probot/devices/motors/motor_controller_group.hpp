#pragma once
#include <probot/devices/motors/imotor_controller.hpp>

namespace probot::motor {
  class MotorControllerGroup : public IMotorController {
  public:
    MotorControllerGroup(IMotorController* a, IMotorController* b)
    : a_(a), b_(b), inverted_(false) {}

    bool setPower(float power) override {
      if (!a_ || !b_) return false;
      bool ok1 = a_->setPower(power);
      bool ok2 = b_->setPower(power);
      return ok1 && ok2;
    }

    float getPower() const override {
      float pa = a_ ? a_->getPower() : IMotorController::kUnsupported;
      float pb = b_ ? b_->getPower() : IMotorController::kUnsupported;
      if (pa == IMotorController::kUnsupported &&
          pb == IMotorController::kUnsupported) {
        return IMotorController::kUnsupported;
      }
      if (pa == IMotorController::kUnsupported) return pb;
      if (pb == IMotorController::kUnsupported) return pa;
      return 0.5f * (pa + pb);
    }

    bool supportsVelocity() const override {
      if (!a_ || !b_) return false;
      return a_->supportsVelocity() && b_->supportsVelocity();
    }
    bool supportsPosition() const override {
      if (!a_ || !b_) return false;
      return a_->supportsPosition() && b_->supportsPosition();
    }
    bool setVelocity(float units_per_s) override {
      if (!a_ || !b_) return false;
      bool ok1 = a_->setVelocity(units_per_s);
      bool ok2 = b_->setVelocity(units_per_s);
      return ok1 && ok2;
    }
    bool setPosition(float units) override {
      if (!a_ || !b_) return false;
      bool ok1 = a_->setPosition(units);
      bool ok2 = b_->setPosition(units);
      return ok1 && ok2;
    }
    float getVelocity() const override {
      float va = a_ ? a_->getVelocity() : IMotorController::kUnsupported;
      float vb = b_ ? b_->getVelocity() : IMotorController::kUnsupported;
      if (va == IMotorController::kUnsupported &&
          vb == IMotorController::kUnsupported) {
        return IMotorController::kUnsupported;
      }
      if (va == IMotorController::kUnsupported) return vb;
      if (vb == IMotorController::kUnsupported) return va;
      return 0.5f * (va + vb);
    }
    float getPosition() const override {
      float pa = a_ ? a_->getPosition() : IMotorController::kUnsupported;
      float pb = b_ ? b_->getPosition() : IMotorController::kUnsupported;
      if (pa == IMotorController::kUnsupported &&
          pb == IMotorController::kUnsupported) {
        return IMotorController::kUnsupported;
      }
      if (pa == IMotorController::kUnsupported) return pb;
      if (pb == IMotorController::kUnsupported) return pa;
      return 0.5f * (pa + pb);
    }

    void setInverted(bool inverted) override {
      inverted_ = inverted;
      if (a_) a_->setInverted(inverted);
      if (b_) b_->setInverted(inverted);
    }
    bool getInverted() const override { return inverted_; }

  private:
    IMotorController* a_;
    IMotorController* b_;
    bool    inverted_;
  };
} // namespace probot::motor 
