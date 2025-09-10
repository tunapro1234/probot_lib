#pragma once
#include <probot/controllers/closed_loop_motor.hpp>
#include <math.h>

namespace probot::controllers {
  struct IChassis {
    virtual void setVelocity(float left_units_per_s, float right_units_per_s) = 0;
    virtual void driveDistance(float distance_units) = 0;   // forward (+) or backward (-)
    virtual void turnDegrees(float degrees) = 0;            // +CCW, -CW
    virtual void setWheelCircumference(float units) = 0;    // e.g., cm
    virtual void setTrackWidth(float units) = 0;            // distance between wheel centers
    virtual void update(uint32_t now_ms, uint32_t dt_ms) = 0;
    virtual ~IChassis() {}
  };

  class BasicTankDrive : public IChassis, public ::control::IUpdatable {
  public:
    BasicTankDrive(ClosedLoopMotor* left, ClosedLoopMotor* right)
    : left_(left), right_(right), wheel_circumference_(1.0f), track_width_(1.0f),
      vel_mode_(true), target_left_pos_(0.0f), target_right_pos_(0.0f) {}

    void setWheelCircumference(float units) override { wheel_circumference_ = units; }
    void setTrackWidth(float units) override { track_width_ = units; }

    void setVelocity(float left_units_per_s, float right_units_per_s) override {
      vel_mode_ = true;
      if (left_)  left_->setSetpoint(left_units_per_s, ControlType::kVelocity);
      if (right_) right_->setSetpoint(right_units_per_s, ControlType::kVelocity);
    }

    void driveDistance(float distance_units) override {
      vel_mode_ = false;
      float wheel_rotations = distance_units / wheel_circumference_;
      target_left_pos_  += wheel_rotations;
      target_right_pos_ += wheel_rotations;
      if (left_)  left_->setSetpoint(target_left_pos_, ControlType::kPosition);
      if (right_) right_->setSetpoint(target_right_pos_, ControlType::kPosition);
    }

    void turnDegrees(float degrees) override {
      vel_mode_ = false;
      float radians = degrees * (3.1415926535f / 180.0f);
      float arc_length = radians * (track_width_ * 0.5f);
      float wheel_rotations = arc_length / wheel_circumference_;
      target_left_pos_  += wheel_rotations;
      target_right_pos_ -= wheel_rotations;
      if (left_)  left_->setSetpoint(target_left_pos_, ControlType::kPosition);
      if (right_) right_->setSetpoint(target_right_pos_, ControlType::kPosition);
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (!vel_mode_) return; // position updates were already sent; hold until next command
      if (left_)  left_->update(now_ms, dt_ms);
      if (right_) right_->update(now_ms, dt_ms);
    }

  private:
    ClosedLoopMotor* left_;
    ClosedLoopMotor* right_;
    float wheel_circumference_;
    float track_width_;
    bool  vel_mode_;
    float target_left_pos_;
    float target_right_pos_;
  };
} // namespace probot::controllers 