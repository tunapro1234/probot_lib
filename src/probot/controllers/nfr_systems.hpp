#pragma once
#include <probot/controllers/slider.hpp>
#include <probot/controllers/elevator.hpp>
#include <probot/controllers/turret.hpp>
#include <probot/controllers/arm.hpp>
#include <probot/controllers/telescopic_tube.hpp>

namespace probot::nfr {

  namespace detail {
    constexpr int kVelocitySlot = 0;
    constexpr int kPositionSlot = 1;
    constexpr probot::control::PidConfig kDefaultVelocityPid{0.2f, 0.0f, 0.0f, -1.0f, 1.0f};
    constexpr probot::control::PidConfig kDefaultPositionPid{0.6f, 0.0f, 0.02f, -1.0f, 1.0f};
  }

  class NfrSlider : public probot::controllers::Slider {
  public:
    explicit NfrSlider(probot::controllers::IMotorController* controller)
    : probot::controllers::Slider(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      setLengthToTicks(kTicksPerUnit);
      setLengthLimits(0.0f, kMaxLengthUnits);
    }
  private:
    static constexpr float kTicksPerUnit = 48.0f;   // ticks per cm
    static constexpr float kMaxLengthUnits = 50.0f; // cm
  };

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
    static constexpr float kTicksPerUnit = 60.0f;   // ticks per cm
    static constexpr float kMaxHeightUnits = 120.0f; // cm
  };

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
    static constexpr float kTicksPerUnit = 80.0f;   // ticks per cm
    static constexpr int   kStageCount = 3;
    static constexpr float kStageLengthUnits = 40.0f; // cm per stage
  };

} // namespace probot::nfr
