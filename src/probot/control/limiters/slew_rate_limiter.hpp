#pragma once
#include <cmath>

namespace probot::control::limiters {
  class SlewRateLimiter {
  public:
    explicit SlewRateLimiter(float maxRate, float initial = 0.0f)
    : maxRate_(std::fabs(maxRate)), prev_(initial) {}

    void reset(float value){ prev_ = value; }
    float last() const { return prev_; }
    float maxRate() const { return maxRate_; }
    void setMaxRate(float rate){ maxRate_ = std::fabs(rate); }

    inline float calculate(float input, float dtSeconds){
      if (dtSeconds <= 0.0f){ prev_ = input; return prev_; }
      float maxDelta = maxRate_ * dtSeconds;
      float delta = input - prev_;
      if (delta > maxDelta) delta = maxDelta;
      else if (delta < -maxDelta) delta = -maxDelta;
      prev_ += delta;
      return prev_;
    }

  private:
    float maxRate_;
    float prev_;
  };
}
