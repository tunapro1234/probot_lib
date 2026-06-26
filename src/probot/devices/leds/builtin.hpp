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
#ifndef NEOPIXEL_BRIGHTNESS
#define NEOPIXEL_BRIGHTNESS 32
#endif

namespace probot::builtinled {
// The status LED is reserved for match-state signaling and is driven solely
// by the runtime sysloop — the color always MEANS something (phase / stalled /
// emergency stop). There is intentionally NO user-facing color API.
//
// `render()` is the single, library-internal entry point. It must be called
// from exactly one task (the sysloop): it performs the blocking NeoPixel RMT
// transmit, so a single caller means no mutex and no reentrancy.
#if defined(ARDUINO)
  namespace detail {
    struct State {
      Adafruit_NeoPixel pixel;
      bool initialized = false;
      State() : pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800) {}
    };
    inline State& state(){ static State s{}; return s; }
  }

  inline void render(uint8_t r, uint8_t g, uint8_t b){
    auto& s = detail::state();
    if (!s.initialized){
      s.pixel.begin();
      s.initialized = true;
    }
    s.pixel.setBrightness(NEOPIXEL_BRIGHTNESS);
    s.pixel.setPixelColor(0, s.pixel.Color(r, g, b));
    s.pixel.show();
  }
#else
  inline void render(uint8_t, uint8_t, uint8_t) {}
#endif
} // namespace probot::builtinled
