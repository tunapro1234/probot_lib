#pragma once
#include <stdint.h>
#include <probot/devices/motors/motor.hpp>
#include <probot/core/scheduler.hpp>

namespace probot::controllers {
  enum class ControlType : uint8_t {
    kVelocity = 0,
    kPosition = 1,
    kPercent = 2
  };

  struct IMotorController : public probot::motor::IMotorDriver, public ::control::IUpdatable {
    virtual void setSetpoint(float value, ControlType mode, int slot = -1) = 0;
    virtual void setTimeoutMs(uint32_t ms) = 0;

    virtual ~IMotorController() {}
  };
}
