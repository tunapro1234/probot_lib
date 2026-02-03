#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <probot/devices/motors/imotor_controller.hpp>
#include <probot/prohub/prohub_board.hpp>

namespace probot::prohub {

// PCA9685 registers
constexpr uint8_t PCA_MODE1 = 0x00;
constexpr uint8_t PCA_PRESCALE = 0xFE;
constexpr uint8_t PCA_LED0_ON_L = 0x06;

/**
 * PCA9685 PWM Driver - Shared instance for I2C access.
 * Thread-safe via critical section (since I2C is fast).
 */
class PCA9685 {
public:
  static PCA9685& instance() {
    static PCA9685 inst;
    return inst;
  }

  void begin() {
    if (initialized_) return;

    Wire.begin(board::I2C_SDA, board::I2C_SCL, board::I2C_FREQ);

    // Initialize PCA at 0x40
    initPCA(board::PCA_ADDR_0);

    initialized_ = true;
  }

  // Set PWM duty cycle (0-4095) for a channel
  void setPWM(uint8_t addr, uint8_t channel, uint16_t value) {
    if (!initialized_) begin();

    // Critical section for thread safety
    portENTER_CRITICAL(&mux_);

    uint8_t reg = PCA_LED0_ON_L + 4 * channel;

    Wire.beginTransmission(addr);
    Wire.write(reg);

    if (value == 0) {
      // Full OFF
      Wire.write(0);
      Wire.write(0);
      Wire.write(0);
      Wire.write(0x10);  // LED_OFF bit
    } else if (value >= 4095) {
      // Full ON
      Wire.write(0);
      Wire.write(0x10);  // LED_ON bit
      Wire.write(0);
      Wire.write(0);
    } else {
      // Normal PWM
      Wire.write(0);           // ON_L
      Wire.write(0);           // ON_H
      Wire.write(value & 0xFF);      // OFF_L
      Wire.write((value >> 8) & 0x0F); // OFF_H
    }

    Wire.endTransmission();

    portEXIT_CRITICAL(&mux_);
  }

private:
  PCA9685() = default;

  void initPCA(uint8_t addr) {
    // Reset
    writeReg(addr, PCA_MODE1, 0x80);
    delay(10);

    // Set PWM frequency to ~1kHz (prescale = 6 for 25MHz osc)
    // freq = 25MHz / (4096 * (prescale + 1))
    // prescale = 25MHz / (4096 * freq) - 1
    // For 1kHz: prescale = 25000000 / (4096 * 1000) - 1 = 5.1 ≈ 6
    writeReg(addr, PCA_MODE1, 0x10);  // Sleep
    writeReg(addr, PCA_PRESCALE, 6);   // ~1kHz PWM
    writeReg(addr, PCA_MODE1, 0x00);  // Wake
    delay(5);
    writeReg(addr, PCA_MODE1, 0xA0);  // Auto-increment, restart
  }

  void writeReg(uint8_t addr, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
  }

  bool initialized_ = false;
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
};

/**
 * Motor controller using PCA9685 PWM channels.
 * Implements IMotorController interface for use with InnerLoop.
 */
class PCAMotor : public motor::IMotorController {
public:
  PCAMotor(uint8_t port) : port_(port) {
    if (port_ >= 1 && port_ <= 8) {
      const auto& cfg = board::MOTOR_PORTS[port_ - 1];
      pca_addr_ = cfg.pca_addr;
      ch_lpwm_ = cfg.ch_lpwm;
      ch_rpwm_ = cfg.ch_rpwm;
    }
  }

  bool setPower(float power) override {
    if (port_ < 1 || port_ > 8) return false;

    power_ = constrain(power, -1.0f, 1.0f);

    float effective = inverted_ ? -power_ : power_;

    auto& pca = PCA9685::instance();

    if (effective > 0.001f) {
      // Forward
      uint16_t pwm = (uint16_t)(effective * 4095.0f);
      pca.setPWM(pca_addr_, ch_lpwm_, pwm);
      pca.setPWM(pca_addr_, ch_rpwm_, 0);
    } else if (effective < -0.001f) {
      // Reverse
      uint16_t pwm = (uint16_t)(-effective * 4095.0f);
      pca.setPWM(pca_addr_, ch_lpwm_, 0);
      pca.setPWM(pca_addr_, ch_rpwm_, pwm);
    } else {
      // Stop
      pca.setPWM(pca_addr_, ch_lpwm_, 0);
      pca.setPWM(pca_addr_, ch_rpwm_, 0);
    }

    return true;
  }

  void setInverted(bool inverted) override { inverted_ = inverted; }
  bool getInverted() const override { return inverted_; }
  float getPower() const override { return power_; }

private:
  uint8_t port_ = 0;
  uint8_t pca_addr_ = 0;
  uint8_t ch_lpwm_ = 0;
  uint8_t ch_rpwm_ = 0;
  float power_ = 0.0f;
  bool inverted_ = false;
};

} // namespace probot::prohub
