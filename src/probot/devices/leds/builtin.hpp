#pragma once
#include <stdint.h>

#if defined(ARDUINO)
#include <Adafruit_NeoPixel.h>
#endif

#ifndef NEOPIXEL_PIN
#define NEOPIXEL_PIN 3
#endif
#ifndef NEOPIXEL_COUNT
#define NEOPIXEL_COUNT 1
#endif

namespace probot::builtinled {
#if defined(ARDUINO)
  // Lock-free status LED.
  //
  // The blocking RMT transmit (pixel.show()) used to run under a
  // portMAX_DELAY FreeRTOS mutex held across the transmit. A task killed
  // mid-show() orphaned that mutex forever, and the sysloop — which also
  // drives the LED — would then wedge on it. To remove that whole class of
  // bug: setColor/set/setBrightness only stash the desired state in one
  // atomic word; the actual show() happens in flush(), which is called from
  // exactly one task (the sysloop). No mutex, single show() caller, no
  // reentrancy. User setColor() takes effect at the next flush (<=500 ms);
  // the sysloop overwrites it each tick with the status color, as before.
  namespace detail {
    // packed desired state: [31:24]=brightness [23:16]=r [15:8]=g [7:0]=b
    inline volatile uint32_t g_word = (32u << 24);

    struct BuiltinLedState {
      Adafruit_NeoPixel pixel;
      bool initialized = false;
      BuiltinLedState() : pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800) {}
    };

    inline BuiltinLedState& state(){
      static BuiltinLedState s{};
      return s;
    }
  } // namespace detail

  inline void setColor(uint8_t r, uint8_t g, uint8_t b){
    uint32_t w = __atomic_load_n(&detail::g_word, __ATOMIC_RELAXED);
    w = (w & 0xFF000000u) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    __atomic_store_n(&detail::g_word, w, __ATOMIC_RELAXED);
  }

  inline void setBrightness(uint8_t brightness){
    uint32_t w = __atomic_load_n(&detail::g_word, __ATOMIC_RELAXED);
    w = (w & 0x00FFFFFFu) | ((uint32_t)brightness << 24);
    __atomic_store_n(&detail::g_word, w, __ATOMIC_RELAXED);
  }

  inline void set(bool on){
    setColor(0, 0, on ? 255 : 0);
  }

  // Push the stashed color to the strip. MUST be called from a single task
  // (the sysloop) — it performs the blocking RMT transmit.
  inline void flush(){
    auto& s = detail::state();
    uint32_t w = __atomic_load_n(&detail::g_word, __ATOMIC_RELAXED);
    uint8_t br = (uint8_t)(w >> 24), r = (uint8_t)(w >> 16),
            g = (uint8_t)(w >> 8), b = (uint8_t)w;
    if (!s.initialized){
      s.pixel.begin();
      s.initialized = true;
    }
    s.pixel.setBrightness(br);
    s.pixel.setPixelColor(0, s.pixel.Color(r, g, b));
    s.pixel.show();
  }
#elif defined(PROBOT_BUILTINLED_EXTERNAL)
  void set(bool on);
  void setBrightness(uint8_t brightness);
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  inline void flush() {}
#else
  inline void set(bool) {}
  inline void setBrightness(uint8_t) {}
  inline void setColor(uint8_t, uint8_t, uint8_t) {}
  inline void flush() {}
#endif
} // namespace probot::builtinled
