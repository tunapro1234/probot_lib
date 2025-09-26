#pragma once
#include <probot/controllers/motor_controller.hpp>

namespace probot::controllers {
  class ClosedLoopMotorGroup : public IMotorController {
  public:
    ClosedLoopMotorGroup(IMotorController* a, IMotorController* b)
    : a_(a), b_(b), owner_(nullptr), inverted_(false) {}

    // Group control API
    void setSetpoint(float value, ControlType mode, int slot = -1) override {
      if (a_) a_->setSetpoint(value, mode, slot);
      if (b_) b_->setSetpoint(value, mode, slot);
    }
    void setTimeoutMs(uint32_t ms) override { if (a_) a_->setTimeoutMs(ms); if (b_) b_->setTimeoutMs(ms); }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (a_) a_->update(now_ms, dt_ms);
      if (b_) b_->update(now_ms, dt_ms);
    }

    // IMotor for raw power when needed
    bool claim(void* owner) override {
      if (owner_ && owner_ != owner) return false;
      if (!a_ || !b_) return false;
      if (!a_->claim(owner)) return false;
      if (!b_->claim(owner)) { a_->release(owner); return false; }
      owner_ = owner;
      return true;
    }
    void release(void* owner) override {
      if (owner_ != owner) return;
      if (b_) b_->release(owner);
      if (a_) a_->release(owner);
      owner_ = nullptr;
    }
    bool setPower(float power, void* owner) override {
      if (owner_ != owner) return false;
      float p = inverted_ ? -power : power;
      bool ok1 = a_ ? a_->setPower(p, owner) : false;
      bool ok2 = b_ ? b_->setPower(p, owner) : false;
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
    IMotorController* a_;
    IMotorController* b_;
    void* owner_;
    bool  inverted_;
  };
} // namespace probot::controllers 
