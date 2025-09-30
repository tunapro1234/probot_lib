#include "test_harness.hpp"

#include <cmath>

#include <probot/mechanism/arm.hpp>
#include <probot/mechanism/elevator.hpp>
#include <probot/mechanism/slider.hpp>
#include <probot/mechanism/telescopic_tube.hpp>
#include <probot/mechanism/turret.hpp>
#include <probot/control/imotor_controller.hpp>

namespace {
  struct ControllerMock : probot::control::IMotorController {
    float setpoint = 0.0f;
    probot::control::ControlType mode = probot::control::ControlType::kVelocity;
    int slot = -1;
    float measurement = 0.0f;
    probot::control::MotionProfileType profileType = probot::control::MotionProfileType::kNone;
    probot::control::MotionProfileConfig profileCfg{};

    bool claim(void*) override { return true; }
    void release(void*) override {}
    bool setPower(float, void*) override { return true; }
    bool isClaimed() const override { return false; }
    void* currentOwner() const override { return nullptr; }
    void setInverted(bool) override {}
    bool getInverted() const override { return false; }

    void setSetpoint(float value, probot::control::ControlType m, int s) override { setpoint = value; mode = m; slot = s; }
    void setTimeoutMs(uint32_t) override {}
    void setPidSlotConfig(int, const probot::control::PidConfig&) override {}
    void selectDefaultSlot(probot::control::ControlType, int) override {}
    int defaultSlot(probot::control::ControlType) const override { return 0; }
    float lastSetpoint() const override { return setpoint; }
    float lastMeasurement() const override { return measurement; }
    float lastOutput() const override { return 0.0f; }
    probot::control::ControlType activeMode() const override { return mode; }
    bool isAtTarget(float tol) const override { return std::fabs(setpoint - measurement) <= tol; }
    void setMotionProfile(probot::control::MotionProfileType type) override { profileType = type; }
    probot::control::MotionProfileType motionProfile() const override { return profileType; }
    void setMotionProfileConfig(const probot::control::MotionProfileConfig& cfg) override { profileCfg = cfg; }
    probot::control::MotionProfileConfig motionProfileConfig() const override { return profileCfg; }
    void update(uint32_t, uint32_t) override {}
  };
}

TEST_CASE(slider_limits_and_conversion){
  ControllerMock mock;
  probot::mechanism::Slider slider(&mock);
  slider.setLengthToTicks(48.0f);
  slider.setLengthLimits(0.0f, 50.0f);
  slider.setTargetLength(60.0f);
  EXPECT_NEAR(slider.getTargetLength(), 50.0f, 1e-5f);

  slider.update(0, 0);
  EXPECT_NEAR(mock.setpoint, 50.0f * 48.0f, 1e-5f);
}

TEST_CASE(elevator_limits_and_conversion){
  ControllerMock mock;
  probot::mechanism::Elevator elevator(&mock);
  elevator.setUnitsToTicks(60.0f);
  elevator.setHeightLimits(0.0f, 120.0f);
  elevator.setTargetHeight(150.0f);
  EXPECT_NEAR(elevator.getTargetHeight(), 120.0f, 1e-5f);
  elevator.update(0, 0);
  EXPECT_NEAR(mock.setpoint, 120.0f * 60.0f, 1e-5f);
}

TEST_CASE(turret_angle_limits){
  ControllerMock mock;
  probot::mechanism::Turret turret(&mock);
  turret.setDegreesToTicks(5.0f);
  turret.setAngleLimits(-90.0f, 90.0f);
  turret.setTargetAngleDeg(120.0f);
  EXPECT_NEAR(turret.getTargetAngleDeg(), 90.0f, 1e-5f);
  turret.update(0, 0);
  EXPECT_NEAR(mock.setpoint, 90.0f * 5.0f, 1e-5f);
}

TEST_CASE(arm_angle_limits){
  ControllerMock mock;
  probot::mechanism::Arm arm(&mock);
  arm.setDegreesToTicks(6.0f);
  arm.setAngleLimits(-120.0f, 120.0f);
  arm.setTargetAngleDeg(-150.0f);
  EXPECT_NEAR(arm.getTargetAngleDeg(), -120.0f, 1e-5f);
  arm.update(0, 0);
  EXPECT_NEAR(mock.setpoint, -120.0f * 6.0f, 1e-5f);
}

TEST_CASE(telescopic_stage_limits){
  ControllerMock mock;
  probot::mechanism::TelescopicTube tube(&mock);
  tube.setUnitsToTicks(80.0f);
  tube.setStageConfiguration(3, 40.0f);
  tube.setTargetExtension(200.0f);
  EXPECT_NEAR(tube.getTargetExtension(), 120.0f, 1e-5f);
  tube.update(0, 0);
  EXPECT_NEAR(mock.setpoint, 120.0f * 80.0f, 1e-5f);
}
