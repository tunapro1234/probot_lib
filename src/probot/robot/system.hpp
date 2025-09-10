#ifndef PROBOT_ROBOT_SYSTEM_HPP
#define PROBOT_ROBOT_SYSTEM_HPP
#pragma once
#include <probot/robot/state.hpp>
#include <probot/io/gamepad.hpp>

namespace probot::robot {
  StateService& state();
}
namespace probot::io {
  GamepadService& gamepad();
}

namespace probot::platform {
  void start_driver_station();
  void driver_station_task(void*);
  void set_driver_station_password(const char* password);
}

// Helper macro for sketches: place at global scope in your .ino before setup
// Example: PROBOT_SET_DRIVER_STATION_PASSWORD("StrongPass123")
#define _PROBOT_CONCAT_INNER(a,b) a##b
#define _PROBOT_CONCAT(a,b) _PROBOT_CONCAT_INNER(a,b)
#define PROBOT_SET_DRIVER_STATION_PASSWORD(PW_LITERAL) \
static_assert((sizeof(PW_LITERAL) - 1) >= 8, "DriverStation password must be at least 8 characters"); \
const char* PROBOT__DS_PASSWORD_REQUIRED = PW_LITERAL; \
namespace { struct _ProbotDsPwSetter { _ProbotDsPwSetter(){ probot::platform::set_driver_station_password(PW_LITERAL); } }; static _ProbotDsPwSetter _PROBOT_CONCAT(_probot_ds_pw_setter_instance_, __LINE__); }

#endif // PROBOT_ROBOT_SYSTEM_HPP 