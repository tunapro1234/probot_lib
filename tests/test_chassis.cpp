#include "test_harness.hpp"

#include <algorithm>
#include <cmath>

#include <probot/command/examples/tank_drive.hpp>
#include <probot/command/examples/mecanum_drive.hpp>
#include <probot/test/test_encoder.hpp>

namespace {
  struct MotorStub : probot::motor::IMotorController {
    float lastPower = 0.0f;
    float lastVelocity = 0.0f;
    float lastPosition = 0.0f;
    bool inverted = false;
    bool velSupported = false;
    bool posSupported = false;

    bool setPower(float power) override {
      lastPower = inverted ? -power : power;
      return true;
    }
    void setInverted(bool inv) override { inverted = inv; }
    bool getInverted() const override { return inverted; }

    bool supportsVelocity() const override { return velSupported; }
    bool supportsPosition() const override { return posSupported; }
    bool setVelocity(float units_per_s) override { lastVelocity = units_per_s; return velSupported; }
    bool setPosition(float units) override { lastPosition = units; return posSupported; }
  };
}

TEST_CASE(tank_drive_power_clamp_and_invert){
  MotorStub left, right;
  probot::command::examples::TankDrive tank(&left, &right);
  tank.drivePower(2.0f, -2.0f);
  EXPECT_NEAR(left.lastPower, 1.0f, 1e-5f);
  EXPECT_NEAR(right.lastPower, -1.0f, 1e-5f);

  tank.setInverted(true, false);
  tank.drivePower(0.5f, 0.5f);
  EXPECT_NEAR(left.lastPower, -0.5f, 1e-5f);
  EXPECT_NEAR(right.lastPower, 0.5f, 1e-5f);
}

TEST_CASE(tank_drive_velocity_normalized){
  MotorStub left, right;
  left.velSupported = true;
  right.velSupported = true;
  probot::command::examples::TankDrive tank(&left, &right);
  tank.setWheelRadius(1.0f);
  tank.setGearRatio(1.0f);
  tank.setMaxVelocity(2.0f);

  tank.driveVelocityNormalized(0.5f, -0.5f);

  float expected = 1.0f / (2.0f * 3.1415926535f);
  EXPECT_NEAR(left.lastVelocity, expected, 1e-5f);
  EXPECT_NEAR(right.lastVelocity, -expected, 1e-5f);
}

TEST_CASE(tank_drive_velocity_status_unsupported){
  MotorStub left, right;
  probot::command::examples::TankDrive tank(&left, &right);

  tank.driveVelocity(1.0f, 1.0f);

  EXPECT_TRUE(tank.lastStatus() == probot::command::examples::TankDrive::CommandStatus::kVelocityUnsupported);
}

TEST_CASE(tank_drive_odometry_updates_pose){
  MotorStub left, right;
  probot::test::TestEncoder encL;
  probot::test::TestEncoder encR;
  probot::command::examples::TankDrive tank(&left, &right, &encL, &encR);
  tank.setWheelRadius(1.0f);
  tank.setGearRatio(1.0f);
  tank.setTrackWidth(2.0f);
  tank.setEncoderTicksPerRevolution(10.0f);

  encL.setTicks(0);
  encR.setTicks(0);
  tank.periodic(0, 20);

  encL.setTicks(10);
  encR.setTicks(10);
  tank.periodic(20, 20);

  EXPECT_NEAR(tank.pose().x, 2.0f * 3.1415926535f, 1e-4f);
  EXPECT_NEAR(tank.pose().y, 0.0f, 1e-4f);
}

TEST_CASE(mecanum_drive_power_normalizes_outputs){
  MotorStub fl, fr, rl, rr;
  probot::command::examples::MecanumDrive mech(&fl, &fr, &rl, &rr);
  mech.setInverted(false, true, false, true);

  float vx = 0.8f, vy = 0.4f, omega = 0.3f;
  mech.drivePower(vx, vy, omega);

  float k = 2.0f;
  float raw_fl = vx - vy - omega * k;
  float raw_fr = vx + vy + omega * k;
  float raw_rl = vx + vy - omega * k;
  float raw_rr = vx - vy + omega * k;
  float maxMag = std::max({std::fabs(raw_fl), std::fabs(raw_fr), std::fabs(raw_rl), std::fabs(raw_rr), 1.0f});
  raw_fl /= maxMag;
  raw_fr /= maxMag;
  raw_rl /= maxMag;
  raw_rr /= maxMag;

  EXPECT_NEAR(fl.lastPower, raw_fl, 1e-5f);
  EXPECT_NEAR(fr.lastPower, -raw_fr, 1e-5f);
  EXPECT_NEAR(rl.lastPower, raw_rl, 1e-5f);
  EXPECT_NEAR(rr.lastPower, -raw_rr, 1e-5f);
}

TEST_CASE(mecanum_drive_velocity_uses_controller){
  MotorStub fl, fr, rl, rr;
  fl.velSupported = true;
  fr.velSupported = true;
  rl.velSupported = true;
  rr.velSupported = true;

  probot::command::examples::MecanumDrive mech(&fl, &fr, &rl, &rr);
  mech.setWheelRadius(1.0f);
  mech.setGearRatio(1.0f);
  mech.setMaxVelocity(2.0f);
  mech.setMaxAngularVelocity(1.0f);

  mech.driveVelocityNormalized(1.0f, 0.0f, 0.0f);

  float expected = 2.0f / (2.0f * 3.1415926535f);
  EXPECT_NEAR(fl.lastVelocity, expected, 1e-5f);
  EXPECT_NEAR(fr.lastVelocity, expected, 1e-5f);
  EXPECT_NEAR(rl.lastVelocity, expected, 1e-5f);
  EXPECT_NEAR(rr.lastVelocity, expected, 1e-5f);
}

TEST_CASE(mecanum_drive_velocity_status_unsupported){
  MotorStub fl, fr, rl, rr;
  probot::command::examples::MecanumDrive mech(&fl, &fr, &rl, &rr);

  mech.driveVelocity(1.0f, 0.0f, 0.0f);

  EXPECT_TRUE(mech.lastStatus() == probot::command::examples::MecanumDrive::CommandStatus::kVelocityUnsupported);
}
