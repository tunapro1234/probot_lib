#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

namespace probot::telemetry {

namespace detail {
  constexpr size_t BUFFER_SIZE = 256;

  struct TelemetryBuffer {
    char data[BUFFER_SIZE];
    volatile uint16_t len = 0;
    volatile uint32_t seq = 0;
  };

  inline TelemetryBuffer g_buffer{};
}

inline void print(const char* msg) {
  auto& buf = detail::g_buffer;
  size_t msgLen = strlen(msg);
  size_t space = detail::BUFFER_SIZE - 1 - buf.len;

  if (msgLen > space) {
    // Buffer dolu, baştan yaz
    buf.len = 0;
    space = detail::BUFFER_SIZE - 1;
  }

  size_t toWrite = (msgLen < space) ? msgLen : space;
  memcpy(buf.data + buf.len, msg, toWrite);
  uint16_t oldLen = buf.len; buf.len = static_cast<uint16_t>(oldLen + toWrite);
  buf.data[buf.len] = '\0';
  uint32_t s = buf.seq; buf.seq = s + 1;
}

inline void println(const char* msg = "") {
  print(msg);
  print("\n");
}

inline void printf(const char* fmt, ...) {
  char tmp[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  print(tmp);
}

inline void clear() {
  auto& buf = detail::g_buffer;
  buf.len = 0;
  buf.data[0] = '\0';
  uint32_t s = buf.seq; buf.seq = s + 1;
}

// Internal: DS tarafından çağrılır
inline const char* getBuffer() {
  return detail::g_buffer.data;
}

inline uint16_t getLength() {
  return detail::g_buffer.len;
}

inline uint32_t getSeq() {
  return detail::g_buffer.seq;
}

} // namespace probot::telemetry
