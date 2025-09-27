#pragma once
#include <probot/controllers/arm.hpp>
#include <probot/nfr/common.hpp>

namespace probot::nfr {
  class NfrArm : public probot::controllers::Arm {
  public:
    explicit NfrArm(probot::controllers::IMotorController* controller)
    : probot::controllers::Arm(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      setDegreesToTicks(kTicksPerDegree);
      setAngleLimits(-120.0f, 120.0f);
    }

  private:
    static constexpr float kTicksPerDegree = 6.0f;
  };
}
