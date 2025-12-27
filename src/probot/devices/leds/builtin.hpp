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
  namespace detail {
    struct BuiltinLedState {
      Adafruit_NeoPixel pixel;
      uint8_t brightness = 32;
      bool initialized = false;

      BuiltinLedState() : pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800) {}
    };

    inline BuiltinLedState& state(){
      static BuiltinLedState s{};
      return s;
    }

    inline void ensureInit(){
      auto& s = state();
      if (!s.initialized){
        s.pixel.begin();
        s.pixel.setBrightness(s.brightness);
        s.pixel.clear();
        s.pixel.show();
        s.initialized = true;
      }
    }
  } // namespace detail

  inline void setBrightness(uint8_t brightness){
    auto& s = detail::state();
    s.brightness = brightness;
    if (s.initialized){
      s.pixel.setBrightness(s.brightness);
      s.pixel.show();
    }
  }

  inline void set(bool on){
    detail::ensureInit();
    auto& s = detail::state();
    if (on){ s.pixel.setPixelColor(0, s.pixel.Color(0, 0, 255)); }
    else { s.pixel.setPixelColor(0, 0); }
    s.pixel.show();
  }

  inline void setColor(uint8_t r, uint8_t g, uint8_t b){
    detail::ensureInit();
    auto& s = detail::state();
    s.pixel.setPixelColor(0, s.pixel.Color(r, g, b));
    s.pixel.show();
  }
#elif defined(PROBOT_BUILTINLED_EXTERNAL)
  void set(bool on);
  void setBrightness(uint8_t brightness);
  void setColor(uint8_t r, uint8_t g, uint8_t b);
#else
  inline void set(bool) {}
  inline void setBrightness(uint8_t) {}
  inline void setColor(uint8_t, uint8_t, uint8_t) {}
#endif
} // namespace probot::builtinled
