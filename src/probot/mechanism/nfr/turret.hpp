#pragma once
#include <probot/mechanism/turret.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrBasicTurret : public probot::mechanism::Turret {
  public:
    explicit NfrBasicTurret(probot::control::PidMotorController* controller)
    : probot::mechanism::Turret(controller) {
      setDegreesToTicks(kTicksPerDegree);
      setAngleLimits(-180.0f, 180.0f);
    }

  protected:
    static constexpr float kTicksPerDegree = 5.0f;
  };

  class NfrTurret : public NfrBasicTurret {
  public:
    explicit NfrTurret(probot::control::PidMotorController* controller)
    : NfrBasicTurret(controller) {
      if (!controller) return;
      controller->setVelocityPidConfig(detail::kDefaultVelocityPid);
      setPositionPidConfig(detail::kDefaultPositionPid);
      setSlewRateLimit(180.0f);
    }
  };
}
