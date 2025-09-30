#pragma once
#include <algorithm>
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::chassis {
  class BasicTankDrive {
  public:
    BasicTankDrive(probot::motor::IMotorDriver* left, probot::motor::IMotorDriver* right)
    : left_(left), right_(right), ownerLeft_(this), ownerRight_(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this)+1)) {
      if (left_)  left_->claim(ownerLeft_);
      if (right_) right_->claim(ownerRight_);
    }

    ~BasicTankDrive(){
      if (left_)  left_->release(ownerLeft_);
      if (right_) right_->release(ownerRight_);
    }

    void setInverted(bool left, bool right){
      if (left_)  left_->setInverted(left);
      if (right_) right_->setInverted(right);
    }

    void drive(float leftPower, float rightPower){
      leftPower = std::clamp(leftPower, -1.0f, 1.0f);
      rightPower = std::clamp(rightPower, -1.0f, 1.0f);
      if (left_)  left_->setPower(leftPower, ownerLeft_);
      if (right_) right_->setPower(rightPower, ownerRight_);
    }

    void stop(){
      if (left_)  left_->setPower(0.0f, ownerLeft_);
      if (right_) right_->setPower(0.0f, ownerRight_);
    }

  private:
    probot::motor::IMotorDriver* left_;
    probot::motor::IMotorDriver* right_;
    void* ownerLeft_;
    void* ownerRight_;
  };
}
