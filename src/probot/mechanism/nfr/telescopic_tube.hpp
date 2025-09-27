#pragma once
#include <probot/mechanism/telescopic_tube.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrTelescopicTube : public probot::mechanism::TelescopicTube {
  public:
    explicit NfrTelescopicTube(probot::control::IMotorController* controller)
    : probot::mechanism::TelescopicTube(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      setUnitsToTicks(kTicksPerUnit);
      setStageConfiguration(kStageCount, kStageLengthUnits);
    }

  private:
    static constexpr float kTicksPerUnit    = 80.0f; // ticks per cm
    static constexpr int   kStageCount      = 3;
    static constexpr float kStageLengthUnits= 40.0f; // cm per stage
  };
}
