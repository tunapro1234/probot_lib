#ifndef PROBOT_RUNTIME_HPP
#define PROBOT_RUNTIME_HPP
#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace probot {
  void runtime_setup();
}

// User hooks (provided by sketch)
void robotInit();
void robotEnd();
void teleopInit();
void teleopLoop();
void autonomousInit();
void autonomousLoop();

#endif // PROBOT_RUNTIME_HPP 