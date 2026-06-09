#pragma once

// Short-critical-section lock shared by the state/gamepad/telemetry
// containers.
//
// On ESP32 this MUST be a portMUX critical section, not a CAS spinlock:
// these locks are contended between tasks of different priorities pinned
// to the same core (e.g. sysloop prio 4 vs ds_push prio 3 on core 0). A
// raw spin loop in the higher-priority task starves the lock holder
// forever — an unrecoverable livelock. portENTER_CRITICAL masks
// scheduling on the core while held, which both prevents the inversion
// and guarantees the holder cannot be preempted or killed mid-section.
// Sections guarded by this lock must stay tiny (a few hundred bytes of
// memcpy, no I/O, no blocking calls).
//
// On host (unit tests, single-threaded) a plain CAS lock suffices.

#ifdef ESP32
#include <freertos/FreeRTOS.h>

namespace probot::core {
  class Mux {
  public:
    void lock() const { portENTER_CRITICAL(&_mux); }
    void unlock() const { portEXIT_CRITICAL(&_mux); }
  private:
    mutable portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;
  };
}

#else

namespace probot::core {
  class Mux {
  public:
    void lock() const {
      unsigned expected = 0;
      while (!__atomic_compare_exchange_n(&_word, &expected, 1u, false,
                                          __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        expected = 0;
      }
    }
    void unlock() const { __atomic_store_n(&_word, 0u, __ATOMIC_RELEASE); }
  private:
    mutable volatile unsigned _word = 0;
  };
}

#endif

namespace probot::core {
  class MuxGuard {
  public:
    explicit MuxGuard(const Mux& m) : _m(m) { _m.lock(); }
    ~MuxGuard() { _m.unlock(); }
    MuxGuard(const MuxGuard&) = delete;
    MuxGuard& operator=(const MuxGuard&) = delete;
  private:
    const Mux& _m;
  };
}
