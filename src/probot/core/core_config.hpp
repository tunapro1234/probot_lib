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
  // One persistent user task now hosts all six hooks (init/end/teleop/auto),
  // so it gets the headroom the four separate worker stacks used to split.
  constexpr uint32_t STACK_USER  = 8192;
}

// User loop cadence: how often teleopLoop/autonomousLoop are called (~50 Hz).
#ifndef USER_LOOP_PERIOD_MS
#define USER_LOOP_PERIOD_MS 20
#endif
