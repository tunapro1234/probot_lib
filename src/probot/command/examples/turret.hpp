#pragma once
#include <algorithm>
#include <math.h>
#include <probot/command/subsystem.hpp>
#include <probot/control/pid_motor_wrapper.hpp>
#include <probot/control/limiters/slew_rate_limiter.hpp>

namespace probot::command::examples {

  // NOTE: This mechanism example is disabled (not tested yet).
#if 0
  struct ITurret {
    virtual void setTargetAngleDeg(float degrees) = 0;
    virtual float getTargetAngleDeg() const = 0;
    virtual void setAngleLimits(float min_deg, float max_deg) = 0;
    virtual void setDegreesToTicks(float ticks_per_degree) = 0;
    virtual float getCurrentAngleDeg() const = 0;
    virtual bool isAtAngle(float tolerance_deg) const = 0;
    virtual ~ITurret() {}
  };

  class Turret : public probot::command::SubsystemBase, public ITurret {
  public:
    explicit Turret(probot::control::PidMotorWrapper* controller)
    : probot::command::SubsystemBase("Turret"),
      controller_(controller),
      ticks_per_degree_(1.0f),
      target_angle_(0.0f),
      min_angle_(-180.0f),
      max_angle_(180.0f),
      has_limits_(false),
      slew_rate_deg_per_s_(0.0f),
      slew_limiter_(0.0f, 0.0f),
      limiter_initialized_(false) {
    }

    ~Turret() override = default;

    void setTargetAngleDeg(float degrees) override {
      if (has_limits_){
        if (degrees < min_angle_) degrees = min_angle_;
        if (degrees > max_angle_) degrees = max_angle_;
      }
      target_angle_ = degrees;
    }

    float getTargetAngleDeg() const override { return target_angle_; }

    void setAngleLimits(float min_deg, float max_deg) override {
      if (max_deg < min_deg){
        float tmp = min_deg; min_deg = max_deg; max_deg = tmp;
      }
      min_angle_ = min_deg;
      max_angle_ = max_deg;
      has_limits_ = true;
      setTargetAngleDeg(target_angle_);
    }

    void setDegreesToTicks(float ticks_per_degree) override {
      if (ticks_per_degree <= 0.0f) ticks_per_degree = 1.0f;
      ticks_per_degree_ = ticks_per_degree;
    }

    float getCurrentAngleDeg() const override {
      if (!controller_) return 0.0f;
      float measurement = controller_->lastMeasurement();
      return (ticks_per_degree_ != 0.0f) ? (measurement / ticks_per_degree_) : measurement;
    }

    bool isAtAngle(float tolerance_deg) const override {
      if (!controller_) return false;
      return fabsf(getCurrentAngleDeg() - target_angle_) <= tolerance_deg;
    }

    void setPositionPidConfig(const probot::control::PidConfig& cfg) {
      if (controller_) controller_->setPositionPidConfig(cfg);
    }

    void setSlewRateLimit(float degrees_per_second) {
      slew_rate_deg_per_s_ = std::max(degrees_per_second, 0.0f);
      limiter_initialized_ = false;
    }

    void periodic(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms;
      if (!controller_) return;

      float dt_s = dt_ms > 0 ? dt_ms * 0.001f : 0.02f;
      float commanded = target_angle_;
      if (slew_rate_deg_per_s_ > 0.0f){
        initializeLimiterIfNeeded();
        slew_limiter_.setMaxRate(slew_rate_deg_per_s_);
        commanded = slew_limiter_.calculate(target_angle_, dt_s);
      } else if (!limiter_initialized_) {
        slew_limiter_.reset(commanded);
        limiter_initialized_ = true;
      }

      float target_ticks = commanded * ticks_per_degree_;
      controller_->setPosition(target_ticks);
    }

  private:
    void initializeLimiterIfNeeded(){
      if (!limiter_initialized_){
        float current = getCurrentAngleDeg();
        float initial = std::isfinite(current) ? current : target_angle_;
        slew_limiter_.reset(initial);
        limiter_initialized_ = true;
      }
    }

    probot::control::PidMotorWrapper* controller_;
    float ticks_per_degree_;
    float target_angle_;
    float min_angle_;
    float max_angle_;
    bool  has_limits_;

    float slew_rate_deg_per_s_;
    probot::control::limiters::SlewRateLimiter slew_limiter_;
    bool limiter_initialized_;
  };
#endif

} // namespace probot::command::examples
