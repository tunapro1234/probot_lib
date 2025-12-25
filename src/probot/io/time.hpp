#ifndef PROBOT_TIME_HPP
#define PROBOT_TIME_HPP
#pragma once
#include <stdint.h>

namespace probot {
  struct UiState { uint32_t seq; uint32_t ms; };
  UiState read_ui_snapshot();
}
#endif // PROBOT_TIME_HPP
