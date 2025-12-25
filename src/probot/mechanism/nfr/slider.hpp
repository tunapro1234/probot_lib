#pragma once
#include <probot/mechanism/slider.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {
  class NfrBasicSlider : public probot::mechanism::Slider {
  public:
    explicit NfrBasicSlider(probot::control::PidMotorController* controller)
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
    explicit NfrSlider(probot::control::PidMotorController* controller)
    : NfrBasicSlider(controller) {
      if (!controller) return;
      controller->setVelocityPidConfig(detail::kDefaultVelocityPid);
      controller->setPositionPidConfig(detail::kDefaultPositionPid);
    }

  };
}
