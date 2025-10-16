#pragma once
#include <probot/control/imotor_controller.hpp>
#include <math.h>

namespace probot::control {
  class ClosedLoopMotorGroup : public IMotorController {
  public:
    ClosedLoopMotorGroup(IMotorController* a, IMotorController* b)
    : a_(a), b_(b), inverted_(false) {}

    // Group control API
    void setSetpoint(float value, ControlType mode, int slot = -1) override {
      if (a_) a_->setSetpoint(value, mode, slot);
      if (b_) b_->setSetpoint(value, mode, slot);
    }
    void setTimeoutMs(uint32_t ms) override { if (a_) a_->setTimeoutMs(ms); if (b_) b_->setTimeoutMs(ms); }
    void setPidSlotConfig(int slot, const probot::control::PidConfig& cfg) override { if (a_) a_->setPidSlotConfig(slot, cfg); if (b_) b_->setPidSlotConfig(slot, cfg); }
    void selectDefaultSlot(ControlType mode, int slot) override {
      if (a_) a_->selectDefaultSlot(mode, slot);
      if (b_) b_->selectDefaultSlot(mode, slot);
    }
    int defaultSlot(ControlType mode) const override {
      return a_ ? a_->defaultSlot(mode) : 0;
    }
    float lastSetpoint() const override {
      if (a_ && b_) return 0.5f * (a_->lastSetpoint() + b_->lastSetpoint());
      return a_ ? a_->lastSetpoint() : (b_ ? b_->lastSetpoint() : 0.0f);
    }
    float lastMeasurement() const override {
      if (a_ && b_) return 0.5f * (a_->lastMeasurement() + b_->lastMeasurement());
      return a_ ? a_->lastMeasurement() : (b_ ? b_->lastMeasurement() : 0.0f);
    }
    float lastOutput() const override {
      if (a_ && b_) return 0.5f * (a_->lastOutput() + b_->lastOutput());
      return a_ ? a_->lastOutput() : (b_ ? b_->lastOutput() : 0.0f);
    }
    ControlType activeMode() const override { return a_ ? a_->activeMode() : ControlType::kPercent; }
    bool isAtTarget(float tolerance) const override {
      bool okA = a_ ? a_->isAtTarget(tolerance) : true;
      bool okB = b_ ? b_->isAtTarget(tolerance) : true;
      return okA && okB;
    }

    void setMotionProfile(probot::control::MotionProfileType type) override {
      if (a_) a_->setMotionProfile(type);
      if (b_) b_->setMotionProfile(type);
    }
    probot::control::MotionProfileType motionProfile() const override {
      return a_ ? a_->motionProfile() : (b_ ? b_->motionProfile() : probot::control::MotionProfileType::kNone);
    }
    void setMotionProfileConfig(const probot::control::MotionProfileConfig& cfg) override {
      if (a_) a_->setMotionProfileConfig(cfg);
      if (b_) b_->setMotionProfileConfig(cfg);
    }
    probot::control::MotionProfileConfig motionProfileConfig() const override {
      return a_ ? a_->motionProfileConfig() : (b_ ? b_->motionProfileConfig() : probot::control::MotionProfileConfig{});
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (a_) a_->update(now_ms, dt_ms);
      if (b_) b_->update(now_ms, dt_ms);
    }

    // IMotor for raw power when needed
    bool setPower(float power) override {
      bool ok1 = a_ ? a_->setPower(power) : false;
      bool ok2 = b_ ? b_->setPower(power) : false;
      return ok1 && ok2;
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
    bool  inverted_;
  };
} // namespace probot::control 
