#pragma once
#include <math.h>
#include <probot/control/pid_motor_controller.hpp>

namespace probot::mechanism {

  struct IArm : public ::control::IUpdatable {
    virtual void setTargetAngleDeg(float degrees) = 0;
    virtual float getTargetAngleDeg() const = 0;
    virtual void setAngleLimits(float min_deg, float max_deg) = 0;
    virtual void setDegreesToTicks(float ticks_per_degree) = 0;
    virtual float getCurrentAngleDeg() const = 0;
    virtual bool isAtAngle(float tolerance_deg) const = 0;
    virtual ~IArm() {}
  };

  class Arm : public IArm {
  public:
    explicit Arm(probot::control::PidMotorController* controller)
    : controller_(controller), ticks_per_degree_(1.0f), target_angle_(0.0f),
      min_angle_(-90.0f), max_angle_(90.0f), has_limits_(false) {
    }

    ~Arm() override {
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
      if (max_deg < min_deg){ float tmp=min_deg; min_deg=max_deg; max_deg=tmp; }
      min_angle_ = min_deg;
      max_angle_ = max_deg;
      has_limits_ = true;
      setTargetAngleDeg(target_angle_);
    }

    void setDegreesToTicks(float ticks_per_degree) override { ticks_per_degree_ = ticks_per_degree; }

    float getCurrentAngleDeg() const override {
      if (!controller_) return 0.0f;
      float measurement = controller_->lastMeasurement();
      return (ticks_per_degree_ != 0.0f) ? (measurement / ticks_per_degree_) : measurement;
    }

    bool isAtAngle(float tolerance_deg) const override {
      if (!controller_) return false;
      return fabsf(getCurrentAngleDeg() - target_angle_) <= tolerance_deg;
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms; (void)dt_ms;
      if (!controller_) return;
      float target_ticks = target_angle_ * ticks_per_degree_;
      controller_->setPosition(target_ticks);
    }

  private:
    probot::control::PidMotorController* controller_;
    float             ticks_per_degree_;
    float             target_angle_;
    float             min_angle_;
    float             max_angle_;
    bool              has_limits_;
  };

} // namespace probot::mechanism
