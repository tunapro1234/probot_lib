#pragma once
#include <probot/controllers/motor_controller.hpp>

namespace probot::controllers {
  struct ISlider {
    virtual void setTargetLength(float length_units) = 0; // user units (e.g., cm)
    virtual float getTargetLength() const = 0;
    virtual void setLengthToTicks(float ticks_per_unit) = 0; // ticks = length * ticks_per_unit
    virtual void update(uint32_t now_ms, uint32_t dt_ms) = 0;
    virtual ~ISlider() {}
  };

  class Slider : public ISlider, public ::control::IUpdatable {
  public:
    explicit Slider(IMotorController* controller)
    : controller_(controller), ticks_per_unit_(1.0f), target_len_(0.0f) {}

    void setTargetLength(float length_units) override {
      target_len_ = length_units;
    }
    float getTargetLength() const override { return target_len_; }

    void setLengthToTicks(float ticks_per_unit) override { ticks_per_unit_ = ticks_per_unit; }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms; (void)dt_ms;
      float ticks_setpoint = target_len_ * ticks_per_unit_;
      if (controller_) controller_->setSetpoint(ticks_setpoint, ControlType::kPosition, -1);
    }

  private:
    IMotorController*     controller_;
    float                 ticks_per_unit_;
    float                 target_len_;
  };
} // namespace probot::controllers 
