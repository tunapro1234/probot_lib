#pragma once
#include <probot/mechanism/slider.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrSlider : public probot::mechanism::Slider {
  public:
    explicit NfrSlider(probot::control::IMotorController* controller)
    : probot::mechanism::Slider(controller) {
      if (!controller) return;
      controller->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                                    detail::kPositionSlot, detail::kDefaultPositionPid);
      setLengthToTicks(kTicksPerUnit);
      setLengthLimits(0.0f, kMaxLengthUnits);
    }

  private:
    static constexpr float kTicksPerUnit    = 48.0f; // ticks per cm
    static constexpr float kMaxLengthUnits  = 50.0f; // cm
  };
}
