#pragma once
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::sim {
  class SimMotor : public probot::motor::IMotorDriver {
  public:
    bool claim(void* owner) override {
      if (owner_ == nullptr || owner_ == owner){ owner_ = owner; return true; }
      return false;
    }
    void release(void* owner) override { if (owner_ == owner) owner_ = nullptr; }
    bool setPower(float power, void* owner) override {
      if (owner_ != owner) return false;
      if (power < -1.0f) power = -1.0f; else if (power > 1.0f) power = 1.0f;
      last_cmd_ = power;
      return true;
    }
    bool isClaimed() const override { return owner_ != nullptr; }
    void* currentOwner() const override { return owner_; }

    void setInverted(bool inverted) override { inverted_ = inverted; }
    bool getInverted() const override { return inverted_; }

    float appliedPower() const { return inverted_ ? -last_cmd_ : last_cmd_; }

  private:
    void*   owner_   = nullptr;
    bool    inverted_= false;
    float   last_cmd_= 0.0f;
  };
} // namespace probot::sim

// Backward compatibility alias
namespace probot::motor {
  using NullMotor = probot::sim::SimMotor;
} 
