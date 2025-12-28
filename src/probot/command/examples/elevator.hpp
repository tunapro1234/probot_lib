#pragma once
#include <math.h>
#include <probot/command/subsystem.hpp>
#include <probot/control/pid_motor_controller.hpp>

namespace probot::command::examples {

  struct IElevator {
    virtual void setTargetHeight(float units) = 0;
    virtual float getTargetHeight() const = 0;
    virtual void setHeightLimits(float min_units, float max_units) = 0;
    virtual void setUnitsToTicks(float ticks_per_unit) = 0;
    virtual float getCurrentHeight() const = 0;
    virtual bool isAtHeight(float tolerance_units) const = 0;
    virtual ~IElevator() {}
  };

  class Elevator : public probot::command::SubsystemBase, public IElevator {
  public:
    explicit Elevator(probot::control::PidMotorController* controller)
    : probot::command::SubsystemBase("Elevator"),
      controller_(controller), ticks_per_unit_(1.0f), target_height_(0.0f),
      min_height_(0.0f), max_height_(0.0f), has_limits_(false) {
    }

    ~Elevator() override = default;

    void setTargetHeight(float units) override {
      if (has_limits_){
        if (units < min_height_) units = min_height_;
        if (units > max_height_) units = max_height_;
      }
      target_height_ = units;
    }

    float getTargetHeight() const override { return target_height_; }

    void setHeightLimits(float min_units, float max_units) override {
      if (max_units < min_units){
        float tmp = min_units; min_units = max_units; max_units = tmp;
      }
      min_height_ = min_units;
      max_height_ = max_units;
      has_limits_ = true;
      setTargetHeight(target_height_);
    }

    void setUnitsToTicks(float ticks_per_unit) override { ticks_per_unit_ = ticks_per_unit; }

    float getCurrentHeight() const override {
      if (!controller_) return 0.0f;
      float measurement = controller_->lastMeasurement();
      return (ticks_per_unit_ != 0.0f) ? (measurement / ticks_per_unit_) : measurement;
    }

    bool isAtHeight(float tolerance_units) const override {
      if (!controller_) return false;
      return fabsf(getCurrentHeight() - target_height_) <= tolerance_units;
    }

    void periodic(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms; (void)dt_ms;
      if (!controller_) return;
      float target_ticks = target_height_ * ticks_per_unit_;
      controller_->setPosition(target_ticks);
    }

  private:
    probot::control::PidMotorController* controller_;
    float             ticks_per_unit_;
    float             target_height_;
    float             min_height_;
    float             max_height_;
    bool              has_limits_;
  };

} // namespace probot::command::examples
