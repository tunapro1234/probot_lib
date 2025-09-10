#include <probot/devices/leds/builtin.hpp>
#include <Adafruit_NeoPixel.h>

#ifndef NEOPIXEL_PIN
#define NEOPIXEL_PIN 3
#endif
#ifndef NEOPIXEL_COUNT
#define NEOPIXEL_COUNT 1
#endif

namespace probot::builtinled {
  static Adafruit_NeoPixel g_pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
  static uint8_t g_pixel_brightness = 32;
  static bool g_initialized = false;

  static inline void ensureInit(){
    if (!g_initialized){
      g_pixel.begin();
      g_pixel.setBrightness(g_pixel_brightness);
      g_pixel.clear();
      g_pixel.show();
      g_initialized = true;
    }
  }

  void setBrightness(uint8_t brightness){
    g_pixel_brightness = brightness;
    if (g_initialized){
      g_pixel.setBrightness(g_pixel_brightness);
      g_pixel.show();
    }
  }

  void set(bool on){
    ensureInit();
    if (on){ g_pixel.setPixelColor(0, g_pixel.Color(0, 0, 255)); }
    else    { g_pixel.setPixelColor(0, 0); }
    g_pixel.show();
  }

  void setColor(uint8_t r, uint8_t g, uint8_t b){
    ensureInit();
    g_pixel.setPixelColor(0, g_pixel.Color(r, g, b));
    g_pixel.show();
  }
} 