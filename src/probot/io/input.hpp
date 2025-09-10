#ifndef PROBOT_INPUT_HPP
#define PROBOT_INPUT_HPP
#pragma once
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace probot {
  struct InputState {
    uint32_t seq;
    uint32_t ms;
    int16_t  x;
    int16_t  y;
    uint8_t  btn;
    uint8_t  _pad[3];
  };

  InputState read_input_snapshot();
  void uiTask(void*);
}
#endif // PROBOT_INPUT_HPP 