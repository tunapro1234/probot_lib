#pragma once
#include <stdint.h>

namespace probot::control {
  enum class ControlType : uint8_t {
    kVelocity = 0,
    kPosition = 1,
    kPercent = 2
  };
}
