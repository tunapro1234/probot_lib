#pragma once
#include <stdint.h>

namespace probot::prohub::board {

// ========== I2C Configuration ==========
constexpr int I2C_SDA = 1;
constexpr int I2C_SCL = 2;
constexpr uint32_t I2C_FREQ = 400000;  // 400kHz

// ========== PCA9685 Addresses ==========
constexpr uint8_t PCA_ADDR_0 = 0x40;  // First PCA (motors 1-4, servos)
constexpr uint8_t PCA_ADDR_1 = 0x41;  // Second PCA (motors 5-8, servos) - future

// ========== UART Configuration ==========
constexpr int ENCODER_UART_RX = 10;
constexpr int ENCODER_UART_TX = 11;

// ========== Motor Port Mapping ==========
// Each motor uses 3 PCA channels: LPWM, RPWM, reserved
struct MotorPort {
  uint8_t pca_addr;
  uint8_t ch_lpwm;    // Left/Forward PWM
  uint8_t ch_rpwm;    // Right/Reverse PWM
  uint8_t ch_extra;   // Reserved for future (brake, etc.)
};

// Motor ports 1-8 (0-indexed internally)
constexpr MotorPort MOTOR_PORTS[8] = {
  // PCA 0x40: Motors 1-4
  {PCA_ADDR_0, 0, 1, 2},    // Port 1
  {PCA_ADDR_0, 3, 4, 5},    // Port 2
  {PCA_ADDR_0, 6, 7, 8},    // Port 3
  {PCA_ADDR_0, 9, 10, 11},  // Port 4
  // PCA 0x41: Motors 5-8 (future)
  {PCA_ADDR_1, 0, 1, 2},    // Port 5
  {PCA_ADDR_1, 3, 4, 5},    // Port 6
  {PCA_ADDR_1, 6, 7, 8},    // Port 7
  {PCA_ADDR_1, 9, 10, 11},  // Port 8
};

// ========== Servo Mapping ==========
// Servos use remaining PCA channels (12-15)
constexpr uint8_t SERVO_BASE_CHANNEL = 12;
constexpr uint8_t MAX_SERVOS_PER_PCA = 4;

// ========== Encoder Mapping ==========
// Encoder index matches motor port (port 1 = encoder 0, etc.)
// Can be overridden in registerMotor()

} // namespace probot::prohub::board
