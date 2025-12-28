#pragma once
#include <stddef.h>
#include <stdint.h>

namespace probot::sensors::imu {
  struct ImuSample {
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float yaw;
    float pitch;
    float roll;
    uint32_t timestampMs;
  };

  class IImu {
  public:
    virtual bool begin() = 0;
    virtual void calibrate(std::size_t sampleCount = 500) = 0;
    virtual bool read(ImuSample& sample) = 0;
    virtual float yaw() const = 0;
    virtual float pitch() const = 0;
    virtual float roll() const = 0;
    virtual ~IImu() {}
  };
}
