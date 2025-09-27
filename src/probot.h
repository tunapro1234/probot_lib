#pragma once
#include <Arduino.h>

// Core
#include <probot/core/core_config.hpp>
#include <probot/core/wdt.hpp>
#include <probot/core/scheduler.hpp>
#include <probot/core/runtime.hpp>

// IO
#include <probot/io/input.hpp>
#include <probot/io/joystick.hpp>
#include <probot/io/time.hpp>
#include <probot/io/gamepad.hpp>

// Devices
#include <probot/devices/leds/builtin.hpp>
#include <probot/devices/motors/imotor_driver.hpp>
#include <probot/devices/motors/boardoza_ba6208_driver.hpp>
#include <probot/control/imotor_controller.hpp>
#include <probot/devices/motors/motor_group.hpp>

// Sensors
#include <probot/sensors/encoder.hpp>

// Controllers
#include <probot/control/blink_pid.hpp>
#include <probot/control/pid.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/closed_loop_motor_group.hpp>
#include <probot/control/feedforward/simple_motor_ff.hpp>
#include <probot/control/feedforward/arm_ff.hpp>
#include <probot/control/feedforward/elevator_ff.hpp>
#include <probot/control/motion_profile/trapezoid_profile.hpp>
#include <probot/control/motion_profile/s_curve_profile.hpp>

// Mechanisms
#include <probot/mechanism/slider.hpp>
#include <probot/mechanism/elevator.hpp>
#include <probot/mechanism/turret.hpp>
#include <probot/mechanism/arm.hpp>
#include <probot/mechanism/telescopic_tube.hpp>

// Drive
#include <probot/drive/basic_tank_drive.hpp>

// NFR presets
#include <probot/mechanism/nfr/slider.hpp>
#include <probot/mechanism/nfr/elevator.hpp>
#include <probot/mechanism/nfr/turret.hpp>
#include <probot/mechanism/nfr/arm.hpp>
#include <probot/mechanism/nfr/telescopic_tube.hpp>

// Simulation
#include <probot/sim/sim_motor.hpp>
#include <probot/sim/sim_encoder.hpp>
#include <probot/sim/sim_plant.hpp>

// Robot
#include <probot/robot/state.hpp>
#include <probot/robot/system.hpp> 

namespace probot {
  namespace nfr = probot::mechanism::nfr;
}

namespace probot::control {
  namespace ff = probot::control::feedforward;
  namespace profile = probot::control::motion_profile;
}
