#pragma once
#include <probot/mechanism/turret.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrTurret : public probot::mechanism::Turret {
  public:
    explicit NfrTurret(probot::control::IMotorController* controller)
    : probot::mechanism::Turret(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      setDegreesToTicks(kTicksPerDegree);
      setAngleLimits(-180.0f, 180.0f);
    }

  private:
    static constexpr float kTicksPerDegree = 5.0f;
  };
}
