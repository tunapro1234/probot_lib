#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace probot {
  // Core 0 runs WiFi/httpd/sysloop ("UI"), core 1 runs user code ("CTRL").
  constexpr int CORE_UI   = 0;
  constexpr int CORE_CTRL = 1;

  constexpr UBaseType_t PRIO_CTRL  = 4;
  constexpr UBaseType_t PRIO_USER  = 1;

  constexpr uint32_t STACK_CTRL  = 4096;
  constexpr uint32_t STACK_USER  = 4096;

  constexpr uint32_t END_KILL_TIMEOUT_MS = 1000;
}
