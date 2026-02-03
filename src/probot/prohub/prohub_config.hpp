#pragma once
#include <stdint.h>
#include <stddef.h>

namespace probot::prohub {

// Encoder configuration
constexpr size_t MAX_ENCODERS = 8;

// UART configuration
constexpr uint32_t DEFAULT_UART_BAUD = 921600;

// Frame format (must match Pico sender)
constexpr uint8_t FRAME_HDR0 = 0xAA;
constexpr uint8_t FRAME_HDR1 = 0x55;
// Frame: [HDR0][HDR1][seq][timestamp_us:4][dt_us:4][totalTicks:8*8][deltaTicks:8*4][crc]
constexpr size_t FRAME_SIZE = 2 + 1 + 4 + 4 + (MAX_ENCODERS * 8) + (MAX_ENCODERS * 4) + 1;  // 108 bytes

// Inner loop configuration
constexpr size_t MAX_CONTROLLERS = 8;
constexpr uint32_t DEFAULT_INNER_LOOP_PERIOD_MS = 3;

// Task configuration
constexpr uint32_t STACK_SIZE = 4096;

// Core assignments
constexpr int CORE_UART = 0;   // UART task on Core 0 (with WiFi)
constexpr int CORE_INNER = 1;  // Inner loop on Core 1 (with scheduler)

} // namespace probot::prohub
