#pragma once
#include <probot/mechanism/telescopic_tube.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrBasicTelescopicTube : public probot::mechanism::TelescopicTube {
  public:
    explicit NfrBasicTelescopicTube(probot::control::IMotorController* controller)
    : probot::mechanism::TelescopicTube(controller) {
      setUnitsToTicks(kTicksPerUnit);
      setStageConfiguration(kStageCount, kStageLengthUnits);
    }

  protected:
    static constexpr float kTicksPerUnit    = 80.0f; // ticks per cm
    static constexpr int   kStageCount      = 3;
    static constexpr float kStageLengthUnits= 40.0f; // cm per stage
  };

  class NfrTelescopicTube : public NfrBasicTelescopicTube {
  public:
    explicit NfrTelescopicTube(probot::control::IMotorController* controller)
    : NfrBasicTelescopicTube(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      setPidConfig(detail::kDefaultPositionPid, detail::kPositionSlot);
      setMotionProfile(probot::control::MotionProfileType::kTrapezoid,
                       {0.5f, 1.5f, 0.0f});
      setSlewRateLimit(25.0f);
    }
  };
}
