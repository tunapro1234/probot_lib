#pragma once
#include <Arduino.h>
#include <probot/control/imotor_controller.hpp>
#include <probot/control/pid.hpp>
#include <math.h>

namespace probot::drive {
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
    BasicTankDrive(probot::control::IMotorController* left,
                   probot::control::IMotorController* right)
    : left_(left), right_(right), wheel_circumference_(1.0f), track_width_(1.0f),
      vel_mode_(true), target_left_pos_(0.0f), target_right_pos_(0.0f),
      velocity_slot_(0), position_slot_(1) {
      if (left_) {
        left_->selectDefaultSlot(probot::control::ControlType::kVelocity, velocity_slot_);
        left_->selectDefaultSlot(probot::control::ControlType::kPosition, position_slot_);
      }
      if (right_) {
        right_->selectDefaultSlot(probot::control::ControlType::kVelocity, velocity_slot_);
        right_->selectDefaultSlot(probot::control::ControlType::kPosition, position_slot_);
      }
    }

    void configurePid(const probot::control::PidConfig& velocityCfg,
                      const probot::control::PidConfig& positionCfg,
                      int velocitySlot = 0,
                      int positionSlot = 1) {
      setVelocitySlot(velocitySlot);
      setPositionSlot(positionSlot);
      if (left_) left_->configurePidSlots(velocity_slot_, velocityCfg, position_slot_, positionCfg);
      if (right_) right_->configurePidSlots(velocity_slot_, velocityCfg, position_slot_, positionCfg);
    }

    void setVelocitySlot(int slot){
      slot = clampSlot(slot);
      velocity_slot_ = slot;
      if (left_) left_->selectDefaultSlot(probot::control::ControlType::kVelocity, slot);
      if (right_) right_->selectDefaultSlot(probot::control::ControlType::kVelocity, slot);
    }

    void setPositionSlot(int slot){
      slot = clampSlot(slot);
      position_slot_ = slot;
      if (left_) left_->selectDefaultSlot(probot::control::ControlType::kPosition, slot);
      if (right_) right_->selectDefaultSlot(probot::control::ControlType::kPosition, slot);
    }

    void setWheelCircumference(float units) override { wheel_circumference_ = units; }
    void setTrackWidth(float units) override { track_width_ = units; }

    void setVelocity(float left_units_per_s, float right_units_per_s) override {
      vel_mode_ = true;
      if (left_)  left_->setSetpoint(left_units_per_s, probot::control::ControlType::kVelocity, velocity_slot_);
      if (right_) right_->setSetpoint(right_units_per_s, probot::control::ControlType::kVelocity, velocity_slot_);
    }

    void setPositionTargets(float left_units, float right_units){
      vel_mode_ = false;
      target_left_pos_  = left_units;
      target_right_pos_ = right_units;
      if (left_)  left_->setSetpoint(target_left_pos_, probot::control::ControlType::kPosition, position_slot_);
      if (right_) right_->setSetpoint(target_right_pos_, probot::control::ControlType::kPosition, position_slot_);
    }

    void setPositionTargetsRelative(float left_delta_units, float right_delta_units){
      setPositionTargets(target_left_pos_ + left_delta_units,
                        target_right_pos_ + right_delta_units);
    }

    void driveDistance(float distance_units) override {
      float wheel_rotations = distance_units / wheel_circumference_;
      setPositionTargetsRelative(wheel_rotations, wheel_rotations);
    }

    void turnDegrees(float degrees) override {
      float radians = degrees * (3.1415926535f / 180.0f);
      float arc_length = radians * (track_width_ * 0.5f);
      float wheel_rotations = arc_length / wheel_circumference_;
      setPositionTargetsRelative(wheel_rotations, -wheel_rotations);
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (!vel_mode_) return; // position updates were already sent; hold until next command
      if (left_)  left_->update(now_ms, dt_ms);
      if (right_) right_->update(now_ms, dt_ms);
    }

    bool positionGoalReached(float tolerance_units) const {
      if (!left_ && !right_) return true;
      auto checkWheel = [&](probot::control::IMotorController* ctrl, float target_pos) {
        if (!ctrl) return true;
        float err_rot = target_pos - ctrl->lastMeasurement();
        float err_dist = fabsf(err_rot * wheel_circumference_);
        return err_dist <= tolerance_units;
      };
      return checkWheel(left_, target_left_pos_) && checkWheel(right_, target_right_pos_);
    }

    bool waitUntilPositionReached(float tolerance_units,
                                  uint32_t timeout_ms,
                                  uint32_t poll_ms = 20u) {
      uint32_t start = millis();
      while (!positionGoalReached(tolerance_units)){
        if ((millis() - start) >= timeout_ms) return false;
        delay(poll_ms);
      }
      return true;
    }

    bool positionActive() const { return !vel_mode_; }

  private:
    static int clampSlot(int slot){ return slot < 0 ? 0 : (slot > 3 ? 3 : slot); }

    probot::control::IMotorController* left_;
    probot::control::IMotorController* right_;
    float wheel_circumference_;
    float track_width_;
    bool  vel_mode_;
    float target_left_pos_;
    float target_right_pos_;
    int   velocity_slot_;
    int   position_slot_;
  };
} // namespace probot::drive 
