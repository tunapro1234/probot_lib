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
  // One persistent user task hosts every FTC-style OpMode hook.
  constexpr uint32_t STACK_USER  = 8192;
}

// User cadence: how often initLoop/loop hooks are called (~50 Hz).
#ifndef USER_LOOP_PERIOD_MS
#define USER_LOOP_PERIOD_MS 20
#endif
