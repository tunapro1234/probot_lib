#ifndef PROBOT_DEVICES_MOTORS_MOTOR_HANDLE_HPP
#define PROBOT_DEVICES_MOTORS_MOTOR_HANDLE_HPP
#pragma once
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::motor {

class MotorHandle {
public:
  explicit MotorHandle(IMotorDriver& motor)
  : _motor(&motor) {}

  void setPower(float value){ _motor->setPower(value); }
  void setInverted(bool inv){ _motor->setInverted(inv); }
  bool getInverted() const { return _motor->getInverted(); }

  IMotorDriver& underlying() { return *_motor; }
  const IMotorDriver& underlying() const { return *_motor; }

private:
  IMotorDriver* _motor;
};

} // namespace probot::motor

#endif // PROBOT_DEVICES_MOTORS_MOTOR_HANDLE_HPP 
