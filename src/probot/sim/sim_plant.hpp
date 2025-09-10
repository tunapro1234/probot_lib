#pragma once
#include <probot/core/scheduler.hpp>
#include <probot/sim/sim_motor.hpp>
#include <probot/sim/sim_encoder.hpp>

namespace probot::sim {
  class SimPlant : public ::control::IUpdatable {
  public:
    SimPlant(SimMotor* m, SimEncoder* e) : motor_(m), enc_(e) {}

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms;
      float dt_s = dt_ms * 0.001f;
      if (dt_s <= 0.0f) dt_s = 0.001f;

      const float max_tps = 800.0f;
      float cmd = (float)motor_->appliedPower() / 1000.0f;
      float target_tps = cmd * max_tps;

      const float tau = 0.2f;
      float alpha = dt_s / tau;
      if (alpha > 1.0f) alpha = 1.0f;
      speed_tps_ += (target_tps - speed_tps_) * alpha;

      float delta_ticks = speed_tps_ * dt_s + frac_ticks_;
      int32_t di = (int32_t)delta_ticks;
      frac_ticks_ = delta_ticks - (float)di;

      ticks_ += di;
      enc_->setTicks(ticks_);
      enc_->setTicksPerSecond((int32_t)(speed_tps_));
    }

  private:
    SimMotor*   motor_;
    SimEncoder* enc_;
    float   speed_tps_ = 0.0f;
    float   frac_ticks_= 0.0f;
    int32_t ticks_     = 0;
  };
} // namespace probot::sim 