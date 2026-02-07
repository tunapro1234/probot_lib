#pragma once

/**
 * ProHub - Dual-loop control system for Probot with Pico co-processor.
 *
 * FTC-Style Usage:
 *   #include <probot.h>
 *   #include <probot/prohub/prohub.hpp>
 *
 *   void robotInit() {
 *     // Direct motor control (port 1-8)
 *     ProHub.motor(1).setPower(0.5);
 *
 *     // PID control (port 1 with encoder 0)
 *     ProHub.initMotorPID(1, pidCfg);
 *   }
 *
 *   void teleopLoop() {
 *     int64_t pos = ProHub.getTicks(0);
 *     ProHub.setTarget(1, pos + 1000);  // Target for port 1
 *   }
 */

#include <probot/prohub/prohub_config.hpp>
#include <probot/prohub/prohub_board.hpp>
#include <probot/prohub/double_buffer.hpp>
#include <probot/prohub/encoder_frame.hpp>
#include <probot/prohub/uart_encoder.hpp>
#include <probot/prohub/inner_loop.hpp>
#include <probot/prohub/pca_motor.hpp>

namespace probot::prohub {

/**
 * ProHub - Main interface class.
 *
 * Auto-initializes on first use:
 *   - UART encoder task (fixed pins from prohub_board.hpp)
 *   - PCA9685 PWM driver (I2C)
 *   - Inner loop task (on first PID registration)
 */
class ProHubClass {
public:
  // ========== Motor Access (FTC-style) ==========

  /**
   * Get motor controller for a port (1-8).
   * Auto-initializes PCA9685 on first call.
   *
   * Example: ProHub.motor(1).setPower(0.5);
   */
  PCAMotor& motor(uint8_t port) {
    ensureInitialized();
    if (port >= 1 && port <= 8) {
      return motors_[port - 1];
    }
    return motors_[0];  // Fallback to port 1
  }

  /**
   * Initialize PID control for a motor port.
   * Uses encoder index = port - 1 by default.
   *
   * @param port Motor port (1-8)
   * @param cfg PID configuration
   * @param encoder_idx Encoder index (default: port-1)
   * @param is_velocity Velocity PID mode (default: position)
   */
  void initMotorPID(uint8_t port,
                    const control::PidConfig& cfg,
                    int encoder_idx = -1,
                    bool is_velocity = false) {
    if (port < 1 || port > 8) return;
    ensureInitialized();

    uint8_t enc = (encoder_idx < 0) ? (port - 1) : encoder_idx;

    int slot = InnerLoop::instance().registerController(
      &motors_[port - 1], cfg, enc, is_velocity
    );

    pid_slots_[port - 1] = slot;
  }

  /**
   * Set target for a motor port (thread-safe).
   * Motor must have PID initialized via initMotorPID().
   */
  void setTarget(uint8_t port, float target) {
    if (port < 1 || port > 8) return;
    int slot = pid_slots_[port - 1];
    if (slot >= 0) {
      InnerLoop::instance().setTarget(slot, target);
    }
  }

  /**
   * Enable/disable PID for a motor port.
   */
  void setEnabled(uint8_t port, bool enabled) {
    if (port < 1 || port > 8) return;
    int slot = pid_slots_[port - 1];
    if (slot >= 0) {
      InnerLoop::instance().setEnabled(slot, enabled);
    }
  }

  // ========== Encoder Reading ==========

  /**
   * Get encoder position in ticks.
   * @param encoder_idx Encoder index (0-7)
   */
  int64_t getTicks(uint8_t encoder_idx) {
    ensureInitialized();
    const auto& frame = UartEncoder::instance().read();
    if (!frame.valid || encoder_idx >= MAX_ENCODERS) return 0;
    return frame.totalTicks[encoder_idx];
  }

  /**
   * Get encoder speed in ticks per second.
   */
  float getSpeed(uint8_t encoder_idx) {
    ensureInitialized();
    const auto& frame = UartEncoder::instance().read();
    if (!frame.valid || encoder_idx >= MAX_ENCODERS || frame.dt_us == 0) return 0.0f;
    return (float)frame.deltaTicks[encoder_idx] * 1000000.0f / (float)frame.dt_us;
  }

  /**
   * Get raw encoder frame (copy).
   */
  EncoderFrame getFrame() {
    ensureInitialized();
    return UartEncoder::instance().read();
  }

  // ========== PID Output Reading ==========

  /**
   * Get PID output for a motor port (-1 to 1).
   */
  float getOutput(uint8_t port) {
    if (port < 1 || port > 8) return 0.0f;
    int slot = pid_slots_[port - 1];
    if (slot < 0) return 0.0f;

    const auto& out = InnerLoop::instance().readOutput();
    if (!out.valid) return 0.0f;
    return out.output[slot];
  }

  /**
   * Get PID error for a motor port.
   */
  float getError(uint8_t port) {
    if (port < 1 || port > 8) return 0.0f;
    int slot = pid_slots_[port - 1];
    if (slot < 0) return 0.0f;

    const auto& out = InnerLoop::instance().readOutput();
    if (!out.valid) return 0.0f;
    return out.error[slot];
  }

  // ========== Statistics ==========

  uint32_t frameCount() { return UartEncoder::instance().frameCount(); }
  uint32_t crcErrors() { return UartEncoder::instance().crcErrors(); }
  uint32_t loopCount() { return InnerLoop::instance().readOutput().iteration; }

  bool isEncoderRunning() { return UartEncoder::instance().isRunning(); }
  bool isLoopRunning() { return InnerLoop::instance().isRunning(); }

private:
  void ensureInitialized() {
    if (initialized_) return;
    initialized_ = true;

    // Configure UART with fixed pins
    UartEncoder::instance().configure(
      Serial1,
      board::ENCODER_UART_RX,
      board::ENCODER_UART_TX,
      DEFAULT_UART_BAUD
    );

    // PCA9685 auto-initializes on first motor access
  }

  bool initialized_ = false;

  // Motor controllers for ports 1-8
  PCAMotor motors_[8] = {
    PCAMotor(1), PCAMotor(2), PCAMotor(3), PCAMotor(4),
    PCAMotor(5), PCAMotor(6), PCAMotor(7), PCAMotor(8)
  };

  // PID slot mapping (port index -> InnerLoop slot)
  int pid_slots_[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
};

} // namespace probot::prohub

// Global instance
inline probot::prohub::ProHubClass ProHub;
