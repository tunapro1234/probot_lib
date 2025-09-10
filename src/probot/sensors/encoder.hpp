#pragma once
#include <stdint.h>

namespace probot::sensors {
  struct IEncoder {
    virtual int32_t readTicks() = 0;               // absolute or signed cumulative ticks
    virtual int32_t readTicksPerSecond() = 0;      // smoothed velocity estimate
    virtual ~IEncoder() {}
  };
} // namespace probot::sensors 