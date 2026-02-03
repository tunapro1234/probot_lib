#pragma once
#include <Arduino.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <probot/prohub/prohub_config.hpp>
#include <probot/prohub/double_buffer.hpp>
#include <probot/prohub/uart_encoder.hpp>
#include <probot/control/pid.hpp>
#include <probot/devices/motors/imotor_controller.hpp>

namespace probot::prohub {

/**
 * Inner loop output - readable by user code (lock-free).
 */
struct InnerLoopOutput {
  float output[MAX_CONTROLLERS] = {};       // Motor output (-1 to 1)
  float measurement[MAX_CONTROLLERS] = {};  // Current position/velocity
  float error[MAX_CONTROLLERS] = {};        // Target - measurement
  uint32_t iteration = 0;                   // Loop counter
  bool valid = false;
};

/**
 * Inner Control Loop - High-frequency PID control (default 3ms).
 *
 * Usage:
 *   int slot = InnerLoop::instance().registerController(&motor, pidConfig, 0, false);
 *   InnerLoop::instance().setTarget(slot, 10000);  // Thread-safe
 *   InnerLoop::instance().setEnabled(slot, true);  // Thread-safe
 */
class InnerLoop {
public:
  static InnerLoop& instance() {
    static InnerLoop inst;
    return inst;
  }

  /**
   * Register a motor controller with PID.
   * Task starts automatically on first registration.
   *
   * @param motor Motor controller to drive
   * @param cfg PID configuration
   * @param encoder_idx Which encoder (0-7) provides feedback
   * @param is_velocity true=velocity PID, false=position PID
   * @return slot index (0-7), or -1 if no slots available
   */
  int registerController(motor::IMotorController* motor,
                         const control::PidConfig& cfg,
                         uint8_t encoder_idx,
                         bool is_velocity = false) {
    // Find free slot
    for (size_t i = 0; i < MAX_CONTROLLERS; i++) {
      if (!controllers_[i].active) {
        controllers_[i].motor = motor;
        controllers_[i].encoder_idx = encoder_idx;
        controllers_[i].is_velocity = is_velocity;
        controllers_[i].active = true;
        pids_[i].setConfig(cfg);
        pids_[i].reset();

        ensureStarted();
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  /**
   * Unregister a controller.
   */
  void unregisterController(int slot) {
    if (slot < 0 || slot >= (int)MAX_CONTROLLERS) return;
    controllers_[slot].active = false;
    setpoints_[slot].enabled.store(false, std::memory_order_release);
  }

  /**
   * Set target value (thread-safe, atomic).
   * For position PID: target position in encoder ticks
   * For velocity PID: target velocity in ticks/second
   */
  void setTarget(int slot, float target) {
    if (slot < 0 || slot >= (int)MAX_CONTROLLERS) return;
    setpoints_[slot].target.store(target, std::memory_order_release);
  }

  /**
   * Enable/disable a controller (thread-safe, atomic).
   * Disabled controllers output 0 to motor.
   */
  void setEnabled(int slot, bool enabled) {
    if (slot < 0 || slot >= (int)MAX_CONTROLLERS) return;
    setpoints_[slot].enabled.store(enabled, std::memory_order_release);
  }

  /**
   * Read current output (lock-free).
   */
  const InnerLoopOutput& readOutput() const {
    return output_buffer_.read();
  }

  /**
   * Set loop period in milliseconds.
   */
  void setPeriodMs(uint32_t ms) {
    period_ms_.store(ms, std::memory_order_release);
  }

  uint32_t periodMs() const {
    return period_ms_.load(std::memory_order_acquire);
  }

  bool isRunning() const {
    return started_.load(std::memory_order_acquire);
  }

private:
  InnerLoop() {
    // Initialize PIDs with default config
    control::PidConfig default_cfg{0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f};
    for (size_t i = 0; i < MAX_CONTROLLERS; i++) {
      pids_[i] = control::PID(default_cfg);
    }
  }
  InnerLoop(const InnerLoop&) = delete;
  InnerLoop& operator=(const InnerLoop&) = delete;

  void ensureStarted() {
    if (started_.load(std::memory_order_acquire)) return;

    bool expected = false;
    if (!started_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
      return;
    }

    xTaskCreatePinnedToCore(
      taskEntry,
      "inner",
      STACK_SIZE,
      this,
      configMAX_PRIORITIES - 2,  // High priority, below UART
      &task_handle_,
      CORE_INNER
    );
  }

  static void taskEntry(void* param) {
    static_cast<InnerLoop*>(param)->taskLoop();
  }

  void taskLoop() {
    uint32_t last_run = millis();
    uint8_t last_seq = 0;
    uint32_t iteration = 0;

    for (;;) {
      uint32_t now = millis();
      uint32_t dt_ms = now - last_run;
      last_run = now;

      float dt_s = dt_ms * 0.001f;
      if (dt_s < 0.0001f) dt_s = 0.0001f;  // Prevent div by zero

      // Read encoder frame (COPY to avoid race with UartEncoder task)
      const EncoderFrame enc = UartEncoder::instance().read();

      // Only run if new encoder data available
      if (enc.valid && enc.seq != last_seq) {
        last_seq = enc.seq;

        InnerLoopOutput& out = output_buffer_.backBuffer();

        for (size_t i = 0; i < MAX_CONTROLLERS; i++) {
          if (!controllers_[i].active) {
            out.output[i] = 0.0f;
            out.measurement[i] = 0.0f;
            out.error[i] = 0.0f;
            continue;
          }

          // Read setpoint (atomic)
          float target = setpoints_[i].target.load(std::memory_order_acquire);
          bool enabled = setpoints_[i].enabled.load(std::memory_order_acquire);

          if (!enabled) {
            // Stop motor when disabled
            if (controllers_[i].motor) {
              controllers_[i].motor->setPower(0.0f);
            }
            pids_[i].reset();
            out.output[i] = 0.0f;
            out.measurement[i] = 0.0f;
            out.error[i] = 0.0f;
            continue;
          }

          // Get measurement from encoder
          uint8_t enc_idx = controllers_[i].encoder_idx;
          float measurement;
          if (controllers_[i].is_velocity) {
            // Velocity mode: use deltaTicks / dt
            // deltaTicks is per-frame, dt_us is frame period
            if (enc.dt_us > 0) {
              measurement = (float)enc.deltaTicks[enc_idx] * 1000000.0f / (float)enc.dt_us;
            } else {
              measurement = 0.0f;
            }
          } else {
            // Position mode: use totalTicks
            measurement = (float)enc.totalTicks[enc_idx];
          }

          // PID calculation
          float error = target - measurement;
          float output = pids_[i].step(error, dt_s);

          // Apply to motor
          if (controllers_[i].motor) {
            controllers_[i].motor->setPower(output);
          }

          // Store output
          out.output[i] = output;
          out.measurement[i] = measurement;
          out.error[i] = error;
        }

        out.iteration = ++iteration;
        out.valid = true;
        output_buffer_.swap();
      }

      // Wait for next period
      vTaskDelay(pdMS_TO_TICKS(period_ms_.load(std::memory_order_acquire)));
    }
  }

  // Controller slot
  struct ControllerSlot {
    motor::IMotorController* motor = nullptr;
    uint8_t encoder_idx = 0;
    bool is_velocity = false;
    bool active = false;
  };

  // Thread-safe setpoint (user -> inner loop)
  struct alignas(64) AtomicSetpoint {  // Cache-line aligned to prevent false sharing
    std::atomic<float> target{0.0f};
    std::atomic<bool> enabled{false};
  };

  // State
  std::atomic<bool> started_{false};
  TaskHandle_t task_handle_ = nullptr;
  std::atomic<uint32_t> period_ms_{DEFAULT_INNER_LOOP_PERIOD_MS};

  // Controllers
  ControllerSlot controllers_[MAX_CONTROLLERS] = {};
  control::PID pids_[MAX_CONTROLLERS] = {
    control::PID({0,0,0,0,-1,1}),
    control::PID({0,0,0,0,-1,1}),
    control::PID({0,0,0,0,-1,1}),
    control::PID({0,0,0,0,-1,1}),
    control::PID({0,0,0,0,-1,1}),
    control::PID({0,0,0,0,-1,1}),
    control::PID({0,0,0,0,-1,1}),
    control::PID({0,0,0,0,-1,1})
  };
  AtomicSetpoint setpoints_[MAX_CONTROLLERS] = {};

  // Output buffer
  DoubleBuffer<InnerLoopOutput> output_buffer_;
};

} // namespace probot::prohub
