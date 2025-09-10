#pragma once
#include <stdint.h>

namespace probot::control {
  struct PidConfig {
    float kp;
    float ki;
    float kd;
    float out_min;   // e.g. -1000
    float out_max;   // e.g. +1000
  };

  class PID {
  public:
    explicit PID(const PidConfig& cfg)
    : cfg_(cfg), integ_(0.0f), prev_err_(0.0f), has_prev_(false),
      continuous_enabled_(false), min_input_(0.0f), max_input_(0.0f),
      input_range_(0.0f), half_range_(0.0f) {}

    void reset(){ integ_ = 0.0f; prev_err_ = 0.0f; has_prev_ = false; }

    void setConfig(const PidConfig& cfg){ cfg_ = cfg; }

    // Enable/disable continuous input (wrap-around) like WPILib
    void setContinuous(bool enabled, float min_input, float max_input){
      continuous_enabled_ = enabled;
      min_input_ = min_input;
      max_input_ = max_input;
      input_range_ = max_input_ - min_input_;
      half_range_  = input_range_ * 0.5f;
    }
    void enableContinuousInput(float min_input, float max_input){ setContinuous(true, min_input, max_input); }
    void disableContinuousInput(){ continuous_enabled_ = false; }

    // Convenience: compute error from setpoint/measurement honoring continuous mode
    float stepFrom(float measurement, float setpoint, float dt_s){
      float error = setpoint - measurement;
      if (continuous_enabled_ && input_range_ > 0.0f){
        while (error >  half_range_) error -= input_range_;
        while (error < -half_range_) error += input_range_;
      }
      return step(error, dt_s);
    }

    float step(float error, float dt_s){
      float p = cfg_.kp * error;
      integ_ += cfg_.ki * error * dt_s;
      // Anti-windup by clamping integral within output range
      if (integ_ > cfg_.out_max) integ_ = cfg_.out_max;
      if (integ_ < cfg_.out_min) integ_ = cfg_.out_min;

      float d = 0.0f;
      if (has_prev_){ d = cfg_.kd * (error - prev_err_) / (dt_s > 1e-6f ? dt_s : 1e-6f); }
      prev_err_ = error;
      has_prev_ = true;

      float out = p + integ_ + d;
      if (out > cfg_.out_max) out = cfg_.out_max;
      if (out < cfg_.out_min) out = cfg_.out_min;
      return out;
    }

  private:
    PidConfig cfg_;
    float     integ_;
    float     prev_err_;
    bool      has_prev_;

    bool  continuous_enabled_;
    float min_input_;
    float max_input_;
    float input_range_;
    float half_range_;
  };
} // namespace probot::control 