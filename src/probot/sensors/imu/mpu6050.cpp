#include <Arduino.h>
#include <probot/sensors/imu/mpu6050.hpp>

namespace {
  constexpr uint8_t REG_PWR_MGMT_1 = 0x6B;
  constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;
  constexpr uint8_t REG_GYRO_CONFIG  = 0x1B;
  constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
  constexpr float ACCEL_SCALE = 16384.0f; // +/-2g
  constexpr float GYRO_SCALE  = 131.0f;   // +/-250 dps
  constexpr float DEG2RAD     = 3.1415926535f / 180.0f;
}

namespace probot::sensors::imu {
  Mpu6050::Mpu6050(TwoWire& wire, uint8_t address)
  : wire_(&wire), address_(address) {}

  bool Mpu6050::begin(){
    wire_->begin();
    wire_->beginTransmission(address_);
    wire_->write(REG_PWR_MGMT_1);
    wire_->write(0x00);
    if (wire_->endTransmission() != 0) return false;

    wire_->beginTransmission(address_);
    wire_->write(REG_ACCEL_CONFIG);
    wire_->write(0x00); // +/-2g
    if (wire_->endTransmission() != 0) return false;

    wire_->beginTransmission(address_);
    wire_->write(REG_GYRO_CONFIG);
    wire_->write(0x00); // +/-250dps
    if (wire_->endTransmission() != 0) return false;

    delay(100);
    calibrate();
    initialized_ = true;
    lastMicros_ = micros();
    return true;
  }

  void Mpu6050::calibrate(std::size_t sampleCount){
    float gxSum=0, gySum=0, gzSum=0;
    float axSum=0, aySum=0, azSum=0;
    for (std::size_t i=0;i<sampleCount;i++){
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
    accelOffsetZ_ = (azSum / sampleCount) - ACCEL_SCALE; // assume gravity on Z
  }

  bool Mpu6050::read(ImuSample& sample){
    if (!initialized_) return false;
    int16_t axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw;
    if (!readRaw(axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw)) return false;

    uint32_t now = micros();
    float dt = (now - lastMicros_) * 1e-6f;
    if (dt <= 0.0f) dt = 1e-3f;
    lastMicros_ = now;

    float ax = (axRaw - accelOffsetX_) / ACCEL_SCALE;
    float ay = (ayRaw - accelOffsetY_) / ACCEL_SCALE;
    float az = (azRaw - accelOffsetZ_) / ACCEL_SCALE;

    float gx = ((gxRaw - gyroOffsetX_) / GYRO_SCALE) * DEG2RAD;
    float gy = ((gyRaw - gyroOffsetY_) / GYRO_SCALE) * DEG2RAD;
    float gz = ((gzRaw - gyroOffsetZ_) / GYRO_SCALE) * DEG2RAD;

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

  bool Mpu6050::readRaw(int16_t& ax, int16_t& ay, int16_t& az,
                        int16_t& gx, int16_t& gy, int16_t& gz){
    wire_->beginTransmission(address_);
    wire_->write(REG_ACCEL_XOUT_H);
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
}
