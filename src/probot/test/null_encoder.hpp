#pragma once
#include <probot/sensors/encoder.hpp>

namespace probot::test {
  class NullEncoder : public probot::sensors::IEncoder {
  public:
    int32_t readTicks() override { return 0; }
    int32_t readTicksPerSecond() override { return 0; }
  };
}
