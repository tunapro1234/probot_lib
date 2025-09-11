#ifndef PROBOT_SIM_NULL_MOTOR_HPP
#define PROBOT_SIM_NULL_MOTOR_HPP
#pragma once
#include <probot/devices/motors/motor.hpp>

namespace probot::motor {

class NullMotor : public IMotor {
public:
  NullMotor() : _owner(nullptr), _power(0), _inverted(false) {}

  bool claim(void* owner) override {
    if (_owner && _owner != owner) return false; _owner = owner; return true;
  }

  void release(void* owner) override {
    if (_owner == owner) { _owner = nullptr; _power = 0; }
  }

  bool setPower(int16_t milli, void* owner) override {
    if (_owner && _owner != owner) return false; _power = _inverted ? (int16_t)-milli : milli; return true;
  }

  bool isClaimed() const override { return _owner != nullptr; }
  void* currentOwner() const override { return _owner; }

  void setInverted(bool inv) override { _inverted = inv; }
  bool getInverted() const override { return _inverted; }

private:
  void*   _owner;
  int16_t _power;
  bool    _inverted;
};

} // namespace probot::motor

#endif // PROBOT_SIM_NULL_MOTOR_HPP 