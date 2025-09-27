#pragma once
#include <probot/controllers/slider.hpp>
#include <probot/nfr/common.hpp>

namespace probot::nfr {
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
    static constexpr float kTicksPerUnit    = 48.0f; // ticks per cm
    static constexpr float kMaxLengthUnits  = 50.0f; // cm
  };
}
