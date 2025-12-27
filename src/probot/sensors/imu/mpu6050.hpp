#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <cmath>
#include <probot/sensors/imu/imu.hpp>

namespace probot::sensors::imu {
  namespace detail {
    constexpr uint8_t kRegPwrMgmt1 = 0x6B;
    constexpr uint8_t kRegAccelConfig = 0x1C;
    constexpr uint8_t kRegGyroConfig  = 0x1B;
    constexpr uint8_t kRegAccelXoutH = 0x3B;
    constexpr float kAccelScale = 16384.0f; // +/-2g
    constexpr float kGyroScale  = 131.0f;   // +/-250 dps
    constexpr float kDeg2Rad    = 3.1415926535f / 180.0f;
  }

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

  inline Mpu6050::Mpu6050(TwoWire& wire, uint8_t address)
  : wire_(&wire), address_(address) {}

  inline bool Mpu6050::begin(){
    wire_->begin();
    wire_->beginTransmission(address_);
    wire_->write(detail::kRegPwrMgmt1);
    wire_->write(0x00);
    if (wire_->endTransmission() != 0) return false;

    wire_->beginTransmission(address_);
    wire_->write(detail::kRegAccelConfig);
    wire_->write(0x00); // +/-2g
    if (wire_->endTransmission() != 0) return false;

    wire_->beginTransmission(address_);
    wire_->write(detail::kRegGyroConfig);
    wire_->write(0x00); // +/-250dps
    if (wire_->endTransmission() != 0) return false;

    delay(100);
    calibrate();
    initialized_ = true;
    lastMicros_ = micros();
    return true;
  }

  inline void Mpu6050::calibrate(std::size_t sampleCount){
    float gxSum = 0.0f;
    float gySum = 0.0f;
    float gzSum = 0.0f;
    float axSum = 0.0f;
    float aySum = 0.0f;
    float azSum = 0.0f;
    for (std::size_t i = 0; i < sampleCount; i++){
      int16_t ax, ay, az, gx, gy, gz;
      if (!readRaw(ax, ay, az, gx, gy, gz)) continue;
      gxSum += gx; gySum += gy; gzSum += gz;
      axSum += ax; aySum += ay; azSum += az;
      delay(2);
    }
    if (sampleCount == 0) sampleCount = 1;
    gyroOffsetX_ = gxSum / sampleCount;
    gyroOffsetY_ = gySum / sampleCount;
    gyroOffsetZ_ = gzSum / sampleCount;
    accelOffsetX_ = axSum / sampleCount;
    accelOffsetY_ = aySum / sampleCount;
    accelOffsetZ_ = (azSum / sampleCount) - detail::kAccelScale; // assume gravity on Z
  }

  inline bool Mpu6050::read(ImuSample& sample){
    if (!initialized_) return false;
    int16_t axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw;
    if (!readRaw(axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw)) return false;

    uint32_t now = micros();
    float dt = (now - lastMicros_) * 1e-6f;
    if (dt <= 0.0f) dt = 1e-3f;
    lastMicros_ = now;

    float ax = (axRaw - accelOffsetX_) / detail::kAccelScale;
    float ay = (ayRaw - accelOffsetY_) / detail::kAccelScale;
    float az = (azRaw - accelOffsetZ_) / detail::kAccelScale;

    float gx = ((gxRaw - gyroOffsetX_) / detail::kGyroScale) * detail::kDeg2Rad;
    float gy = ((gyRaw - gyroOffsetY_) / detail::kGyroScale) * detail::kDeg2Rad;
    float gz = ((gzRaw - gyroOffsetZ_) / detail::kGyroScale) * detail::kDeg2Rad;

    float accelRoll = std::atan2(ay, az);
    float accelPitch = std::atan2(-ax, std::sqrt(ay*ay + az*az));

    if (!std::isfinite(accelRoll)) accelRoll = roll_;
    if (!std::isfinite(accelPitch)) accelPitch = pitch_;

    roll_ = complementaryAlpha_ * (roll_ + gx * dt) + (1.0f - complementaryAlpha_) * accelRoll;
    pitch_ = complementaryAlpha_ * (pitch_ + gy * dt) + (1.0f - complementaryAlpha_) * accelPitch;
    yaw_ += gz * dt;

    sample.accelX = ax;
    sample.accelY = ay;
    sample.accelZ = az;
    sample.gyroX = gx;
    sample.gyroY = gy;
    sample.gyroZ = gz;
    sample.yaw = yaw_;
    sample.pitch = pitch_;
    sample.roll = roll_;
    sample.timestampMs = now / 1000u;
    return true;
  }

  inline bool Mpu6050::readRaw(int16_t& ax, int16_t& ay, int16_t& az,
                               int16_t& gx, int16_t& gy, int16_t& gz){
    wire_->beginTransmission(address_);
    wire_->write(detail::kRegAccelXoutH);
    if (wire_->endTransmission(false) != 0) return false;
    if (wire_->requestFrom(address_, static_cast<uint8_t>(14)) != 14) return false;
    ax = (wire_->read() << 8) | wire_->read();
    ay = (wire_->read() << 8) | wire_->read();
    az = (wire_->read() << 8) | wire_->read();
    wire_->read(); wire_->read(); // temperature ignore
    gx = (wire_->read() << 8) | wire_->read();
    gy = (wire_->read() << 8) | wire_->read();
    gz = (wire_->read() << 8) | wire_->read();
    return true;
  }
} // namespace probot::sensors::imu
