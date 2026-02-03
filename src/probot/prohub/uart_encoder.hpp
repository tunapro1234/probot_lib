#pragma once
#include <Arduino.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <probot/prohub/prohub_config.hpp>
#include <probot/prohub/double_buffer.hpp>
#include <probot/prohub/encoder_frame.hpp>

namespace probot::prohub {

/**
 * UART Encoder Reader - Reads encoder frames from Pico co-processor.
 *
 * Usage:
 *   UartEncoder::instance().configure(Serial1, 10, 11);
 *   // Task starts automatically on first read()
 *   const auto& frame = UartEncoder::instance().read();
 */
class UartEncoder {
public:
  static UartEncoder& instance() {
    static UartEncoder inst;
    return inst;
  }

  /**
   * Configure UART connection to Pico.
   * Call this in robotInit() before using read().
   */
  void configure(HardwareSerial& serial, int rx_pin, int tx_pin, uint32_t baud = DEFAULT_UART_BAUD) {
    serial_ = &serial;
    rx_pin_ = rx_pin;
    tx_pin_ = tx_pin;
    baud_ = baud;
  }

  /**
   * Read current encoder frame (lock-free).
   * Task starts automatically on first call if configured.
   */
  const EncoderFrame& read() {
    ensureStarted();
    return frame_buffer_.read();
  }

  // Statistics
  uint32_t frameCount() const { return frame_count_.load(std::memory_order_relaxed); }
  uint32_t crcErrors() const { return crc_errors_.load(std::memory_order_relaxed); }
  uint32_t bytesReceived() const { return bytes_received_.load(std::memory_order_relaxed); }

  // Check if task is running
  bool isRunning() const { return started_.load(std::memory_order_acquire); }

private:
  UartEncoder() = default;
  UartEncoder(const UartEncoder&) = delete;
  UartEncoder& operator=(const UartEncoder&) = delete;

  void ensureStarted() {
    if (started_.load(std::memory_order_acquire)) return;
    if (!serial_) return;  // Not configured

    // Double-check with atomic exchange
    bool expected = false;
    if (!started_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
      return;  // Another thread started it
    }

    // Initialize UART
    serial_->begin(baud_, SERIAL_8N1, rx_pin_, tx_pin_);

    // Start task on Core 0
    xTaskCreatePinnedToCore(
      taskEntry,
      "uart_enc",
      STACK_SIZE,
      this,
      configMAX_PRIORITIES - 1,  // Highest priority
      &task_handle_,
      CORE_UART
    );
  }

  static void taskEntry(void* param) {
    static_cast<UartEncoder*>(param)->taskLoop();
  }

  void taskLoop() {
    // Parser state machine
    enum class State { SYNC0, SYNC1, PAYLOAD };
    State state = State::SYNC0;
    uint8_t rx_buf[FRAME_SIZE];
    size_t rx_idx = 0;

    for (;;) {
      while (serial_->available()) {
        uint8_t b = serial_->read();
        bytes_received_.fetch_add(1, std::memory_order_relaxed);

        switch (state) {
          case State::SYNC0:
            if (b == FRAME_HDR0) state = State::SYNC1;
            break;

          case State::SYNC1:
            if (b == FRAME_HDR1) {
              state = State::PAYLOAD;
              rx_idx = 0;
            } else if (b == FRAME_HDR0) {
              // Stay in SYNC1 (consecutive 0xAA)
            } else {
              state = State::SYNC0;
            }
            break;

          case State::PAYLOAD:
            rx_buf[rx_idx++] = b;
            if (rx_idx >= FRAME_SIZE - 2) {  // Got full payload
              // Verify CRC
              uint8_t full_buf[FRAME_SIZE];
              full_buf[0] = FRAME_HDR0;
              full_buf[1] = FRAME_HDR1;
              memcpy(&full_buf[2], rx_buf, rx_idx);

              uint8_t calc_crc = crc8_xor(full_buf, FRAME_SIZE - 1);
              uint8_t recv_crc = rx_buf[rx_idx - 1];

              if (calc_crc == recv_crc) {
                parseFrame(rx_buf);
                frame_count_.fetch_add(1, std::memory_order_relaxed);
              } else {
                crc_errors_.fetch_add(1, std::memory_order_relaxed);
              }

              state = State::SYNC0;
            }
            break;
        }
      }

      // Small yield to prevent watchdog issues
      vTaskDelay(1);
    }
  }

  void parseFrame(const uint8_t* buf) {
    EncoderFrame& f = frame_buffer_.backBuffer();

    size_t p = 0;
    f.seq = buf[p++];
    f.timestamp_us = read_u32_le(&buf[p]); p += 4;
    f.dt_us = read_u32_le(&buf[p]); p += 4;

    for (size_t i = 0; i < MAX_ENCODERS; i++) {
      f.totalTicks[i] = read_i64_le(&buf[p]); p += 8;
    }
    for (size_t i = 0; i < MAX_ENCODERS; i++) {
      f.deltaTicks[i] = read_i32_le(&buf[p]); p += 4;
    }
    f.valid = true;

    frame_buffer_.swap();
  }

  // Parse helpers
  static uint8_t crc8_xor(const uint8_t* d, size_t n) {
    uint8_t c = 0;
    for (size_t i = 0; i < n; i++) c ^= d[i];
    return c;
  }

  static uint32_t read_u32_le(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
  }

  static int32_t read_i32_le(const uint8_t* p) {
    return (int32_t)read_u32_le(p);
  }

  static int64_t read_i64_le(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) v |= ((uint64_t)p[i] << (8 * i));
    return (int64_t)v;
  }

  // Configuration
  HardwareSerial* serial_ = nullptr;
  int rx_pin_ = -1;
  int tx_pin_ = -1;
  uint32_t baud_ = DEFAULT_UART_BAUD;

  // State
  std::atomic<bool> started_{false};
  TaskHandle_t task_handle_ = nullptr;
  DoubleBuffer<EncoderFrame> frame_buffer_;

  // Statistics
  std::atomic<uint32_t> frame_count_{0};
  std::atomic<uint32_t> crc_errors_{0};
  std::atomic<uint32_t> bytes_received_{0};
};

} // namespace probot::prohub
