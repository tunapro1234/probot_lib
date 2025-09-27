#pragma once
#include <cmath>

namespace probot::control::feedforward {
  class SimpleMotorFF {
  public:
    constexpr SimpleMotorFF(float kS, float kV, float kA)
    : kS_(kS), kV_(kV), kA_(kA) {}

    constexpr float ks() const { return kS_; }
    constexpr float kv() const { return kV_; }
    constexpr float ka() const { return kA_; }

    constexpr void setKs(float kS) { kS_ = kS; }
    constexpr void setKv(float kV) { kV_ = kV; }
    constexpr void setKa(float kA) { kA_ = kA; }

    // velocity in units per second, acceleration in units per second^2;
    // voltage output assumes normalized range (-1..1) for simplicity.
    inline float calculate(float velocity, float acceleration = 0.0f) const {
      float sign = (velocity == 0.0f) ? 0.0f : std::copysign(1.0f, velocity);
      return kS_ * sign + kV_ * velocity + kA_ * acceleration;
    }

  private:
    float kS_;
    float kV_;
    float kA_;
  };
}
