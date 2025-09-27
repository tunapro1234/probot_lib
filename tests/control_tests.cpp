#include <cassert>
#include <cmath>
#include <iostream>

#include <probot/control/feedforward/simple_motor_ff.hpp>
#include <probot/control/feedforward/arm_ff.hpp>
#include <probot/control/feedforward/elevator_ff.hpp>
#include <probot/control/limiters/slew_rate_limiter.hpp>
#include <probot/control/bang_bang_controller.hpp>
#include <probot/control/trajectory/ramsete_controller.hpp>
#include <probot/control/trajectory/holonomic_drive_controller.hpp>
#include <probot/control/geometry.hpp>
#include <probot/control/state_space/luenberger_observer.hpp>
#include <probot/control/state_space/kalman_filter.hpp>
#include <probot/control/state_space/lqr.hpp>
#include <probot/control/estimation/pose_estimator.hpp>
#include <probot/control/kinematics/differential_drive_kinematics.hpp>
#include <probot/control/odometry/differential_drive_odometry.hpp>
#include <probot/control/motion_profile/trapezoid_profile.hpp>
#include <probot/control/motion_profile/s_curve_profile.hpp>

using probot::control::feedforward::SimpleMotorFF;
using probot::control::feedforward::ArmFF;
using probot::control::feedforward::ElevatorFF;
using probot::control::limiters::SlewRateLimiter;
using probot::control::BangBangController;
using probot::control::trajectory::RamseteController;
using probot::control::trajectory::HolonomicDriveController;
using probot::control::Pose2d;
using probot::control::ChassisSpeeds;
using probot::control::state_space::Matrix;
using probot::control::state_space::LuenbergerObserver;
using probot::control::state_space::KalmanFilter;
using probot::control::state_space::computeLQR;
using probot::control::state_space::identity;
using probot::control::state_space::add;
using probot::control::state_space::subtract;
using probot::control::state_space::multiply;
using probot::control::state_space::transpose;
using probot::control::state_space::scale;
using probot::control::estimation::PoseEstimator;
using probot::control::kinematics::DifferentialDriveKinematics;
using probot::control::odometry::DifferentialDriveOdometry;
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

static void testSlewRateLimiter(){
  SlewRateLimiter limiter(2.0f, 0.0f); // 2 units/sec
  float value = limiter.calculate(1.0f, 0.1f); // max delta 0.2
  assert(std::fabs(value - 0.2f) < 1e-5f);
  value = limiter.calculate(-1.0f, 0.2f); // max delta -0.4
  assert(std::fabs(value - (-0.2f)) < 1e-5f);
  limiter.reset(0.5f);
  value = limiter.calculate(0.7f, 0.0f); // dt zero => snap to input
  assert(std::fabs(value - 0.7f) < 1e-5f);
}

static void testBangBangController(){
  BangBangController ctrl(0.1f);
  float out = ctrl.calculate(0.0f, 1.0f);
  assert(out == 1.0f);
  out = ctrl.calculate(1.0f, 0.0f);
  assert(out == -1.0f);
  out = ctrl.calculate(0.05f, 0.0f);
  assert(out == 0.0f);
  assert(ctrl.atSetpoint(0.05f, 0.0f));
  ctrl.setTolerance(0.2f);
  assert(ctrl.tolerance() == 0.2f);
}

static void testRamseteController(){
  RamseteController controller;
  Pose2d current(0.0f, 0.0f, 0.0f);
  Pose2d desired(0.0f, 0.0f, 0.0f);
  ChassisSpeeds refSpeeds(1.0f, 0.0f, 0.0f);
  auto output = controller.calculate(current, desired, refSpeeds);
  assert(std::fabs(output.vx - 1.0f) < 1e-5f);
  assert(std::fabs(output.omega) < 1e-5f);

  desired = Pose2d(1.0f, 0.0f, 0.0f);
  refSpeeds = ChassisSpeeds(1.0f, 0.0f, 0.0f);
  output = controller.calculate(current, desired, refSpeeds);
  assert(output.vx > 0.0f);
  controller.setTolerance(0.05f, 0.05f, 0.05f);
  bool atRef = controller.atReference();
  assert(!atRef);
}

static void testHolonomicDriveController(){
  probot::control::PidConfig cfg{1.0f, 0.0f, 0.0f, -5.0f, 5.0f};
  probot::control::PID xPid(cfg), yPid(cfg), thetaPid(cfg);
  HolonomicDriveController controller(&xPid, &yPid, &thetaPid);
  controller.setTolerance(0.05f, 0.05f, 0.05f);

  Pose2d current(0.0f, 0.0f, 0.0f);
  Pose2d desired(0.0f, 0.0f, 0.0f);
  ChassisSpeeds ref(0.5f, 0.0f, 0.1f);
  auto out = controller.calculate(current, desired, ref, 0.02f);
  assert(std::fabs(out.vx - ref.vx) < 1e-5f);
  assert(std::fabs(out.omega - ref.omega) < 1e-5f);

  desired = Pose2d(1.0f, -0.5f, 0.3f);
  out = controller.calculate(current, desired, ChassisSpeeds(), 0.02f);
  assert(out.vx > 0.0f);
  assert(out.vy < 0.0f);
  assert(out.omega > 0.0f);
  assert(!controller.atReference());

  controller.calculate(desired, desired, ChassisSpeeds(), 0.02f);
  assert(controller.atReference());
}

static void testLuenbergerObserver(){
  Matrix A(1,1); A(0,0) = 0.8f;
  Matrix B(1,1); B(0,0) = 1.0f;
  Matrix C(1,1); C(0,0) = 1.0f;
  Matrix L(1,1); L(0,0) = 1.0f;
  Matrix x0(1,1); x0(0,0) = 0.0f;
  LuenbergerObserver observer(A,B,C,L,x0);
  Matrix u(1,1); u(0,0) = 1.0f;
  float trueState = 0.0f;
  for (int i=0;i<20;i++){
    trueState = 0.8f * trueState + 1.0f;
    Matrix y(1,1); y(0,0) = trueState;
    observer.update(u, y);
  }
  float estimate = observer.state()(0,0);
  assert(std::fabs(estimate - trueState) < 1e-1f);
}

static void testKalmanFilter(){
  Matrix A(1,1); A(0,0)=1.0f;
  Matrix B(1,1); B(0,0)=1.0f;
  Matrix C(1,1); C(0,0)=1.0f;
  Matrix Q(1,1); Q(0,0)=0.01f;
  Matrix R(1,1); R(0,0)=0.1f;
  Matrix x0(1,1); x0(0,0)=0.0f;
  Matrix P0(1,1); P0(0,0)=1.0f;
  KalmanFilter kf(A,B,C,Q,R,x0,P0);
  Matrix u(1,1); u(0,0)=1.0f;
  float trueState=0.0f;
  const float noises[5] = {0.05f, -0.02f, 0.03f, -0.04f, 0.01f};
  for (int i=0;i<5;i++){
    trueState += 1.0f;
    Matrix y(1,1); y(0,0)=trueState + noises[i];
    kf.predict(u);
    kf.correct(y);
  }
  float estimate = kf.state()(0,0);
  assert(std::fabs(estimate - trueState) < 0.2f);
}

static void testLQR(){
  const float dt = 0.1f;
  Matrix A(2,2);
  A(0,0)=1.0f; A(0,1)=dt;
  A(1,0)=0.0f; A(1,1)=1.0f;
  Matrix B(2,1);
  B(0,0)=0.5f*dt*dt;
  B(1,0)=dt;
  Matrix Q = identity(2);
  Matrix R(1,1); R(0,0)=1.0f;
  Matrix K = computeLQR(A,B,Q,R,200,1e-5f);
  assert(K.rows==1 && K.cols==2);
  float k0 = K(0,0);
  float k1 = K(0,1);
  assert(k0 > 0.0f && k1 > 0.0f);
  assert(k1 > k0); // velocity gain typically larger for integrator
}

static void testPoseEstimator(){
  PoseEstimator estimator(0.3f);
  estimator.reset(Pose2d(), 0.0f);
  for (int i=1;i<=10;i++){
    estimator.predict(i*0.1f, ChassisSpeeds(1.0f, 0.0f, 0.0f));
  }
  assert(std::fabs(estimator.pose().x - 1.0f) < 0.05f);
  estimator.addVisionMeasurement(Pose2d(2.0f, 0.0f, 0.0f), 1.2f, 1.0f);
  estimator.predict(1.2f, ChassisSpeeds(0.0f,0.0f,0.0f));
  assert(estimator.pose().x > 1.0f);
}

static void testKinematicsAndOdometry(){
  DifferentialDriveKinematics kin(0.6f);
  auto speeds = kin.toWheelSpeeds(ChassisSpeeds(1.0f, 0.0f, 0.5f));
  auto chassis = kin.toChassisSpeeds(speeds.first, speeds.second);
  assert(std::fabs(chassis.vx - 1.0f) < 1e-5f);
  assert(std::fabs(chassis.omega - 0.5f) < 1e-5f);

  DifferentialDriveOdometry odo;
  odo.reset(Pose2d(), 0.0f, 0.0f);
  auto pose = odo.update(0.5f, 0.6f, 0.1f);
  assert(pose.x > 0.0f);
  assert(pose.heading == 0.1f);
}

int main(){
  testSimpleMotorFF();
  testArmFF();
  testElevatorFF();
  testTrapezoidProfile();
  testSCurveProfile();
  testSlewRateLimiter();
  testBangBangController();
  testRamseteController();
  testHolonomicDriveController();
  testLuenbergerObserver();
  testKalmanFilter();
  testLQR();
  testPoseEstimator();
  testKinematicsAndOdometry();
  std::cout << "All control tests passed\n";
  return 0;
}
