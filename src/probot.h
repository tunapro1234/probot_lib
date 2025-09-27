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
#include <probot/controllers/imotor_controller.hpp>
#include <probot/devices/motors/motor_group.hpp>

// Sensors
#include <probot/sensors/encoder.hpp>

// Controllers
#include <probot/controllers/blink_pid.hpp>
#include <probot/controllers/pid.hpp>
#include <probot/controllers/closed_loop_motor.hpp>
#include <probot/controllers/closed_loop_motor_group.hpp>
#include <probot/controllers/slider.hpp>
#include <probot/controllers/basic_tank_drive.hpp>
#include <probot/controllers/elevator.hpp>
#include <probot/controllers/turret.hpp>
#include <probot/controllers/arm.hpp>
#include <probot/controllers/telescopic_tube.hpp>

// NFR presets
#include <probot/nfr/slider.hpp>
#include <probot/nfr/elevator.hpp>
#include <probot/nfr/turret.hpp>
#include <probot/nfr/arm.hpp>
#include <probot/nfr/telescopic_tube.hpp>

// Simulation
#include <probot/sim/sim_motor.hpp>
#include <probot/sim/sim_encoder.hpp>
#include <probot/sim/sim_plant.hpp>

// Robot
#include <probot/robot/state.hpp>
#include <probot/robot/system.hpp> 
