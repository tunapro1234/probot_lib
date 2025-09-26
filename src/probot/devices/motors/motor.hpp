#pragma once
#include <stdint.h>

namespace probot::motor {
  struct IMotor {
    virtual bool claim(void* owner) = 0;                    // exclusive write claim
    virtual void release(void* owner) = 0;                  // release claim if owner matches
    virtual bool setPower(float power, void* owner) = 0;     // -1.0..1.0 normalized output
    virtual bool isClaimed() const = 0;
    virtual void* currentOwner() const = 0;

    // Direction inversion: if inverted, drivers should negate applied power internally
    virtual void setInverted(bool inverted) = 0;
    virtual bool getInverted() const = 0;

    virtual ~IMotor() {}
  };
} // namespace probot::motor 
