#pragma once
#include <math.h>
#include <probot/control/imotor_controller.hpp>

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
    : controller_(controller), ticks_per_unit_(1.0f), target_extension_(0.0f),
      stage_length_(0.0f), stage_count_(0), has_stage_limits_(false) {}

    void setTargetExtension(float units) override {
      if (has_stage_limits_){
        float max_ext = stage_length_ * static_cast<float>(stage_count_);
        if (units < 0.0f) units = 0.0f;
        if (units > max_ext) units = max_ext;
      }
      target_extension_ = units;
    }

    float getTargetExtension() const override { return target_extension_; }

    void setStageConfiguration(int stage_count, float stage_length_units) override {
      if (stage_count < 0) stage_count = 0;
      stage_count_ = stage_count;
      stage_length_ = stage_length_units;
      has_stage_limits_ = stage_count_ > 0 && stage_length_ > 0.0f;
      setTargetExtension(target_extension_);
    }

    void setUnitsToTicks(float ticks_per_unit) override { ticks_per_unit_ = ticks_per_unit; }

    float getCurrentExtension() const override {
      if (!controller_) return 0.0f;
      float measurement = controller_->lastMeasurement();
      return (ticks_per_unit_ != 0.0f) ? (measurement / ticks_per_unit_) : measurement;
    }

    bool isAtExtension(float tolerance_units) const override {
      if (!controller_) return false;
      return fabsf(getCurrentExtension() - target_extension_) <= tolerance_units;
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms; (void)dt_ms;
      if (!controller_) return;
      float target_ticks = target_extension_ * ticks_per_unit_;
      controller_->setSetpoint(target_ticks, probot::control::ControlType::kPosition, -1);
    }

  private:
    probot::control::IMotorController* controller_;
    float             ticks_per_unit_;
    float             target_extension_;
    float             stage_length_;
    int               stage_count_;
    bool              has_stage_limits_;
  };

} // namespace probot::mechanism
