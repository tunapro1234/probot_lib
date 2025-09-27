#pragma once
#include <Wire.h>
#include <probot/sensors/imu/imu.hpp>

namespace probot::sensors::imu {
  class Mpu6050 : public IImu {
  public:
    explicit Mpu6050(TwoWire& wire = Wire, uint8_t address = 0x68);

    bool begin() override;
    void calibrate(std::size_t sampleCount = 500) override;
    bool read(ImuSample& sample) override;

    float yaw() const override { return yaw_; }
    float pitch() const override { return pitch_; }
    float roll() const override { return roll_; }

    void setComplementaryGain(float alpha) { complementaryAlpha_ = alpha; }
    float complementaryGain() const { return complementaryAlpha_; }

  private:
    bool readRaw(int16_t& ax, int16_t& ay, int16_t& az,
                 int16_t& gx, int16_t& gy, int16_t& gz);

    TwoWire* wire_;
    uint8_t address_;
    float gyroOffsetX_ = 0.0f;
    float gyroOffsetY_ = 0.0f;
    float gyroOffsetZ_ = 0.0f;
    float accelOffsetX_ = 0.0f;
    float accelOffsetY_ = 0.0f;
    float accelOffsetZ_ = 0.0f;
    float yaw_ = 0.0f;
    float pitch_ = 0.0f;
    float roll_ = 0.0f;
    float complementaryAlpha_ = 0.98f;
    uint32_t lastMicros_ = 0;
    bool initialized_ = false;
  };
}
