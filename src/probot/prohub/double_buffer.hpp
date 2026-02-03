#pragma once
#include <atomic>
#include <cstring>

namespace probot::prohub {

/**
 * Lock-free double buffer for single-producer, single-consumer pattern.
 *
 * Writer:
 *   T& back = buffer.backBuffer();
 *   back.field = value;
 *   buffer.swap();  // Atomic publish
 *
 * Reader:
 *   const T& front = buffer.read();  // Lock-free, always consistent
 */
template <typename T>
class DoubleBuffer {
public:
  DoubleBuffer() : read_idx_(0) {
    buffers_[0] = T{};
    buffers_[1] = T{};
  }

  // Writer: Get reference to back buffer for writing
  T& backBuffer() {
    int write_idx = 1 - read_idx_.load(std::memory_order_acquire);
    return buffers_[write_idx];
  }

  // Writer: Atomically swap buffers (publishes new data to reader)
  void swap() {
    int new_read = 1 - read_idx_.load(std::memory_order_acquire);
    read_idx_.store(new_read, std::memory_order_release);
  }

  // Reader: Get current front buffer (read-only, lock-free)
  const T& read() const {
    return buffers_[read_idx_.load(std::memory_order_acquire)];
  }

private:
  T buffers_[2];
  std::atomic<int> read_idx_;
};

} // namespace probot::prohub
