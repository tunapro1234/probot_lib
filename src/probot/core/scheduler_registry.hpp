#pragma once
#include <stdint.h>

namespace probot::detail {
  using SchedulerInitFn = void (*)(uint8_t queue_len);
  using SchedulerTaskFn = void (*)(void*);

  inline SchedulerInitFn g_scheduler_init = nullptr;
  inline SchedulerTaskFn g_scheduler_task = nullptr;
} // namespace probot::detail
