#include "test_harness.hpp"

#include <Arduino.h>
#include <cmath>

#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/closed_loop_motor_group.hpp>
#include <probot/control/pid.hpp>
#include <probot/control/imotor_controller.hpp>
#include <probot/sensors/encoder.hpp>
#include <probot/devices/motors/imotor_driver.hpp>

namespace {
  struct EncoderStub : probot::sensors::IEncoder {
    int32_t ticks = 0;
    int32_t tps = 0;
    int32_t readTicks() override { return ticks; }
    int32_t readTicksPerSecond() override { return tps; }
  };

  struct MotorStub : probot::motor::IMotorDriver {
    float lastPower = 0.0f;
    bool inverted = false;

    bool setPower(float power) override {
      lastPower = inverted ? -power : power;
      return true;
    }
    void setInverted(bool inv) override { inverted = inv; }
    bool getInverted() const override { return inverted; }
  };

  struct ControllerStub : probot::control::IMotorController {
    float setpoint = 0.0f;
    probot::control::ControlType mode = probot::control::ControlType::kVelocity;
    int lastSlot = -1;
    float measurement = 0.0f;
    float output = 0.0f;
    float lastCommand = 0.0f;
    bool inverted = false;
    probot::control::MotionProfileType profileType = probot::control::MotionProfileType::kNone;
    probot::control::MotionProfileConfig profileCfg{};

    bool setPower(float power) override {
      lastCommand = power;
      output = inverted ? -power : power;
      return true;
    }
    void setInverted(bool inv) override { inverted = inv; }
    bool getInverted() const override { return inverted; }

    void setSetpoint(float value, probot::control::ControlType m, int slot) override {
      setpoint = value;
      mode = m;
      lastSlot = slot;
    }
    void setTimeoutMs(uint32_t) override {}
    void setPidSlotConfig(int, const probot::control::PidConfig&) override {}
    void selectDefaultSlot(probot::control::ControlType, int) override {}
    int defaultSlot(probot::control::ControlType) const override { return 0; }
    float lastSetpoint() const override { return setpoint; }
    float lastMeasurement() const override { return measurement; }
    float lastOutput() const override { return output; }
    probot::control::ControlType activeMode() const override { return mode; }
    bool isAtTarget(float tolerance) const override { return std::fabs(setpoint - measurement) <= tolerance; }
    void setMotionProfile(probot::control::MotionProfileType type) override { profileType = type; }
    probot::control::MotionProfileType motionProfile() const override { return profileType; }
    void setMotionProfileConfig(const probot::control::MotionProfileConfig& cfg) override { profileCfg = cfg; }
    probot::control::MotionProfileConfig motionProfileConfig() const override { return profileCfg; }
    void update(uint32_t, uint32_t) override {}
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

TEST_CASE(closed_loop_motor_velocity_control){
  EncoderStub encoder;
  MotorStub motor;
  auto cfg = makePid(0.2f);
  probot::control::PID pid(cfg);
  probot::control::ClosedLoopMotor controller(&encoder, &pid, &motor, 1.0f, 1.0f);
  controller.setTimeoutMs(0);
  controller.configurePidSlots(0, cfg, 1, cfg);

  controller.setSetpoint(10.0f, probot::control::ControlType::kVelocity);
  encoder.tps = 5;
  controller.update(0, 20);
  EXPECT_TRUE(motor.lastPower > 0.0f);

  encoder.tps = 12;
  controller.update(0, 20);
  EXPECT_TRUE(motor.lastPower < 0.0f);
  EXPECT_TRUE(controller.isAtTarget(5.0f));
}

TEST_CASE(closed_loop_motor_group_broadcast){
  ControllerStub a, b;
  probot::control::ClosedLoopMotorGroup group(&a, &b);

  group.setSetpoint(15.0f, probot::control::ControlType::kPosition, 2);
  EXPECT_NEAR(a.setpoint, 15.0f, 1e-5f);
  EXPECT_NEAR(b.setpoint, 15.0f, 1e-5f);
  EXPECT_TRUE(a.mode == probot::control::ControlType::kPosition);
  EXPECT_TRUE(b.mode == probot::control::ControlType::kPosition);

  EXPECT_TRUE(group.setPower(0.5f));
  EXPECT_NEAR(a.output, 0.5f, 1e-5f);
  EXPECT_NEAR(b.output, 0.5f, 1e-5f);
  EXPECT_NEAR(a.lastCommand, 0.5f, 1e-5f);

  group.setInverted(true);
  EXPECT_TRUE(group.getInverted());
  EXPECT_TRUE(group.setPower(0.5f));
  EXPECT_NEAR(a.output, -0.5f, 1e-5f);
  EXPECT_NEAR(a.lastCommand, 0.5f, 1e-5f);
}

TEST_CASE(closed_loop_motor_percent_and_timeout){
  EncoderStub encoder;
  MotorStub driver;
  auto cfg = makePid(0.1f);
  probot::control::PID pid(cfg);
  probot::control::ClosedLoopMotor controller(&encoder, &pid, &driver, 1.0f, 0.001f);
  controller.setTimeoutMs(50);
  controller.setPidSlotConfig(0, cfg);
  _test_millis_now = 0;

  controller.setSetpoint(0.5f, probot::control::ControlType::kPercent);
  controller.update(0, 10);
  EXPECT_NEAR(driver.lastPower, 0.5f, 1e-5f);

  controller.setSetpoint(5.0f, probot::control::ControlType::kVelocity);
  encoder.tps = 0;
  controller.update(20, 10);
  EXPECT_TRUE(driver.lastPower > 0.0f);

  _test_millis_now = 100;
  controller.update(150, 10);
  EXPECT_NEAR(driver.lastPower, 0.0f, 1e-5f);
}

TEST_CASE(closed_loop_motor_trapezoid_profile_ramps){
  EncoderStub encoder;
  MotorStub driver;
  probot::control::PidConfig cfg{};
  cfg.kp = 0.0f; cfg.ki = 0.0f; cfg.kd = 0.0f; cfg.kf = 1.0f; cfg.out_min = -100.0f; cfg.out_max = 100.0f;
  probot::control::PID pid(cfg);
  probot::control::ClosedLoopMotor controller(&encoder, &pid, &driver, 1.0f, 0.001f);
  controller.setTimeoutMs(0);
  controller.configurePidSlots(0, cfg, 1, cfg);
  controller.setMotionProfile(probot::control::MotionProfileType::kTrapezoid);
  controller.setMotionProfileConfig({2.0f, 4.0f, 0.0f});
  auto storedCfg = controller.motionProfileConfig();
  EXPECT_NEAR(storedCfg.maxVelocity, 2.0f, 1e-6f);
  EXPECT_NEAR(storedCfg.maxAcceleration, 4.0f, 1e-6f);
  EXPECT_TRUE(controller.motionProfile() == probot::control::MotionProfileType::kTrapezoid);

  controller.setSetpoint(4.0f, probot::control::ControlType::kPosition);

  float prev = 0.0f;
  unsigned now = 0;
  bool sawPositive = false;
  for (int i=0; i<200; ++i){
    controller.update(now, 20);
    now += 20;
    float cur = driver.lastPower;
    EXPECT_TRUE(cur >= prev - 1e-3f);
    prev = cur;
    encoder.ticks = static_cast<int32_t>(std::lround(cur * 1000.0f));
    if (cur > 0.0f) sawPositive = true;
  }
  EXPECT_TRUE(sawPositive);
  EXPECT_NEAR(driver.lastPower, 4.0f, 0.2f);
}

TEST_CASE(closed_loop_motor_velocity_profile_limits_accel){
  EncoderStub encoder;
  MotorStub driver;
  probot::control::PidConfig cfg{};
  cfg.kp = 0.0f; cfg.ki = 0.0f; cfg.kd = 0.0f; cfg.kf = 1.0f; cfg.out_min = -100.0f; cfg.out_max = 100.0f;
  probot::control::PID pid(cfg);
  probot::control::ClosedLoopMotor controller(&encoder, &pid, &driver, 1.0f, 1.0f);
  controller.setTimeoutMs(0);
  controller.setPidSlotConfig(0, cfg);
  controller.setMotionProfile(probot::control::MotionProfileType::kTrapezoid);
  controller.setMotionProfileConfig({5.0f, 2.0f, 0.0f});

  controller.setSetpoint(4.0f, probot::control::ControlType::kVelocity);
  controller.update(0, 20);
  EXPECT_NEAR(driver.lastPower, 0.04f, 1e-5f);

  unsigned now = 20;
  for (int i=0;i<300 && driver.lastPower < 3.99f; ++i){
    controller.update(now, 20);
    now += 20;
  }
  EXPECT_NEAR(driver.lastPower, 4.0f, 1e-2f);
}
