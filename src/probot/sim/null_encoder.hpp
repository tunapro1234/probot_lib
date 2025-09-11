#ifndef PROBOT_SIM_NULL_ENCODER_HPP
#define PROBOT_SIM_NULL_ENCODER_HPP
#pragma once
#include <probot/sensors/encoder.hpp>

namespace probot::sensors {

class NullEncoder : public IEncoder {
public:
  NullEncoder() : _pos(0), _vel(0) {}

  int32_t readTicks() override { return _pos; }
  int32_t readTicksPerSecond() override { return _vel; }

  // Helpers to simulate if needed
  void _setSim(int32_t pos, int32_t vel){ _pos = pos; _vel = vel; }

private:
  int32_t _pos;
  int32_t _vel;
};

} // namespace probot::sensors

#endif // PROBOT_SIM_NULL_ENCODER_HPP 