#pragma once
#include <stdint.h>

#if defined(ARDUINO)
#include <Adafruit_NeoPixel.h>
#endif
#if defined(ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
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
#if defined(ESP32)
      // The sysloop blinks the status LED while user code may also call
      // setColor — NeoPixel show() is not reentrant (shared RMT).
      SemaphoreHandle_t mtx = xSemaphoreCreateMutex();
#endif

      BuiltinLedState() : pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800) {}

      void lock(){
#if defined(ESP32)
        if (mtx) xSemaphoreTake(mtx, portMAX_DELAY);
#endif
      }
      void unlock(){
#if defined(ESP32)
        if (mtx) xSemaphoreGive(mtx);
#endif
      }
    };

    inline BuiltinLedState& state(){
      static BuiltinLedState s{};
      return s;
    }

    // Caller must hold the lock.
    inline void ensureInitLocked(BuiltinLedState& s){
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
    s.lock();
    s.brightness = brightness;
    if (s.initialized){
      s.pixel.setBrightness(s.brightness);
      s.pixel.show();
    }
    s.unlock();
  }

  inline void set(bool on){
    auto& s = detail::state();
    s.lock();
    detail::ensureInitLocked(s);
    if (on){ s.pixel.setPixelColor(0, s.pixel.Color(0, 0, 255)); }
    else { s.pixel.setPixelColor(0, 0); }
    s.pixel.show();
    s.unlock();
  }

  inline void setColor(uint8_t r, uint8_t g, uint8_t b){
    auto& s = detail::state();
    s.lock();
    detail::ensureInitLocked(s);
    s.pixel.setPixelColor(0, s.pixel.Color(r, g, b));
    s.pixel.show();
    s.unlock();
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
