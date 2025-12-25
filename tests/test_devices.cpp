#include "test_harness.hpp"

#include <probot/devices/motors/motor_controller_group.hpp>
#include <probot/test/test_motor.hpp>

#include <probot/devices/motors/boardoza_vnh5019_motor_driver.hpp>
#include <type_traits>

namespace {
  struct MotorStub : probot::motor::IMotorController {
    float lastPower = 0.0f;
    float lastCommand = 0.0f;
    bool inverted = false;
    bool setPower(float power) override { lastCommand = power; lastPower = inverted ? -power : power; return true; }
    void setInverted(bool inv) override { inverted = inv; }
    bool getInverted() const override { return inverted; }
  };
}

TEST_CASE(motor_group_power_and_invert){
  MotorStub a, b;
  probot::motor::MotorControllerGroup group(&a, &b);
  EXPECT_TRUE(group.setPower(0.3f));
  EXPECT_NEAR(a.lastPower, 0.3f, 1e-5f);
  EXPECT_NEAR(b.lastPower, 0.3f, 1e-5f);

  group.setInverted(true);
  EXPECT_TRUE(group.setPower(0.4f));
  EXPECT_NEAR(a.lastPower, -0.4f, 1e-5f);
  EXPECT_NEAR(a.lastCommand, 0.4f, 1e-5f);
}

TEST_CASE(null_motor_behaves_like_driver){
  probot::motor::NullMotor motor;
  EXPECT_TRUE(motor.setPower(0.5f));
  EXPECT_NEAR(motor.appliedPower(), 0.5f, 1e-5f);
  motor.setInverted(true);
  EXPECT_TRUE(motor.getInverted());
  EXPECT_TRUE(motor.setPower(0.2f));
  EXPECT_NEAR(motor.appliedPower(), -0.2f, 1e-5f);
}

static_assert(std::is_base_of<probot::motor::IMotorController, probot::motor::BoardozaVNH5019MotorDriver>::value,
              "BoardozaVNH5019MotorDriver must implement IMotorController");
