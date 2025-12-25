#include "test_harness.hpp"

#include <cmath>

#include <probot/mechanism/arm.hpp>
#include <probot/mechanism/elevator.hpp>
#include <probot/mechanism/slider.hpp>
#include <probot/mechanism/telescopic_tube.hpp>
#include <probot/mechanism/turret.hpp>
#include <probot/control/pid_motor_controller.hpp>
#include <probot/test/test_encoder.hpp>
#include <probot/test/test_motor.hpp>

namespace {
  struct PidHarness {
    probot::test::TestEncoder encoder;
    probot::test::TestMotor driver;
    probot::control::PidMotorController motor;

    PidHarness()
    : motor(&encoder, &driver, 1.0f, 1.0f) {
      motor.setTimeoutMs(0);
    }
  };
}

TEST_CASE(slider_limits_and_conversion){
  PidHarness harness;
  probot::mechanism::Slider slider(&harness.motor);
  slider.setLengthToTicks(48.0f);
  slider.setLengthLimits(0.0f, 50.0f);
  slider.setTargetLength(60.0f);
  EXPECT_NEAR(slider.getTargetLength(), 50.0f, 1e-5f);

  slider.update(0, 0);
  EXPECT_NEAR(harness.motor.lastSetpoint(), 50.0f * 48.0f, 1e-5f);
}

TEST_CASE(elevator_limits_and_conversion){
  PidHarness harness;
  probot::mechanism::Elevator elevator(&harness.motor);
  elevator.setUnitsToTicks(60.0f);
  elevator.setHeightLimits(0.0f, 120.0f);
  elevator.setTargetHeight(150.0f);
  EXPECT_NEAR(elevator.getTargetHeight(), 120.0f, 1e-5f);
  elevator.update(0, 0);
  EXPECT_NEAR(harness.motor.lastSetpoint(), 120.0f * 60.0f, 1e-5f);
}

TEST_CASE(turret_angle_limits){
  PidHarness harness;
  probot::mechanism::Turret turret(&harness.motor);
  turret.setDegreesToTicks(5.0f);
  turret.setAngleLimits(-90.0f, 90.0f);
  turret.setPositionPidConfig({0.0f,0.0f,0.0f,0.0f,-1.0f,1.0f});
  turret.setTargetAngleDeg(120.0f);
  EXPECT_NEAR(turret.getTargetAngleDeg(), 90.0f, 1e-5f);
  turret.update(0, 0);
  EXPECT_NEAR(harness.motor.lastSetpoint(), 90.0f * 5.0f, 1e-5f);
}

TEST_CASE(turret_slew_limit){
  PidHarness harness;
  probot::mechanism::Turret turret(&harness.motor);
  turret.setDegreesToTicks(10.0f);
  turret.setSlewRateLimit(60.0f); // deg/s
  turret.setTargetAngleDeg(90.0f);

  turret.update(0, 20); // 20 ms -> 0.02 s
  float expectedDegrees = 60.0f * 0.02f; // slew limited
  EXPECT_NEAR(harness.motor.lastSetpoint(), expectedDegrees * 10.0f, 1e-3f);

  turret.update(40, 20);
  EXPECT_TRUE(harness.motor.lastSetpoint() >= expectedDegrees * 10.0f);
}

TEST_CASE(arm_angle_limits){
  PidHarness harness;
  probot::mechanism::Arm arm(&harness.motor);
  arm.setDegreesToTicks(6.0f);
  arm.setAngleLimits(-120.0f, 120.0f);
  arm.setTargetAngleDeg(-150.0f);
  EXPECT_NEAR(arm.getTargetAngleDeg(), -120.0f, 1e-5f);
  arm.update(0, 0);
  EXPECT_NEAR(harness.motor.lastSetpoint(), -120.0f * 6.0f, 1e-5f);
}

TEST_CASE(telescopic_stage_limits){
  PidHarness harness;
  probot::mechanism::TelescopicTube tube(&harness.motor);
  tube.setUnitsToTicks(80.0f);
  tube.setStageConfiguration(3, 40.0f);
  tube.setTargetExtension(200.0f);
  EXPECT_NEAR(tube.getTargetExtension(), 120.0f, 1e-5f);
  tube.update(0, 0);
  EXPECT_NEAR(harness.motor.lastSetpoint(), 120.0f * 80.0f, 1e-5f);
}
