#pragma once
#include <Arduino.h>

// Core
#include <probot/core/core_config.hpp>
#include <probot/core/wdt.hpp>
#include <probot/core/scheduler.hpp>
#include <probot/core/runtime.hpp>

// IO
#include <probot/io/time.hpp>
#include <probot/io/gamepad.hpp>

// Devices
#include <probot/devices/leds/builtin.hpp>
#include <probot/devices/motors/imotor_controller.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_driver.hpp>
#include <probot/devices/motors/bts7960b_motor_driver.hpp>
#include <probot/devices/motors/motor_controller_group.hpp>

// Sensors
#include <probot/sensors/encoder.hpp>
#include <probot/sensors/imu/imu.hpp>
#include <probot/sensors/imu/mpu6050.hpp>

// Controllers
#include <probot/control/blink_pid.hpp>
#include <probot/control/control_types.hpp>
#include <probot/control/pid.hpp>
#include <probot/control/pid_motor_controller.hpp>
#include <probot/control/pid_motor_controller_group.hpp>
#include <probot/control/geometry.hpp>
#include <probot/control/feedforward/simple_motor_ff.hpp>
#include <probot/control/feedforward/arm_ff.hpp>
#include <probot/control/feedforward/elevator_ff.hpp>
#include <probot/control/bang_bang_controller.hpp>
#include <probot/control/limiters/slew_rate_limiter.hpp>
#include <probot/control/trajectory/ramsete_controller.hpp>
#include <probot/control/trajectory/holonomic_drive_controller.hpp>
#include <probot/control/kinematics/differential_drive_kinematics.hpp>
#include <probot/control/kinematics/mecanum_drive_kinematics.hpp>
#include <probot/control/odometry/differential_drive_odometry.hpp>
#include <probot/control/odometry/mecanum_drive_odometry.hpp>
#include <probot/control/state_space/matrix.hpp>
#include <probot/control/state_space/luenberger_observer.hpp>
#include <probot/control/state_space/kalman_filter.hpp>
#include <probot/control/state_space/lqr.hpp>
#include <probot/control/estimation/pose_estimator.hpp>

// Mechanisms
#include <probot/mechanism/slider.hpp>
#include <probot/mechanism/elevator.hpp>
#include <probot/mechanism/turret.hpp>
#include <probot/mechanism/arm.hpp>
#include <probot/mechanism/telescopic_tube.hpp>

// Chassis (Drive)
#include <probot/chassis/tank_drive.hpp>
#include <probot/chassis/mecanum_drive.hpp>

// NFR presets
#include <probot/mechanism/nfr/slider.hpp>
#include <probot/mechanism/nfr/shooter.hpp>
#include <probot/mechanism/nfr/elevator.hpp>
#include <probot/mechanism/nfr/turret.hpp>
#include <probot/mechanism/nfr/arm.hpp>
#include <probot/mechanism/nfr/telescopic_tube.hpp>

// Robot
#include <probot/robot/state.hpp>
#include <probot/robot/system.hpp> 

namespace probot {
  namespace nfr = probot::mechanism::nfr;
}

namespace probot::control {
  namespace ff = probot::control::feedforward;
  namespace limiter = probot::control::limiters;
  namespace traj = probot::control::trajectory;
  namespace kinematics = probot::control::kinematics;
  namespace odometry = probot::control::odometry;
  namespace ss = probot::control::state_space;
  namespace est = probot::control::estimation;
}
