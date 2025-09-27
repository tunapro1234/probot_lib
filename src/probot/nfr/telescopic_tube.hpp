#pragma once
#include <probot/controllers/telescopic_tube.hpp>
#include <probot/nfr/common.hpp>

namespace probot::nfr {
  class NfrTelescopicTube : public probot::controllers::TelescopicTube {
  public:
    explicit NfrTelescopicTube(probot::controllers::IMotorController* controller)
    : probot::controllers::TelescopicTube(controller) {
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
