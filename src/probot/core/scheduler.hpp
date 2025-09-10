#ifndef PROBOT_CONTROL_SCHEDULER_HPP
#define PROBOT_CONTROL_SCHEDULER_HPP
#pragma once
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace control {
  struct IUpdatable {
    virtual void update(uint32_t now_ms, uint32_t dt_ms) = 0;
    virtual ~IUpdatable() {}
  };

  void init(uint8_t queue_len = 8);
  void setGlobalPeriodMs(uint32_t period_ms);
  bool attach(IUpdatable* obj);
  bool detach(IUpdatable* obj);
  size_t count();

  extern QueueHandle_t qCmd;
  void schedulerTask(void*);
}
#endif // PROBOT_CONTROL_SCHEDULER_HPP 