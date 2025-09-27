#pragma once
#include <math.h>
#include <probot/controllers/imotor_controller.hpp>

namespace probot::controllers {
  struct ISlider {
    virtual void setTargetLength(float length_units) = 0; // user units (e.g., cm)
    virtual float getTargetLength() const = 0;
    virtual void setLengthToTicks(float ticks_per_unit) = 0; // ticks = length * ticks_per_unit
    virtual void setLengthLimits(float min_units, float max_units) = 0;
    virtual float getCurrentLength() const = 0;
    virtual bool isAtTarget(float tolerance_units) const = 0;
    virtual void update(uint32_t now_ms, uint32_t dt_ms) = 0;
    virtual ~ISlider() {}
  };

  class Slider : public ISlider, public ::control::IUpdatable {
  public:
    explicit Slider(IMotorController* controller)
    : controller_(controller), ticks_per_unit_(1.0f), target_len_(0.0f),
      min_len_(0.0f), max_len_(0.0f), has_limits_(false) {}

    void setTargetLength(float length_units) override {
      if (has_limits_){
        if (length_units < min_len_) length_units = min_len_;
        if (length_units > max_len_) length_units = max_len_;
      }
      target_len_ = length_units;
    }
    float getTargetLength() const override { return target_len_; }

    void setLengthToTicks(float ticks_per_unit) override { ticks_per_unit_ = ticks_per_unit; }

    void setLengthLimits(float min_units, float max_units) override {
      if (max_units < min_units) {
        float tmp = min_units;
        min_units = max_units;
        max_units = tmp;
      }
      min_len_ = min_units;
      max_len_ = max_units;
      has_limits_ = true;
      setTargetLength(target_len_); // re-clamp existing target
    }

    float getCurrentLength() const override {
      if (!controller_) return 0.0f;
      float measurement = controller_->lastMeasurement();
      return (ticks_per_unit_ != 0.0f) ? (measurement / ticks_per_unit_) : measurement;
    }

    bool isAtTarget(float tolerance_units) const override {
      if (!controller_) return false;
      float current = getCurrentLength();
      return fabsf(current - target_len_) <= tolerance_units;
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms; (void)dt_ms;
      float ticks_setpoint = target_len_ * ticks_per_unit_;
      if (controller_) controller_->setSetpoint(ticks_setpoint, ControlType::kPosition, -1);
    }

  private:
    IMotorController*     controller_;
    float                 ticks_per_unit_;
    float                 target_len_;
    float                 min_len_;
    float                 max_len_;
    bool                  has_limits_;
  };
} // namespace probot::controllers 
