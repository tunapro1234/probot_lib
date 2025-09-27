#pragma once
#include <probot/controllers/turret.hpp>
#include <probot/nfr/common.hpp>

namespace probot::nfr {
  class NfrTurret : public probot::controllers::Turret {
  public:
    explicit NfrTurret(probot::controllers::IMotorController* controller)
    : probot::controllers::Turret(controller) {
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
