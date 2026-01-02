#pragma once
#include <probot/control/pid_motor_wrapper.hpp>

namespace probot::control {
  class PidMotorWrapperGroup : public motor::IMotorController {
  public:
    PidMotorWrapperGroup(PidMotorWrapper* a, PidMotorWrapper* b)
    : a_(a), b_(b), inverted_(false) {}

    ~PidMotorWrapperGroup() override = default;

    void setVelocityPidConfig(const probot::control::PidConfig& cfg){
      if (a_) a_->setVelocityPidConfig(cfg);
      if (b_) b_->setVelocityPidConfig(cfg);
    }
    void setPositionPidConfig(const probot::control::PidConfig& cfg){
      if (a_) a_->setPositionPidConfig(cfg);
      if (b_) b_->setPositionPidConfig(cfg);
    }

    void setTimeoutMs(uint32_t ms){
      if (a_) a_->setTimeoutMs(ms);
      if (b_) b_->setTimeoutMs(ms);
    }

    bool setPower(float power) override {
      bool ok1 = a_ ? a_->setPower(power) : false;
      bool ok2 = b_ ? b_->setPower(power) : false;
      return ok1 && ok2;
    }

    float getPower() const override {
      float pa = a_ ? a_->getPower() : motor::IMotorController::kUnsupported;
      float pb = b_ ? b_->getPower() : motor::IMotorController::kUnsupported;
      if (pa == motor::IMotorController::kUnsupported &&
          pb == motor::IMotorController::kUnsupported) {
        return motor::IMotorController::kUnsupported;
      }
      if (pa == motor::IMotorController::kUnsupported) return pb;
      if (pb == motor::IMotorController::kUnsupported) return pa;
      return 0.5f * (pa + pb);
    }

    bool supportsVelocity() const override {
      bool ok1 = a_ ? a_->supportsVelocity() : false;
      bool ok2 = b_ ? b_->supportsVelocity() : false;
      return ok1 && ok2;
    }
    bool supportsPosition() const override {
      bool ok1 = a_ ? a_->supportsPosition() : false;
      bool ok2 = b_ ? b_->supportsPosition() : false;
      return ok1 && ok2;
    }

    bool setVelocity(float units_per_s) override {
      bool ok1 = a_ ? a_->setVelocity(units_per_s) : false;
      bool ok2 = b_ ? b_->setVelocity(units_per_s) : false;
      return ok1 && ok2;
    }

    bool setPosition(float units) override {
      bool ok1 = a_ ? a_->setPosition(units) : false;
      bool ok2 = b_ ? b_->setPosition(units) : false;
      return ok1 && ok2;
    }

    float getVelocity() const override {
      float va = a_ ? a_->getVelocity() : motor::IMotorController::kUnsupported;
      float vb = b_ ? b_->getVelocity() : motor::IMotorController::kUnsupported;
      if (va == motor::IMotorController::kUnsupported &&
          vb == motor::IMotorController::kUnsupported) {
        return motor::IMotorController::kUnsupported;
      }
      if (va == motor::IMotorController::kUnsupported) return vb;
      if (vb == motor::IMotorController::kUnsupported) return va;
      return 0.5f * (va + vb);
    }

    float getPosition() const override {
      float pa = a_ ? a_->getPosition() : motor::IMotorController::kUnsupported;
      float pb = b_ ? b_->getPosition() : motor::IMotorController::kUnsupported;
      if (pa == motor::IMotorController::kUnsupported &&
          pb == motor::IMotorController::kUnsupported) {
        return motor::IMotorController::kUnsupported;
      }
      if (pa == motor::IMotorController::kUnsupported) return pb;
      if (pb == motor::IMotorController::kUnsupported) return pa;
      return 0.5f * (pa + pb);
    }

    void setInverted(bool inverted) override {
      inverted_ = inverted;
      if (a_) a_->setInverted(inverted);
      if (b_) b_->setInverted(inverted);
    }
    bool getInverted() const override { return inverted_; }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (a_) a_->update(now_ms, dt_ms);
      if (b_) b_->update(now_ms, dt_ms);
    }

    float lastSetpoint() const {
      if (a_ && b_) return 0.5f * (a_->lastSetpoint() + b_->lastSetpoint());
      return a_ ? a_->lastSetpoint() : (b_ ? b_->lastSetpoint() : 0.0f);
    }
    float lastMeasurement() const {
      if (a_ && b_) return 0.5f * (a_->lastMeasurement() + b_->lastMeasurement());
      return a_ ? a_->lastMeasurement() : (b_ ? b_->lastMeasurement() : 0.0f);
    }
    float lastOutput() const {
      if (a_ && b_) return 0.5f * (a_->lastOutput() + b_->lastOutput());
      return a_ ? a_->lastOutput() : (b_ ? b_->lastOutput() : 0.0f);
    }
    ControlType activeMode() const { return a_ ? a_->activeMode() : ControlType::kPercent; }
    bool isAtTarget(float tolerance) const {
      bool okA = a_ ? a_->isAtTarget(tolerance) : true;
      bool okB = b_ ? b_->isAtTarget(tolerance) : true;
      return okA && okB;
    }

  private:
    PidMotorWrapper* a_;
    PidMotorWrapper* b_;
    bool inverted_;
  };
} // namespace probot::control
