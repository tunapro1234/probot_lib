#pragma once
#include <stdint.h>
#include <probot/prohub/prohub_config.hpp>

namespace probot::prohub {

/**
 * Encoder frame received from Pico via UART.
 *
 * Wire format (108 bytes):
 *   [AA][55][seq:1][timestamp_us:4][dt_us:4][totalTicks:64][deltaTicks:32][crc:1]
 */
struct EncoderFrame {
  uint8_t  seq = 0;                        // Sequence number (wraps at 256)
  uint32_t timestamp_us = 0;               // Pico timestamp (micros)
  uint32_t dt_us = 0;                      // Delta time since last frame
  int64_t  totalTicks[MAX_ENCODERS] = {};  // Cumulative encoder ticks
  int32_t  deltaTicks[MAX_ENCODERS] = {};  // Delta ticks since last frame
  bool     valid = false;                  // True if frame was successfully parsed
};

} // namespace probot::prohub
