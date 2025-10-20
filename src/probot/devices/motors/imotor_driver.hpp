#pragma once

namespace probot::motor {
  struct IMotorDriver {
    virtual bool setPower(float power) = 0; // -1.0..1.0 normalized output

    // Direction inversion: if inverted, drivers should negate applied power internally
    virtual void setInverted(bool inverted) = 0;
    virtual bool getInverted() const = 0;

    virtual ~IMotorDriver() {}
  };
} // namespace probot::motor
