#include "test_harness.hpp"

#include <probot/devices/motors/motor_group.hpp>
#include <probot/devices/motors/motor_handle.hpp>
#include <probot/test/test_motor.hpp>

namespace {
  struct MotorStub : probot::motor::IMotorDriver {
    void* owner = nullptr;
    float lastPower = 0.0f;
    float lastCommand = 0.0f;
    bool inverted = false;
    bool claim(void* o) override { if (owner && owner != o) return false; owner = o; return true; }
    void release(void* o) override { if (owner == o){ owner = nullptr; lastPower = 0.0f; } }
    bool setPower(float power, void* o) override { if (owner != o) return false; lastCommand = power; lastPower = inverted ? -power : power; return true; }
    bool isClaimed() const override { return owner != nullptr; }
    void* currentOwner() const override { return owner; }
    void setInverted(bool inv) override { inverted = inv; }
    bool getInverted() const override { return inverted; }
  };
}

TEST_CASE(motor_group_claim_and_invert){
  MotorStub a, b;
  probot::motor::MotorGroup group(&a, &b);
  int token = 42;
  EXPECT_TRUE(group.claim(&token));
  EXPECT_TRUE(group.setPower(0.3f, &token));
  EXPECT_NEAR(a.lastPower, 0.3f, 1e-5f);
  EXPECT_NEAR(b.lastPower, 0.3f, 1e-5f);

  group.setInverted(true);
  EXPECT_TRUE(group.setPower(0.4f, &token));
  EXPECT_NEAR(a.lastCommand, -0.4f, 1e-5f);
  group.release(&token);
  EXPECT_TRUE(!a.isClaimed());
}

TEST_CASE(motor_handle_claims_motor){
  probot::motor::NullMotor motor;
  {
    probot::motor::MotorHandle handle(motor);
    handle.setPower(0.5f);
    EXPECT_TRUE(motor.isClaimed());
    handle.setInverted(true);
    EXPECT_TRUE(handle.getInverted());
    handle.release();
  }
  EXPECT_TRUE(!motor.isClaimed());
}
