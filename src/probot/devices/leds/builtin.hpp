#pragma once
#include <stdint.h>

namespace probot::builtinled {
  void set(bool on);
  void setBrightness(uint8_t brightness);
  void setColor(uint8_t r, uint8_t g, uint8_t b);
} 