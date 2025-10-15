#include "test_harness.hpp"

#include <cmath>

#include <probot/control/geometry.hpp>
#include <probot/control/kinematics/differential_drive_kinematics.hpp>
#include <probot/control/kinematics/mecanum_drive_kinematics.hpp>
#include <probot/control/odometry/differential_drive_odometry.hpp>
#include <probot/control/odometry/mecanum_drive_odometry.hpp>

TEST_CASE(geometry_normalize_angle){
  EXPECT_NEAR(probot::control::normalizeAngle(3.5f), -2.7831855f, 1e-5f);
  EXPECT_NEAR(probot::control::normalizeAngle(-4.0f), 2.2831855f, 1e-5f);
}

TEST_CASE(geometry_robot_relative_error){
  probot::control::Pose2d ref(1.0f, 0.0f, 0.0f);
  probot::control::Pose2d cur(0.0f, 0.0f, 0.0f);
  auto err = probot::control::computeRobotRelativeError(ref, cur);
  EXPECT_NEAR(err.x, 1.0f, 1e-5f);
  EXPECT_NEAR(err.y, 0.0f, 1e-5f);
  EXPECT_NEAR(err.heading, 0.0f, 1e-5f);
}

TEST_CASE(differential_drive_kinematics_roundtrip){
  probot::control::kinematics::DifferentialDriveKinematics kin(0.6f);
  probot::control::ChassisSpeeds target(1.0f, 0.0f, 0.5f);
  auto wheels = kin.toWheelSpeeds(target);
  auto back = kin.toChassisSpeeds(wheels.first, wheels.second);
  EXPECT_NEAR(back.vx, target.vx, 1e-5f);
  EXPECT_NEAR(back.omega, target.omega, 1e-5f);
}

TEST_CASE(mecanum_drive_kinematics_roundtrip){
  probot::control::kinematics::MecanumDriveKinematics kin(0.3f, 0.4f);
  probot::control::ChassisSpeeds target(1.0f, -0.5f, 0.2f);
  auto wheels = kin.toWheelSpeeds(target);
  auto back = kin.toChassisSpeeds(wheels);
  EXPECT_NEAR(back.vx, target.vx, 1e-5f);
  EXPECT_NEAR(back.vy, target.vy, 1e-5f);
  EXPECT_NEAR(back.omega, target.omega, 1e-5f);
}

TEST_CASE(differential_drive_odometry_update){
  probot::control::odometry::DifferentialDriveOdometry odo;
  odo.reset(probot::control::Pose2d(), 0.0f, 0.0f);
  auto pose = odo.update(0.5f, 0.6f, 0.1f);
  EXPECT_TRUE(pose.x > 0.0f);
  EXPECT_NEAR(pose.heading, 0.1f, 1e-5f);
}

TEST_CASE(mecanum_drive_odometry_tracks_motion){
  probot::control::odometry::MecanumDriveOdometry odo;
  probot::control::kinematics::MecanumDriveKinematics kin(0.3f, 0.4f);
  probot::control::kinematics::WheelPositions4 wheels{0.0f, 0.0f, 0.0f, 0.0f};
  odo.reset(probot::control::Pose2d(), 0.0f);
  odo.update(0.0f, wheels, kin);

  wheels.frontLeft = wheels.frontRight = wheels.rearLeft = wheels.rearRight = 0.2f;
  auto pose1 = odo.update(0.1f, wheels, kin);
  EXPECT_TRUE(pose1.x > 0.0f);
  EXPECT_NEAR(pose1.y, 0.0f, 1e-4f);

  wheels.frontLeft += -0.1f;
  wheels.frontRight += 0.1f;
  wheels.rearLeft += 0.1f;
  wheels.rearRight += -0.1f;
  auto pose2 = odo.update(0.2f, wheels, kin);
  EXPECT_TRUE(std::fabs(pose2.y) > 1e-4f);
}
