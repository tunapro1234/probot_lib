#pragma once
#include <stdint.h>
#include <probot/control/pid.hpp>
#include <probot/devices/motors/imotor_driver.hpp>
#include <probot/core/scheduler.hpp>

namespace probot::control {
  enum class ControlType : uint8_t {
    kVelocity = 0,
    kPosition = 1,
    kPercent = 2
  };

  struct IMotorController : public probot::motor::IMotorDriver, public ::control::IUpdatable {
    virtual void setSetpoint(float value, ControlType mode, int slot = -1) = 0;
    virtual void setTimeoutMs(uint32_t ms) = 0;
    virtual void setPidSlotConfig(int slot, const probot::control::PidConfig& cfg) = 0;
    virtual void selectDefaultSlot(ControlType mode, int slot) = 0;
    virtual int defaultSlot(ControlType mode) const = 0;
    virtual float lastSetpoint() const = 0;
    virtual float lastMeasurement() const = 0;
    virtual float lastOutput() const = 0;
    virtual ControlType activeMode() const = 0;
    virtual bool isAtTarget(float tolerance) const = 0;

    virtual ~IMotorController() {}

    void configurePidSlots(int velocitySlot,
                           const probot::control::PidConfig& velocityCfg,
                           int positionSlot,
                           const probot::control::PidConfig& positionCfg) {
      setPidSlotConfig(velocitySlot, velocityCfg);
      setPidSlotConfig(positionSlot, positionCfg);
      selectDefaultSlot(ControlType::kVelocity, velocitySlot);
      selectDefaultSlot(ControlType::kPosition, positionSlot);
    }
  };
}
