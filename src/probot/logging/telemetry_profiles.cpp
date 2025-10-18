#include <probot/logging/telemetry_profiles.hpp>
#include <math.h>
#include <probot/control/imotor_controller.hpp>
#include <probot/mechanism/slider.hpp>
#include <probot/mechanism/arm.hpp>
#include <probot/mechanism/elevator.hpp>
#include <probot/mechanism/telescopic_tube.hpp>
#include <probot/mechanism/turret.hpp>
#include <probot/chassis/basic_tank_drive.hpp>
#include <probot/control/blink_pid.hpp>

namespace probot::logging::profiles {

  void motorControllerDynamic(const void* object,
                              TelemetryCollector& collector,
                              uint32_t now_ms,
                              uint32_t dt_ms){
    (void)now_ms; (void)dt_ms;
    auto ctrl = static_cast<const probot::control::IMotorController*>(object);
    if (!ctrl) return;
    float setpoint = ctrl->lastSetpoint();
    float meas     = ctrl->lastMeasurement();
    float output   = ctrl->lastOutput();
    collector.addFloat("setpoint", setpoint, Priority::kUserMarked);
    collector.addFloat("measurement", meas, Priority::kUserMarked);
    collector.addFloat("output", output, Priority::kUserMarked);
    collector.addFloat("error", setpoint - meas, Priority::kUserMarked);
    collector.addInt("mode", static_cast<int32_t>(ctrl->activeMode()), Priority::kBackground);
    collector.addInt("motion_profile", static_cast<int32_t>(ctrl->motionProfile()), Priority::kBackground);
  }

  void motorControllerStatic(const void* object,
                             TelemetryCollector& collector){
    auto ctrl = static_cast<const probot::control::IMotorController*>(object);
    if (!ctrl) return;
    auto cfg = ctrl->motionProfileConfig();
    collector.addFloat("mp_max_velocity", cfg.maxVelocity);
    collector.addFloat("mp_max_acceleration", cfg.maxAcceleration);
    collector.addFloat("mp_max_jerk", cfg.maxJerk);
  }

  void sliderDynamic(const void* object,
                     TelemetryCollector& collector,
                     uint32_t now_ms,
                     uint32_t dt_ms){
    (void)now_ms; (void)dt_ms;
    auto slider = static_cast<const probot::mechanism::Slider*>(object);
    if (!slider) return;
    float target = slider->getTargetLength();
    float current = slider->getCurrentLength();
    collector.addFloat("target_length", target);
    collector.addFloat("current_length", current);
    collector.addFloat("error_length", current - target);
  }

  void armDynamic(const void* object,
                  TelemetryCollector& collector,
                  uint32_t now_ms,
                  uint32_t dt_ms){
    (void)now_ms; (void)dt_ms;
    auto arm = static_cast<const probot::mechanism::Arm*>(object);
    if (!arm) return;
    float target = arm->getTargetAngleDeg();
    float current = arm->getCurrentAngleDeg();
    collector.addFloat("target_angle_deg", target);
    collector.addFloat("current_angle_deg", current);
    collector.addFloat("error_angle_deg", current - target);
  }

  void elevatorDynamic(const void* object,
                       TelemetryCollector& collector,
                       uint32_t now_ms,
                       uint32_t dt_ms){
    (void)now_ms; (void)dt_ms;
    auto elevator = static_cast<const probot::mechanism::Elevator*>(object);
    if (!elevator) return;
    float target = elevator->getTargetHeight();
    float current = elevator->getCurrentHeight();
    collector.addFloat("target_height", target);
    collector.addFloat("current_height", current);
    collector.addFloat("error_height", current - target);
  }

  void telescopicDynamic(const void* object,
                         TelemetryCollector& collector,
                         uint32_t now_ms,
                         uint32_t dt_ms){
    (void)now_ms; (void)dt_ms;
    auto tube = static_cast<const probot::mechanism::TelescopicTube*>(object);
    if (!tube) return;
    float target = tube->getTargetExtension();
    float current = tube->getCurrentExtension();
    collector.addFloat("target_extension", target);
    collector.addFloat("current_extension", current);
    collector.addFloat("error_extension", current - target);
  }

  void turretDynamic(const void* object,
                     TelemetryCollector& collector,
                     uint32_t now_ms,
                     uint32_t dt_ms){
    (void)now_ms; (void)dt_ms;
    auto turret = static_cast<const probot::mechanism::Turret*>(object);
    if (!turret) return;
    float target = turret->getTargetAngleDeg();
    float current = turret->getCurrentAngleDeg();
    collector.addFloat("target_angle_deg", target);
    collector.addFloat("current_angle_deg", current);
    collector.addFloat("error_angle_deg", current - target);
  }

  void tankDriveDynamic(const void* object,
                        TelemetryCollector& collector,
                        uint32_t now_ms,
                        uint32_t dt_ms){
    (void)now_ms; (void)dt_ms;
    auto drive = static_cast<const probot::drive::BasicTankDrive*>(object);
    if (!drive) return;
    collector.addBool("position_mode", drive->positionActive());
    collector.addBool("position_goal_reached",
                      drive->positionGoalReached(0.5f));
  }

  void blinkPidDynamic(const void* object,
                       TelemetryCollector& collector,
                       uint32_t now_ms,
                       uint32_t dt_ms){
    (void)now_ms; (void)dt_ms;
    auto pid = static_cast<const probot::control::BlinkPid*>(object);
    if (!pid) return;
    collector.addInt("reference", static_cast<int32_t>(pid->currentReference()), Priority::kUserMarked);
    collector.addBool("led_state", pid->ledState());
  }

} // namespace probot::logging::profiles
