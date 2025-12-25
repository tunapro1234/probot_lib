#pragma once
#include <probot/mechanism/arm.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrArm : public probot::mechanism::Arm {
  public:
    explicit NfrArm(probot::control::PidMotorController* controller)
    : probot::mechanism::Arm(controller) {
      if (!controller) return;
      controller->setVelocityPidConfig(detail::kDefaultVelocityPid);
      controller->setPositionPidConfig(detail::kDefaultPositionPid);
      setDegreesToTicks(kTicksPerDegree);
      setAngleLimits(-120.0f, 120.0f);
    }

  private:
    static constexpr float kTicksPerDegree = 6.0f;
  };
}
