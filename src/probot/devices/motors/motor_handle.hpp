#ifndef PROBOT_DEVICES_MOTORS_MOTOR_HANDLE_HPP
#define PROBOT_DEVICES_MOTORS_MOTOR_HANDLE_HPP
#pragma once
#include <probot/devices/motors/motor.hpp>

namespace probot::motor {

class MotorHandle {
public:
  explicit MotorHandle(IMotorDriver& motor)
  : _motor(&motor), _owner(this)
  {
    _motor->claim(_owner);
  }

  void setPower(float value){ _motor->setPower(value, _owner); }
  void setInverted(bool inv){ _motor->setInverted(inv); }
  bool getInverted() const { return _motor->getInverted(); }

  void release(){ _motor->release(_owner); }

  IMotorDriver& underlying() { return *_motor; }
  const IMotorDriver& underlying() const { return *_motor; }

private:
  IMotorDriver* _motor;
  void*   _owner; // unique owner token
};

} // namespace probot::motor

#endif // PROBOT_DEVICES_MOTORS_MOTOR_HANDLE_HPP 
