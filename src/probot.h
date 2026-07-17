#pragma once
#include <Arduino.h>

// FTC-style user OpMode hooks. The recurring loop and stop hooks are required:
// omitting any one produces a linker error. INIT-loop and START are advanced,
// optional hooks; an undefined weak declaration is bound to the empty default
// by the runtime. A sketch definition overrides it.
//
// [PB-E201] "undefined reference to `teleopLoop()`" (veya autonomousLoop/
//   autonomousStop/teleopStop): zorunlu bir hook sketch'te tanımlanmamış.
//   Boş da olsa dördünü tanımlayın. Docs: probotstudio.com/docs/hatalar/#pb-e201
// [PB-E202] "multiple definition of `setup()`/`loop()`": setup()/loop()
//   kütüphaneye aittir, sketch'te TANIMLANMAZ — hook'ları kullanın.
//   Docs: probotstudio.com/docs/hatalar/#pb-e202
void autonomousInit() __attribute__((weak));
void autonomousInitLoop() __attribute__((weak));
void autonomousStart() __attribute__((weak));
void autonomousLoop();
void autonomousStop();
void teleopInit() __attribute__((weak));
void teleopInitLoop() __attribute__((weak));
void teleopStart() __attribute__((weak));
void teleopLoop();
void teleopStop();

// Core runtime
#include <probot/core/core_config.hpp>
#include <probot/core/wdt.hpp>
#include <probot/core/runtime.hpp>

// Driver station communication
#include <probot/io/gamepad.hpp>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/leds/builtin.hpp>
#include <probot/robot/state.hpp>
#include <probot/robot/system.hpp>
#include <probot/telemetry/telemetry.hpp>
