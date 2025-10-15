#pragma once
#include <probot/mechanism/slider.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrBasicSlider : public probot::mechanism::Slider {
  public:
    explicit NfrBasicSlider(probot::control::IMotorController* controller)
    : probot::mechanism::Slider(controller) {
      setLengthToTicks(kTicksPerUnit);
      setLengthLimits(0.0f, kMaxLengthUnits);
    }

  protected:
    static constexpr float kTicksPerUnit    = 48.0f; // ticks per cm
    static constexpr float kMaxLengthUnits  = 50.0f; // cm
  };

  class NfrSlider : public NfrBasicSlider {
  public:
    explicit NfrSlider(probot::control::IMotorController* controller)
    : NfrBasicSlider(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      controller->setMotionProfile(probot::control::MotionProfileType::kTrapezoid);
      controller->setMotionProfileConfig({0.6f * kTicksPerUnit, 2.0f * kTicksPerUnit, 0.0f});
    }

  };
}
