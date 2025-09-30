#include "test_harness.hpp"

#include <cmath>

#include <probot/control/feedforward/arm_ff.hpp>
#include <probot/control/feedforward/elevator_ff.hpp>
#include <probot/control/feedforward/simple_motor_ff.hpp>

TEST_CASE(feedforward_simple_motor){
  probot::control::feedforward::SimpleMotorFF ff(0.5f, 1.2f, 0.3f);
  float out = ff.calculate(2.0f, 0.5f);
  EXPECT_NEAR(out, 0.5f + 1.2f * 2.0f + 0.3f * 0.5f, 1e-5f);

  ff.setKs(0.2f);
  ff.setKv(0.8f);
  ff.setKa(0.1f);
  out = ff.calculate(-1.0f, -0.3f);
  EXPECT_NEAR(out, -0.2f + 0.8f * -1.0f + 0.1f * -0.3f, 1e-5f);
}

TEST_CASE(feedforward_arm){
  probot::control::feedforward::ArmFF ff(0.2f, 0.3f, 0.05f, 0.4f);
  float theta = 0.25f;
  float out = ff.calculate(theta, 1.0f, -0.2f);
  EXPECT_NEAR(out, 0.2f + 0.4f * std::cos(theta) + 0.3f * 1.0f + 0.05f * -0.2f, 1e-5f);
}

TEST_CASE(feedforward_elevator){
  probot::control::feedforward::ElevatorFF ff(0.1f, 0.4f, 0.05f, 0.5f);
  float out = ff.calculate(0.5f, -0.3f);
  EXPECT_NEAR(out, 0.1f + 0.5f + 0.4f * 0.5f + 0.05f * -0.3f, 1e-5f);
}
