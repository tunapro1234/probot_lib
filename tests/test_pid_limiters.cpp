#include "test_harness.hpp"

#include <Arduino.h>
#include <cmath>

#include <probot/control/bang_bang_controller.hpp>
#include <probot/control/blink_pid.hpp>
#include <probot/control/limiters/slew_rate_limiter.hpp>
#include <probot/control/pid.hpp>

namespace probot::builtinled {
  bool test_last_on();
  void test_reset();
}

TEST_CASE(pid_basic_response){
  probot::control::PidConfig cfg{0.3f, 0.1f, 0.05f, 0.0f, -1.0f, 1.0f};
  probot::control::PID pid(cfg);

  float out1 = pid.step(1.0f, 0.02f);
  EXPECT_TRUE(out1 > 0.0f);

  float out2 = pid.step(0.0f, 0.02f);
  EXPECT_TRUE(out2 < out1);

  pid.reset();
  pid.enableContinuousInput(-3.14f, 3.14f);
  float cont = pid.stepFrom(3.0f, -3.0f, 0.02f); // wrap-around error ~0.28
  EXPECT_TRUE(std::fabs(cont) <= cfg.out_max + 1e-5f);
}

TEST_CASE(pid_saturation_clamp){
  probot::control::PidConfig cfg{5.0f, 3.0f, 0.0f, 0.0f, -0.5f, 0.5f};
  probot::control::PID pid(cfg);
  float out = pid.step(10.0f, 0.05f);
  EXPECT_NEAR(out, 0.5f, 1e-5f);
  out = pid.step(-10.0f, 0.05f);
  EXPECT_NEAR(out, -0.5f, 1e-5f);
}

TEST_CASE(slew_rate_limiter){
  probot::control::limiters::SlewRateLimiter limiter(2.0f, 0.0f);
  float value = limiter.calculate(1.0f, 0.1f);
  EXPECT_NEAR(value, 0.2f, 1e-5f);
  value = limiter.calculate(-1.0f, 0.2f);
  EXPECT_NEAR(value, -0.2f, 1e-5f);
  limiter.reset(0.5f);
  value = limiter.calculate(0.7f, 0.0f);
  EXPECT_NEAR(value, 0.7f, 1e-5f);
}

TEST_CASE(bang_bang_controller){
  probot::control::BangBangController ctrl(0.1f);
  EXPECT_NEAR(ctrl.calculate(0.0f, 1.0f), 1.0f, 1e-5f);
  EXPECT_NEAR(ctrl.calculate(1.0f, 0.0f), -1.0f, 1e-5f);
  EXPECT_NEAR(ctrl.calculate(0.05f, 0.0f), 0.0f, 1e-5f);
  EXPECT_TRUE(ctrl.atSetpoint(0.05f, 0.0f));
  ctrl.setTolerance(0.2f);
  EXPECT_NEAR(ctrl.tolerance(), 0.2f, 1e-5f);
}

TEST_CASE(blink_pid_led_toggles){
  probot::builtinled::test_reset();
  probot::control::BlinkPid blink;

  EXPECT_TRUE(!probot::builtinled::test_last_on());
  blink.setReference(1);
  blink.update(0, 0);
  EXPECT_TRUE(probot::builtinled::test_last_on());

  blink.setReference(2);
  blink.update(0, 0);
  EXPECT_TRUE(!probot::builtinled::test_last_on());
}
