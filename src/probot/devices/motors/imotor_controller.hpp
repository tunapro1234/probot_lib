#pragma once
#include <limits>
#include <stdint.h>
namespace probot::motor {
  struct IMotorController {
    virtual bool setPower(float power) = 0; // -1.0..1.0 normalized output

    // Direction inversion: if inverted, controllers should negate applied power internally
    virtual void setInverted(bool inverted) = 0;
    virtual bool getInverted() const = 0;

    static constexpr float kUnsupported = std::numeric_limits<float>::max();
    virtual float getPower() const { return kUnsupported; }

    // Optional closed-loop hooks (default: unsupported).
    virtual bool supportsVelocity() const { return false; }
    virtual bool supportsPosition() const { return false; }
    virtual bool setVelocity(float units_per_s) { (void)units_per_s; return false; }
    virtual bool setPosition(float units) { (void)units; return false; }
    virtual float getVelocity() const { return kUnsupported; }
    virtual float getPosition() const { return kUnsupported; }

    virtual void update(uint32_t now_ms, uint32_t dt_ms) {
      (void)now_ms;
      (void)dt_ms;
    }

    virtual ~IMotorController() {}
  };
} // namespace probot::motor
