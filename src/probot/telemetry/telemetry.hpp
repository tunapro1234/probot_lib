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
    volatile uint16_t head = 0;
    volatile uint16_t len = 0;
    volatile uint32_t seq = 0;
  };

  inline TelemetryBuffer g_buffer{};
}

inline void writeBytes(const char* msg, size_t msgLen) {
  auto& buf = detail::g_buffer;
  if (msgLen == 0) return;

  if (msgLen >= detail::BUFFER_SIZE) {
    msg += (msgLen - detail::BUFFER_SIZE);
    msgLen = detail::BUFFER_SIZE;
    buf.head = 0;
    buf.len = 0;
  }

  uint16_t head = buf.head;
  size_t first = detail::BUFFER_SIZE - head;
  if (first > msgLen) first = msgLen;
  memcpy(buf.data + head, msg, first);
  size_t remaining = msgLen - first;
  if (remaining) {
    memcpy(buf.data, msg + first, remaining);
  }
  head = static_cast<uint16_t>((head + msgLen) % detail::BUFFER_SIZE);
  buf.head = head;

  uint16_t newLen = buf.len;
  if (newLen + msgLen >= detail::BUFFER_SIZE) {
    newLen = detail::BUFFER_SIZE;
  } else {
    newLen = static_cast<uint16_t>(newLen + msgLen);
  }
  buf.len = newLen;
  uint32_t s = buf.seq; buf.seq = s + 1;
}

inline void print(const char* msg) {
  writeBytes(msg, strlen(msg));
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
  buf.head = 0;
  buf.len = 0;
  uint32_t s = buf.seq; buf.seq = s + 1;
}

// Internal: DS tarafından çağrılır
inline const char* getBuffer() {
  auto& buf = detail::g_buffer;
  static char out[detail::BUFFER_SIZE + 1];
  uint16_t len = buf.len;
  if (len == 0) {
    out[0] = '\0';
    return out;
  }
  uint16_t head = buf.head;
  uint16_t tail = static_cast<uint16_t>((head + detail::BUFFER_SIZE - len) % detail::BUFFER_SIZE);
  size_t first = detail::BUFFER_SIZE - tail;
  if (first > len) first = len;
  memcpy(out, buf.data + tail, first);
  size_t remaining = len - first;
  if (remaining) {
    memcpy(out + first, buf.data, remaining);
  }
  out[len] = '\0';
  return out;
}

inline uint16_t getLength() {
  return detail::g_buffer.len;
}

inline uint32_t getSeq() {
  return detail::g_buffer.seq;
}

} // namespace probot::telemetry

// Convenience shortcuts in probot:: namespace
namespace probot {
  inline void print(const char* msg) { telemetry::print(msg); }
  inline void println(const char* msg = "") { telemetry::println(msg); }
  inline void printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
  inline void printf(const char* fmt, ...) {
    char tmp[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);
    telemetry::print(tmp);
  }
  inline void clearTelemetry() { telemetry::clear(); }
}
