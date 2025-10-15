#pragma once
#include <probot/mechanism/turret.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrBasicTurret : public probot::mechanism::Turret {
  public:
    explicit NfrBasicTurret(probot::control::IMotorController* controller)
    : probot::mechanism::Turret(controller) {
      setDegreesToTicks(kTicksPerDegree);
      setAngleLimits(-180.0f, 180.0f);
    }

  protected:
    static constexpr float kTicksPerDegree = 5.0f;
  };

  class NfrTurret : public NfrBasicTurret {
  public:
    explicit NfrTurret(probot::control::IMotorController* controller)
    : NfrBasicTurret(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      setPidConfig(detail::kDefaultPositionPid, detail::kPositionSlot);
      setMotionProfile(probot::control::MotionProfileType::kTrapezoid,
                       {90.0f, 360.0f, 0.0f});
      setSlewRateLimit(180.0f);
    }
  };
}
