#ifndef PROBOT_CORE_CONFIG_HPP
#define PROBOT_CORE_CONFIG_HPP
#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifndef PROBOT_WITH_DS
#define PROBOT_WITH_DS 1
#endif

namespace probot {
  constexpr int CORE_UI   = 0;
  constexpr int CORE_CTRL = 1;

  constexpr UBaseType_t PRIO_CTRL  = 4;
  constexpr UBaseType_t PRIO_STATE = 5; // state manager higher than scheduler
  constexpr UBaseType_t PRIO_USER  = 1;
  constexpr UBaseType_t PRIO_UI    = 3;

  constexpr uint32_t STACK_UI    = 4096;
  constexpr uint32_t STACK_CTRL  = 4096;
  constexpr uint32_t STACK_USER  = 4096;

  constexpr uint32_t INIT_KILL_TIMEOUT_MS = 3000;
  constexpr uint32_t END_KILL_TIMEOUT_MS  = 1000;
}
#endif // PROBOT_CORE_CONFIG_HPP 