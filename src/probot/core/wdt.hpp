#ifndef PROBOT_WDT_HPP
#define PROBOT_WDT_HPP
#pragma once
#include <stdint.h>

namespace probot {
  void wdt_init_no_idle(uint32_t timeout_s = 3, bool panic_on = true);
}
#endif // PROBOT_WDT_HPP 