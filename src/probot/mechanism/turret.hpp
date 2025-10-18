#pragma once
#include <algorithm>
#include <math.h>
#include <probot/control/imotor_controller.hpp>
#include <probot/control/limiters/slew_rate_limiter.hpp>
#include <probot/logging/logger.hpp>
#include <probot/logging/telemetry_profiles.hpp>

namespace probot::mechanism {

  struct ITurret : public ::control::IUpdatable {
    virtual void setTargetAngleDeg(float degrees) = 0;
    virtual float getTargetAngleDeg() const = 0;
    virtual void setAngleLimits(float min_deg, float max_deg) = 0;
    virtual void setDegreesToTicks(float ticks_per_degree) = 0;
    virtual float getCurrentAngleDeg() const = 0;
    virtual bool isAtAngle(float tolerance_deg) const = 0;
    virtual ~ITurret() {}
  };

  class Turret : public ITurret {
  public:
    explicit Turret(probot::control::IMotorController* controller)
    : controller_(controller),
      ticks_per_degree_(1.0f),
      target_angle_(0.0f),
      min_angle_(-180.0f),
      max_angle_(180.0f),
      has_limits_(false),
      position_slot_(0),
      pid_config_{0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f},
      profile_type_(probot::control::MotionProfileType::kNone),
      profile_cfg_deg_{},
      profile_dirty_(true),
      slew_rate_deg_per_s_(0.0f),
      slew_limiter_(0.0f, 0.0f),
      limiter_initialized_(false) {
      if (controller_) {
        controller_->selectDefaultSlot(probot::control::ControlType::kPosition, position_slot_);
        controller_->setPidSlotConfig(position_slot_, pid_config_);
      }
      probot::logging::SourceRegistration reg{
        "turret",
        nullptr,
        probot::logging::Priority::kBackground,
        false,
        false,
        probot::logging::profiles::turretDynamic,
        nullptr
      };
      probot::logging::registerSource(this, reg);
    }

    ~Turret() override {
      probot::logging::unregisterSource(this);
    }

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
      profile_dirty_ = true;
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

    void setPidConfig(const probot::control::PidConfig& cfg, int slot = 0) {
      position_slot_ = std::clamp(slot, 0, 3);
      pid_config_ = cfg;
      if (controller_) {
        controller_->setPidSlotConfig(position_slot_, pid_config_);
        controller_->selectDefaultSlot(probot::control::ControlType::kPosition, position_slot_);
      }
    }

    void setMotionProfile(probot::control::MotionProfileType type,
                          const probot::control::MotionProfileConfig& cfg) {
      if (type != probot::control::MotionProfileType::kTrapezoid &&
          type != probot::control::MotionProfileType::kNone){
        type = probot::control::MotionProfileType::kNone;
      }
      profile_type_ = type;
      profile_cfg_deg_ = cfg;
      profile_dirty_ = true;
    }

    void setSlewRateLimit(float degrees_per_second) {
      slew_rate_deg_per_s_ = std::max(degrees_per_second, 0.0f);
      limiter_initialized_ = false;
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms;
      if (!controller_) return;
      ensureMotionProfileApplied();

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
      controller_->setSetpoint(target_ticks, probot::control::ControlType::kPosition, position_slot_);
    }

  private:
    void ensureMotionProfileApplied(){
      if (!profile_dirty_ || !controller_) return;
      profile_dirty_ = false;
      if (profile_type_ == probot::control::MotionProfileType::kNone ||
          profile_cfg_deg_.maxVelocity <= 0.0f ||
          profile_cfg_deg_.maxAcceleration <= 0.0f){
        controller_->setMotionProfile(probot::control::MotionProfileType::kNone);
        controller_->setMotionProfileConfig({0.0f, 0.0f, 0.0f});
        return;
      }
      probot::control::MotionProfileConfig cfgTicks;
      cfgTicks.maxVelocity = profile_cfg_deg_.maxVelocity * ticks_per_degree_;
      cfgTicks.maxAcceleration = profile_cfg_deg_.maxAcceleration * ticks_per_degree_;
      cfgTicks.maxJerk = 0.0f;
      controller_->setMotionProfile(profile_type_);
      controller_->setMotionProfileConfig(cfgTicks);
    }

    void initializeLimiterIfNeeded(){
      if (!limiter_initialized_){
        float current = getCurrentAngleDeg();
        float initial = std::isfinite(current) ? current : target_angle_;
        slew_limiter_.reset(initial);
        limiter_initialized_ = true;
      }
    }

    probot::control::IMotorController* controller_;
    float ticks_per_degree_;
    float target_angle_;
    float min_angle_;
    float max_angle_;
    bool  has_limits_;

    int position_slot_;
    probot::control::PidConfig pid_config_;

    probot::control::MotionProfileType profile_type_;
    probot::control::MotionProfileConfig profile_cfg_deg_;
    bool profile_dirty_;

    float slew_rate_deg_per_s_;
    probot::control::limiters::SlewRateLimiter slew_limiter_;
    bool limiter_initialized_;
  };

} // namespace probot::mechanism
