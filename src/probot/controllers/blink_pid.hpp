#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <probot/core/scheduler.hpp>
#include <probot/devices/leds/builtin.hpp>

namespace probot::controllers {
  class BlinkPid : public ::control::IUpdatable {
  public:
    BlinkPid()
    : current_reference_(0), pending_reference_(0), has_pending_reference_(false), led_is_on_(false) {}

    void setReference(uint32_t new_reference){
      __atomic_store_n(&pending_reference_, new_reference, __ATOMIC_SEQ_CST);
      __atomic_store_n(&has_pending_reference_, true, __ATOMIC_SEQ_CST);
    }

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      (void)now_ms; (void)dt_ms;
      bool has_pending = __atomic_exchange_n(&has_pending_reference_, false, __ATOMIC_SEQ_CST);
      if (has_pending){
        uint32_t ref = __atomic_load_n(&pending_reference_, __ATOMIC_SEQ_CST);
        current_reference_ = ref;
        led_is_on_ = !led_is_on_;
        probot::builtinled::set(led_is_on_);
        Serial.printf("[PID  ] update: ref=%lu -> led=%s\n", (unsigned long)ref, led_is_on_ ? "ON" : "OFF");
      }
    }

  private:
    uint32_t current_reference_;
    uint32_t pending_reference_;
    bool     has_pending_reference_;
    bool     led_is_on_;
  };
} // namespace probot::controllers 
