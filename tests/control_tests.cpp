#include <cassert>
#include <cmath>
#include <iostream>

#include <probot/control/feedforward/simple_motor_ff.hpp>
#include <probot/control/feedforward/arm_ff.hpp>
#include <probot/control/feedforward/elevator_ff.hpp>
#include <probot/control/motion_profile/trapezoid_profile.hpp>
#include <probot/control/motion_profile/s_curve_profile.hpp>

using probot::control::feedforward::SimpleMotorFF;
using probot::control::feedforward::ArmFF;
using probot::control::feedforward::ElevatorFF;
using probot::control::motion_profile::TrapezoidProfile;
using probot::control::motion_profile::SCurveProfile;

static void testSimpleMotorFF(){
  SimpleMotorFF ff(0.5f, 1.2f, 0.3f);
  float output = ff.calculate(2.0f, 0.5f);
  float expected = 0.5f + 1.2f * 2.0f + 0.3f * 0.5f;
  assert(std::fabs(output - expected) < 1e-5f);

  ff.setKs(0.6f); ff.setKv(1.0f); ff.setKa(0.2f);
  output = ff.calculate(-1.5f, -0.5f);
  expected = -0.6f + 1.0f * -1.5f + 0.2f * -0.5f;
  assert(std::fabs(output - expected) < 1e-5f);
}

static void testArmFF(){
  ArmFF ff(0.2f, 0.3f, 0.1f, 0.5f);
  float theta = static_cast<float>(M_PI) / 6.0f;
  float velocity = 1.2f;
  float acceleration = -0.4f;
  float output = ff.calculate(theta, velocity, acceleration);
  float expected = 0.2f + 0.5f * std::cos(theta) + 0.3f * velocity + 0.1f * acceleration;
  assert(std::fabs(output - expected) < 1e-5f);
}

static void testElevatorFF(){
  ElevatorFF ff(0.15f, 0.45f, 0.05f, 0.4f);
  float output = ff.calculate(0.8f, -0.2f);
  float expected = 0.15f + 0.4f + 0.45f * 0.8f + 0.05f * -0.2f;
  assert(std::fabs(output - expected) < 1e-5f);
}

static void testTrapezoidProfile(){
  TrapezoidProfile::Constraints cons(2.0f, 4.0f);
  TrapezoidProfile::State goal(3.0f, 0.0f);
  TrapezoidProfile profile(cons, goal);

  assert(profile.totalTime() > 0.0f);

  auto start = profile.calculate(0.0f);
  assert(std::fabs(start.position - 0.0f) < 1e-5f);
  assert(std::fabs(start.velocity - 0.0f) < 1e-5f);

  auto end = profile.calculate(profile.totalTime());
  assert(std::fabs(end.position - goal.position) < 1e-3f);
  assert(std::fabs(end.velocity - goal.velocity) < 1e-3f);

  // Check mid point not exceeding velocity constraints
  auto mid = profile.calculate(profile.totalTime() * 0.5f);
  assert(std::fabs(mid.velocity) <= cons.maxVelocity + 1e-3f);
}

static void testSCurveProfile(){
  SCurveProfile::Constraints cons(2.5f, 3.0f, 10.0f);
  SCurveProfile::State goal(2.0f, 0.0f, 0.0f);
  SCurveProfile profile(cons, goal);

  assert(profile.totalTime() > 0.0f);

  auto start = profile.calculate(0.0f);
  assert(std::fabs(start.position) < 1e-5f);
  assert(std::fabs(start.velocity) < 1e-5f);

  auto end = profile.calculate(profile.totalTime());
  assert(std::fabs(end.position - goal.position) < 5e-2f);
  assert(std::fabs(end.velocity - goal.velocity) < 5e-2f);

  // Acceleration and velocity should remain within bounds
  float maxVelObserved = 0.0f;
  float maxAccObserved = 0.0f;
  for (float t = 0.0f; t <= profile.totalTime(); t += profile.timeStep()){
    auto s = profile.calculate(t);
    maxVelObserved = std::max(maxVelObserved, std::fabs(s.velocity));
    maxAccObserved = std::max(maxAccObserved, std::fabs(s.acceleration));
  }
  assert(maxVelObserved <= cons.maxVelocity + 1e-2f);
  assert(maxAccObserved <= cons.maxAcceleration + 1e-2f);
}

int main(){
  testSimpleMotorFF();
  testArmFF();
  testElevatorFF();
  testTrapezoidProfile();
  testSCurveProfile();
  std::cout << "All control tests passed\n";
  return 0;
}
