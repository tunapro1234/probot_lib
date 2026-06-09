#pragma once
#include <stdint.h>
#include <Arduino.h>

// How long after the last received joystick packet axes/buttons keep
// their values before read() returns neutral. FRC uses 125 ms, FTC
// ~300 ms, WPILib XRP 500 ms; tighten for faster failsafe.
#ifndef PROBOT_INPUT_TIMEOUT_MS
#define PROBOT_INPUT_TIMEOUT_MS 500
#endif

namespace probot::io {
  struct GamepadSnapshot {
    uint32_t ms;
    uint32_t seq;
    uint32_t axisCount;
    uint32_t buttonCount;
    float    axes[20];
    bool     buttons[20];
  };

  class IGamepadSource {
  public:
    virtual GamepadSnapshot read() const = 0;
    virtual ~IGamepadSource() {}
  };

  class GamepadService : public IGamepadSource {
  public:
    GamepadService(){
      _cur = 0;
      GamepadSnapshot z{};
      _buf[0] = z;
      _buf[1] = z;
      _timeout_ms = PROBOT_INPUT_TIMEOUT_MS;
    }

    // Written from two tasks (WS/HTTP handlers push frames, sysloop
    // zeroes state on owner release) — the spinlock serializes writers;
    // readers stay lock-free on the double buffer.
    void write(uint32_t now_ms, const float* axes, uint32_t nAxis, const bool* buttons, uint32_t nButton){
      lock();
      uint32_t cur = __atomic_load_n(&_cur, __ATOMIC_SEQ_CST);
      uint32_t w   = 1u - cur;
      GamepadSnapshot s = _buf[cur];
      s.ms = now_ms;
      s.seq++;
      s.axisCount = (nAxis>20?20:nAxis);
      s.buttonCount = (nButton>20?20:nButton);
      for (uint32_t i=0;i<s.axisCount;i++){ s.axes[i] = axes[i]; }
      for (uint32_t i=0;i<s.buttonCount;i++){ s.buttons[i] = buttons[i]; }
      __atomic_thread_fence(__ATOMIC_SEQ_CST);
      _buf[w] = s;
      __atomic_store_n(&_cur, w, __ATOMIC_SEQ_CST);
      unlock();
    }

    void setTimeoutMs(uint32_t timeout_ms){
      __atomic_store_n(&_timeout_ms, timeout_ms, __ATOMIC_SEQ_CST);
    }

    GamepadSnapshot read() const override {
      uint32_t idx = __atomic_load_n(&_cur, __ATOMIC_SEQ_CST);
      GamepadSnapshot s = _buf[idx];
      uint32_t timeout_ms = __atomic_load_n(&_timeout_ms, __ATOMIC_SEQ_CST);
      if (timeout_ms > 0){
        uint32_t now_ms = millis();
        if ((uint32_t)(now_ms - s.ms) > timeout_ms){
          GamepadSnapshot z{};
          z.ms = now_ms;
          z.seq = s.seq;
          return z;
        }
      }
      return s;
    }

  private:
    void lock() const {
      uint32_t expected = 0;
      while (!__atomic_compare_exchange_n(&_write_lock, &expected, 1u, false,
                                          __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        expected = 0;
      }
    }

    void unlock() const { __atomic_store_n(&_write_lock, 0u, __ATOMIC_RELEASE); }

    mutable GamepadSnapshot _buf[2];
    mutable volatile uint32_t _cur;
    mutable volatile uint32_t _timeout_ms;
    mutable volatile uint32_t _write_lock = 0;
  };
}
