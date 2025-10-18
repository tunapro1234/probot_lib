#pragma once
#include <algorithm>
#include <math.h>
#include <probot/control/imotor_controller.hpp>
#include <probot/control/limiters/slew_rate_limiter.hpp>
#include <probot/logging/logger.hpp>
#include <probot/logging/telemetry_profiles.hpp>

namespace probot::mechanism {

  struct ITelescopicTube : public ::control::IUpdatable {
    virtual void setTargetExtension(float units) = 0;
    virtual float getTargetExtension() const = 0;
    virtual void setStageConfiguration(int stage_count, float stage_length_units) = 0;
    virtual void setUnitsToTicks(float ticks_per_unit) = 0;
    virtual float getCurrentExtension() const = 0;
    virtual bool isAtExtension(float tolerance_units) const = 0;
    virtual ~ITelescopicTube() {}
  };

  class TelescopicTube : public ITelescopicTube {
  public:
    explicit TelescopicTube(probot::control::IMotorController* controller)
    : controller_(controller),
      ticks_per_unit_(1.0f),
      target_extension_(0.0f),
      stage_length_(0.0f),
      stage_count_(0),
      has_stage_limits_(false),
      position_slot_(0),
      pid_config_{0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f},
      profile_type_(probot::control::MotionProfileType::kNone),
      profile_cfg_units_{},
      profile_dirty_(true),
      slew_rate_units_per_s_(0.0f),
      slew_limiter_(0.0f, 0.0f),
      limiter_initialized_(false) {
      if (controller_) {
        controller_->selectDefaultSlot(probot::control::ControlType::kPosition, position_slot_);
        controller_->setPidSlotConfig(position_slot_, pid_config_);
      }
      probot::logging::SourceRegistration reg{
        "telescopic_tube",
        nullptr,
        probot::logging::Priority::kBackground,
        false,
        false,
        probot::logging::profiles::telescopicDynamic,
        nullptr
      };
      probot::logging::registerSource(this, reg);
    }

    ~TelescopicTube() override {
      probot::logging::unregisterSource(this);
    }

    void setTargetExtension(float units) override {
      if (has_stage_limits_){
        float max_ext = stage_length_ * static_cast<float>(stage_count_);
        units = std::clamp(units, 0.0f, max_ext);
      }
      target_extension_ = units;
    }

    float getTargetExtension() const override { return target_extension_; }

    void setStageConfiguration(int stage_count, float stage_length_units) override {
      stage_count_ = std::max(stage_count, 0);
      stage_length_ = stage_length_units;
      has_stage_limits_ = stage_count_ > 0 && stage_length_ > 0.0f;
      setTargetExtension(target_extension_);
    }

    void setUnitsToTicks(float ticks_per_unit) override {
      if (ticks_per_unit <= 0.0f) ticks_per_unit = 1.0f;
      ticks_per_unit_ = ticks_per_unit;
      profile_dirty_ = true;
    }

    float getCurrentExtension() const override {
      if (!controller_) return 0.0f;
      float measurement = controller_->lastMeasurement();
      return (ticks_per_unit_ != 0.0f) ? (measurement / ticks_per_unit_) : measurement;
    }

    bool isAtExtension(float tolerance_units) const override {
      if (!controller_) return false;
      return fabsf(getCurrentExtension() - target_extension_) <= tolerance_units;
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
          type != probot::control::MotionProfileType::kNone) {
        type = probot::control::MotionProfileType::kNone; // Only trapezoid supported for now
      }
      profile_type_ = type;
      profile_cfg_units_ = cfg;
      profile_dirty_ = true;
    }

    void setSlewRateLimit(float units_per_second) {
      slew_rate_units_per_s_ = std::max(units_per_second, 0.0f);
      limiter_initialized_ = false;
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms;
      if (!controller_) return;
      ensureMotionProfileApplied();

      float dt_s = dt_ms > 0 ? dt_ms * 0.001f : 0.02f;
      float commanded = target_extension_;
      if (slew_rate_units_per_s_ > 0.0f){
        initializeLimiterIfNeeded();
        slew_limiter_.setMaxRate(slew_rate_units_per_s_);
        commanded = slew_limiter_.calculate(target_extension_, dt_s);
      } else if (!limiter_initialized_) {
        slew_limiter_.reset(commanded);
        limiter_initialized_ = true;
      }

      float ticks = commanded * ticks_per_unit_;
      controller_->setSetpoint(ticks, probot::control::ControlType::kPosition, position_slot_);
    }

  private:
    void ensureMotionProfileApplied(){
      if (!profile_dirty_ || !controller_) return;
      profile_dirty_ = false;
      if (profile_type_ == probot::control::MotionProfileType::kNone ||
          profile_cfg_units_.maxVelocity <= 0.0f ||
          profile_cfg_units_.maxAcceleration <= 0.0f){
        controller_->setMotionProfile(probot::control::MotionProfileType::kNone);
        controller_->setMotionProfileConfig({0.0f, 0.0f, 0.0f});
        return;
      }
      probot::control::MotionProfileConfig cfgTicks;
      cfgTicks.maxVelocity = profile_cfg_units_.maxVelocity * ticks_per_unit_;
      cfgTicks.maxAcceleration = profile_cfg_units_.maxAcceleration * ticks_per_unit_;
      cfgTicks.maxJerk = 0.0f;
      controller_->setMotionProfile(profile_type_);
      controller_->setMotionProfileConfig(cfgTicks);
    }

    void initializeLimiterIfNeeded(){
      if (!limiter_initialized_){
        float current = getCurrentExtension();
        float initial = std::isfinite(current) ? current : target_extension_;
        slew_limiter_.reset(initial);
        limiter_initialized_ = true;
      }
    }

    probot::control::IMotorController* controller_;
    float ticks_per_unit_;
    float target_extension_;
    float stage_length_;
    int   stage_count_;
    bool  has_stage_limits_;

    int position_slot_;
    probot::control::PidConfig pid_config_;

    probot::control::MotionProfileType profile_type_;
    probot::control::MotionProfileConfig profile_cfg_units_;
    bool profile_dirty_;

    float slew_rate_units_per_s_;
    probot::control::limiters::SlewRateLimiter slew_limiter_;
    bool limiter_initialized_;
  };

} // namespace probot::mechanism
