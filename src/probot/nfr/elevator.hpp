#pragma once
#include <probot/controllers/elevator.hpp>
#include <probot/nfr/common.hpp>

namespace probot::nfr {
  class NfrElevator : public probot::controllers::Elevator {
  public:
    explicit NfrElevator(probot::controllers::IMotorController* controller)
    : probot::controllers::Elevator(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      setUnitsToTicks(kTicksPerUnit);
      setHeightLimits(0.0f, kMaxHeightUnits);
    }

  private:
    static constexpr float kTicksPerUnit   = 60.0f;  // ticks per cm
    static constexpr float kMaxHeightUnits = 120.0f; // cm
  };
}
