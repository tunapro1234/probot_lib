#pragma once
#include <Arduino.h>

// Core runtime
#include <probot/core/core_config.hpp>
#include <probot/core/wdt.hpp>
#include <probot/core/runtime.hpp>

// DriverStation communication
#include <probot/io/gamepad.hpp>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/leds/builtin.hpp>
#include <probot/robot/state.hpp>
#include <probot/robot/system.hpp>
#include <probot/telemetry/telemetry.hpp>

// Optional modules (explicit include):
// - <probot/command.hpp> (command + subsystem)
// - <probot/devices/...>, <probot/control/...>
// - <probot/command/examples/...> (mechanism + chassis examples)
