#pragma once
#include <probot/sensors/encoder.hpp>

namespace probot::test {
  class TestEncoder : public probot::sensors::IEncoder {
  public:
    int32_t readTicks() override { return ticks_; }
    int32_t readTicksPerSecond() override { return tps_; }

    void setTicks(int32_t ticks){ ticks_ = ticks; }
    void setTicksPerSecond(int32_t tps){ tps_ = tps; }
  private:
    int32_t ticks_ = 0;
    int32_t tps_   = 0;
  };
} // namespace probot::test 
