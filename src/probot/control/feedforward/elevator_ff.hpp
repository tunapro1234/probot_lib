#pragma once
#include <cmath>

namespace probot::control::feedforward {
  class ElevatorFF {
  public:
    constexpr ElevatorFF(float kS, float kV, float kA, float kG)
    : kS_(kS), kV_(kV), kA_(kA), kG_(kG) {}

    constexpr float ks() const { return kS_; }
    constexpr float kv() const { return kV_; }
    constexpr float ka() const { return kA_; }
    constexpr float kg() const { return kG_; }

    constexpr void setKs(float kS) { kS_ = kS; }
    constexpr void setKv(float kV) { kV_ = kV; }
    constexpr void setKa(float kA) { kA_ = kA; }
    constexpr void setKg(float kG) { kG_ = kG; }

    inline float calculate(float velocityUnitsPerSec,
                           float accelerationUnitsPerSec2 = 0.0f) const {
      float sign = (velocityUnitsPerSec >= 0.0f) ? 1.0f : -1.0f;
      return kS_ * sign + kG_ + kV_ * velocityUnitsPerSec + kA_ * accelerationUnitsPerSec2;
    }

  private:
    float kS_;
    float kV_;
    float kA_;
    float kG_;
  };
}
