#include "test_harness.hpp"

#include <cmath>

#include <probot/control/geometry.hpp>
#include <probot/control/pid.hpp>
#include <probot/control/trajectory/holonomic_drive_controller.hpp>
#include <probot/control/trajectory/ramsete_controller.hpp>

TEST_CASE(ramsete_controller_basic){
  probot::control::trajectory::RamseteController controller;
  probot::control::Pose2d current(0.0f, 0.0f, 0.0f);
  probot::control::Pose2d desired(0.0f, 0.0f, 0.0f);
  probot::control::ChassisSpeeds ref(1.0f, 0.0f, 0.0f);

  auto out = controller.calculate(current, desired, ref);
  EXPECT_NEAR(out.vx, 1.0f, 1e-5f);
  EXPECT_NEAR(out.omega, 0.0f, 1e-5f);

  desired = probot::control::Pose2d(1.0f, 0.0f, 0.0f);
  out = controller.calculate(current, desired, ref);
  EXPECT_TRUE(out.vx > 1.0f);

  controller.setTolerance(0.05f, 0.05f, 0.05f);
  EXPECT_TRUE(!controller.atReference());
}

TEST_CASE(holonomic_drive_controller_basic){
  probot::control::PidConfig cfg{1.0f, 0.0f, 0.0f, 0.0f, -5.0f, 5.0f};
  probot::control::PID x(cfg), y(cfg), theta(cfg);
  probot::control::trajectory::HolonomicDriveController controller(&x, &y, &theta);
  controller.setTolerance(0.05f, 0.05f, 0.05f);

  probot::control::Pose2d current(0.0f, 0.0f, 0.0f);
  probot::control::Pose2d desired(1.0f, -0.5f, 0.3f);

  auto out = controller.calculate(current, desired, probot::control::ChassisSpeeds(), 0.02f);
  EXPECT_TRUE(out.vx > 0.0f);
  EXPECT_TRUE(out.vy < 0.0f);
  EXPECT_TRUE(out.omega > 0.0f);
  EXPECT_TRUE(!controller.atReference());

  controller.calculate(desired, desired, probot::control::ChassisSpeeds(), 0.02f);
  EXPECT_TRUE(controller.atReference());
}
