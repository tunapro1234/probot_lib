#pragma once
#include <probot/mechanism/elevator.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrElevator : public probot::mechanism::Elevator {
  public:
    explicit NfrElevator(probot::control::PidMotorController* controller)
    : probot::mechanism::Elevator(controller) {
      if (!controller) return;
      controller->setVelocityPidConfig(detail::kDefaultVelocityPid);
      controller->setPositionPidConfig(detail::kDefaultPositionPid);
      setUnitsToTicks(kTicksPerUnit);
      setHeightLimits(0.0f, kMaxHeightUnits);
    }

  private:
    static constexpr float kTicksPerUnit   = 60.0f;  // ticks per cm
    static constexpr float kMaxHeightUnits = 120.0f; // cm
  };
}
