#include "test_harness.hpp"

#include <Arduino.h>
#include <cmath>

#include <probot/control/pid_motor_controller.hpp>
#include <probot/control/pid_motor_controller_group.hpp>
#include <probot/sensors/encoder.hpp>
#include <probot/devices/motors/imotor_controller.hpp>

namespace {
  struct EncoderStub : probot::sensors::IEncoder {
    int32_t ticks = 0;
    int32_t tps = 0;
    int32_t readTicks() override { return ticks; }
    int32_t readTicksPerSecond() override { return tps; }
  };

  struct MotorStub : probot::motor::IMotorController {
    float lastPower = 0.0f;
    bool inverted = false;

    bool setPower(float power) override {
      lastPower = inverted ? -power : power;
      return true;
    }
    void setInverted(bool inv) override { inverted = inv; }
    bool getInverted() const override { return inverted; }
  };

}

extern unsigned long _test_millis_now;

static probot::control::PidConfig makePid(float kp){
  probot::control::PidConfig cfg{};
  cfg.kp = kp;
  cfg.ki = 0.0f;
  cfg.kd = 0.0f;
  cfg.out_min = -1.0f;
  cfg.out_max = 1.0f;
  return cfg;
}

TEST_CASE(pid_motor_controller_velocity_control){
  EncoderStub encoder;
  MotorStub motor;
  auto cfg = makePid(0.2f);
  probot::control::PidMotorController controller(&encoder, &motor, 1.0f, 1.0f);
  controller.setTimeoutMs(0);
  controller.setVelocityPidConfig(cfg);

  controller.setVelocity(10.0f);
  encoder.tps = 5;
  controller.update(0, 20);
  EXPECT_TRUE(motor.lastPower > 0.0f);

  encoder.tps = 12;
  controller.update(0, 20);
  EXPECT_TRUE(motor.lastPower < 0.0f);
  EXPECT_TRUE(controller.isAtTarget(5.0f));
}

TEST_CASE(pid_motor_controller_group_broadcast){
  EncoderStub encA, encB;
  MotorStub driverA, driverB;
  auto cfg = makePid(0.2f);
  probot::control::PidMotorController a(&encA, &driverA, 1.0f, 1.0f);
  probot::control::PidMotorController b(&encB, &driverB, 1.0f, 1.0f);
  a.setPositionPidConfig(cfg);
  b.setPositionPidConfig(cfg);
  a.setTimeoutMs(0);
  b.setTimeoutMs(0);
  probot::control::PidMotorControllerGroup group(&a, &b);

  group.setPosition(15.0f);
  EXPECT_NEAR(a.lastSetpoint(), 15.0f, 1e-5f);
  EXPECT_NEAR(b.lastSetpoint(), 15.0f, 1e-5f);
  EXPECT_TRUE(a.activeMode() == probot::control::ControlType::kPosition);
  EXPECT_TRUE(b.activeMode() == probot::control::ControlType::kPosition);

  EXPECT_TRUE(group.setPower(0.5f));
  EXPECT_NEAR(driverA.lastPower, 0.5f, 1e-5f);
  EXPECT_NEAR(driverB.lastPower, 0.5f, 1e-5f);

  group.setInverted(true);
  EXPECT_TRUE(group.getInverted());
  EXPECT_TRUE(group.setPower(0.5f));
  EXPECT_NEAR(driverA.lastPower, -0.5f, 1e-5f);
}

TEST_CASE(pid_motor_controller_percent_and_timeout){
  EncoderStub encoder;
  MotorStub driver;
  auto cfg = makePid(0.1f);
  probot::control::PidMotorController controller(&encoder, &driver, 1.0f, 0.001f);
  controller.setTimeoutMs(50);
  controller.setVelocityPidConfig(cfg);
  _test_millis_now = 0;

  controller.setPower(0.5f);
  controller.update(0, 10);
  EXPECT_NEAR(driver.lastPower, 0.5f, 1e-5f);

  controller.setVelocity(5.0f);
  encoder.tps = 0;
  controller.update(20, 10);
  EXPECT_TRUE(driver.lastPower > 0.0f);

  _test_millis_now = 100;
  controller.update(150, 10);
  EXPECT_NEAR(driver.lastPower, 0.0f, 1e-5f);
}

TEST_CASE(pid_motor_controller_position_control){
  EncoderStub encoder;
  MotorStub driver;
  probot::control::PidConfig cfg{};
  cfg.kp = 0.3f; cfg.ki = 0.0f; cfg.kd = 0.0f; cfg.kf = 0.0f; cfg.out_min = -1.0f; cfg.out_max = 1.0f;
  probot::control::PidMotorController controller(&encoder, &driver, 1.0f, 1.0f);
  controller.setTimeoutMs(0);
  controller.setPositionPidConfig(cfg);

  controller.setPosition(5.0f);
  encoder.ticks = 0;
  controller.update(0, 20);
  EXPECT_TRUE(driver.lastPower > 0.0f);

  encoder.ticks = 10;
  controller.update(20, 20);
  EXPECT_TRUE(driver.lastPower < 0.0f);
}
