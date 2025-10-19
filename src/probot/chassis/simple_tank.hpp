#pragma once
#include <algorithm>
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::chassis {
  /**
   * @brief Simple tank drive with direct motor power control
   * Uses IMotorDriver for open-loop control (no PID)
   */
  class SimpleTankDrive {
  public:
    SimpleTankDrive(probot::motor::IMotorDriver* left, probot::motor::IMotorDriver* right)
    : left_(left), right_(right) {}

    void setInverted(bool left, bool right){
      if (left_)  left_->setInverted(left);
      if (right_) right_->setInverted(right);
    }

    void drive(float leftPower, float rightPower){
      leftPower = std::clamp(leftPower, -1.0f, 1.0f);
      rightPower = std::clamp(rightPower, -1.0f, 1.0f);
      if (left_)  left_->setPower(leftPower);
      if (right_) right_->setPower(rightPower);
    }

    void stop(){
      if (left_)  left_->setPower(0.0f);
      if (right_) right_->setPower(0.0f);
    }

  private:
    probot::motor::IMotorDriver* left_;
    probot::motor::IMotorDriver* right_;
  };
}
