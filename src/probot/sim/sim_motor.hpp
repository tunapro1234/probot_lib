#pragma once
#include <probot/devices/motors/motor.hpp>

namespace probot::sim {
  class SimMotor : public probot::motor::IMotor {
  public:
    bool claim(void* owner) override {
      if (owner_ == nullptr || owner_ == owner){ owner_ = owner; return true; }
      return false;
    }
    void release(void* owner) override { if (owner_ == owner) owner_ = nullptr; }
    bool setPower(int16_t power, void* owner) override {
      if (owner_ != owner) return false;
      if (power < -1000) power = -1000; else if (power > 1000) power = 1000;
      last_cmd_ = power;
      return true;
    }
    bool isClaimed() const override { return owner_ != nullptr; }
    void* currentOwner() const override { return owner_; }

    void setInverted(bool inverted) override { inverted_ = inverted; }
    bool getInverted() const override { return inverted_; }

    int16_t appliedPower() const { return inverted_ ? (int16_t)(-last_cmd_) : last_cmd_; }

  private:
    void*   owner_   = nullptr;
    bool    inverted_= false;
    int16_t last_cmd_= 0;
  };
} // namespace probot::sim 