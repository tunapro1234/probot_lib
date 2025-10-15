#include "test_harness.hpp"

#include <algorithm>
#include <cmath>

#include <probot/control/motion_profile/s_curve_profile.hpp>
#include <probot/control/motion_profile/trapezoid_profile.hpp>

TEST_CASE(trapezoid_profile_basic){
  probot::control::motion_profile::TrapezoidProfile::Constraints cons(2.0f, 4.0f);
  probot::control::motion_profile::TrapezoidProfile::State goal(3.0f, 0.0f);
  probot::control::motion_profile::TrapezoidProfile profile(cons, goal);

  EXPECT_TRUE(profile.totalTime() > 0.0f);
  auto start = profile.calculate(0.0f);
  EXPECT_NEAR(start.position, 0.0f, 1e-5f);
  EXPECT_NEAR(start.velocity, 0.0f, 1e-5f);

  auto end = profile.calculate(profile.totalTime());
  EXPECT_NEAR(end.position, goal.position, 1e-3f);
  EXPECT_NEAR(end.velocity, goal.velocity, 1e-3f);

  auto mid = profile.calculate(profile.totalTime() * 0.5f);
  EXPECT_TRUE(std::fabs(mid.velocity) <= cons.maxVelocity + 1e-3f);
}

TEST_CASE(scurve_profile_basic){
  probot::control::motion_profile::SCurveProfile::Constraints cons(2.5f, 3.0f, 10.0f);
  probot::control::motion_profile::SCurveProfile::State goal(2.0f, 0.0f, 0.0f);
  probot::control::motion_profile::SCurveProfile profile(cons, goal);

  EXPECT_TRUE(profile.totalTime() > 0.0f);
  auto start = profile.calculate(0.0f);
  EXPECT_NEAR(start.position, 0.0f, 1e-5f);
  EXPECT_NEAR(start.velocity, 0.0f, 1e-5f);

  auto end = profile.calculate(profile.totalTime());
  EXPECT_NEAR(end.position, goal.position, 5e-2f);
  EXPECT_NEAR(end.velocity, goal.velocity, 5e-2f);

  float maxVel = 0.0f;
  float maxAcc = 0.0f;
  for (float t = 0.0f; t <= profile.totalTime(); t += profile.timeStep()){
    auto s = profile.calculate(t);
    maxVel = std::max(maxVel, std::fabs(s.velocity));
    maxAcc = std::max(maxAcc, std::fabs(s.acceleration));
  }
  EXPECT_TRUE(maxVel <= cons.maxVelocity + 1e-2f);
  EXPECT_TRUE(maxAcc <= cons.maxAcceleration + 1e-2f);
}
