#pragma once
#include <cmath>

namespace probot::control {
  class BangBangController {
  public:
    BangBangController(float tolerance = 0.0f)
    : tolerance_(tolerance) {}

    void setTolerance(float tol){ tolerance_ = tol >= 0.0f ? tol : 0.0f; }
    float tolerance() const { return tolerance_; }

    inline float calculate(float measurement, float setpoint) const {
      float error = setpoint - measurement;
      if (error > tolerance_) return 1.0f;
      if (error < -tolerance_) return -1.0f;
      return 0.0f;
    }

    inline bool atSetpoint(float measurement, float setpoint) const {
      return std::fabs(setpoint - measurement) <= tolerance_;
    }

  private:
    float tolerance_;
  };
}
