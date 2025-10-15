#include "test_harness.hpp"

#include <cmath>

#include <probot/chassis/simple_tank.hpp>
#include <probot/chassis/simple_mecanum.hpp>
#include <probot/chassis/nfr_advanced_tank_drive.hpp>
#include <probot/chassis/nfr_advanced_mecanum_drive.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>
#include <probot/test/test_motor.hpp>
#include <probot/test/test_encoder.hpp>

namespace {
  struct DummyMotor : probot::motor::IMotorDriver {
    void* owner = nullptr;
    float lastPower = 0.0f;
    bool inverted = false;
    bool claim(void* o) override { if (owner && owner != o) return false; owner = o; return true; }
    void release(void* o) override { if (owner == o){ owner = nullptr; lastPower = 0.0f; } }
    bool setPower(float power, void* o) override { if (owner != o) return false; lastPower = inverted ? -power : power; return true; }
    bool isClaimed() const override { return owner != nullptr; }
    void* currentOwner() const override { return owner; }
    void setInverted(bool inv) override { inverted = inv; }
    bool getInverted() const override { return inverted; }
  };

  struct DummyEncoder : probot::sensors::IEncoder {
    int32_t ticks = 0;
    int32_t ticksPerSecond = 0;
    int32_t readTicks() override { return ticks; }
    int32_t readTicksPerSecond() override { return ticksPerSecond; }
  };

  struct MockMotorController : probot::control::IMotorController {
    void* owner = nullptr;
    float lastSetpointValue = 0.0f;
    probot::control::ControlType lastMode = probot::control::ControlType::kPercent;
    int lastSlot = -1;
    probot::control::MotionProfileType profileType = probot::control::MotionProfileType::kNone;
    probot::control::MotionProfileConfig profileCfg{};
    bool inverted = false;
    bool setPowerCalled = false;
    float lastPower = 0.0f;

    bool claim(void* o) override {
      if (owner && owner != o) return false;
      owner = o;
      return true;
    }
    void release(void* o) override { if (owner == o) owner = nullptr; }
    bool setPower(float power, void* o) override {
      if (owner != o) return false;
      setPowerCalled = true;
      lastPower = inverted ? -power : power;
      return true;
    }
    bool isClaimed() const override { return owner != nullptr; }
    void* currentOwner() const override { return owner; }
    void setInverted(bool inv) override { inverted = inv; }
    bool getInverted() const override { return inverted; }

    void setSetpoint(float value, probot::control::ControlType mode, int slot = -1) override {
      lastSetpointValue = value;
      lastMode = mode;
      lastSlot = slot;
    }
    void setTimeoutMs(uint32_t) override {}
    void setPidSlotConfig(int, const probot::control::PidConfig&) override {}
    void selectDefaultSlot(probot::control::ControlType, int) override {}
    int defaultSlot(probot::control::ControlType) const override { return 0; }
    float lastSetpoint() const override { return lastSetpointValue; }
    float lastMeasurement() const override { return 0.0f; }
    float lastOutput() const override { return lastPower; }
    probot::control::ControlType activeMode() const override { return lastMode; }
    bool isAtTarget(float) const override { return false; }
    void setMotionProfile(probot::control::MotionProfileType type) override { profileType = type; }
    probot::control::MotionProfileType motionProfile() const override { return profileType; }
    void setMotionProfileConfig(const probot::control::MotionProfileConfig& cfg) override { profileCfg = cfg; }
    probot::control::MotionProfileConfig motionProfileConfig() const override { return profileCfg; }
    void update(uint32_t, uint32_t) override {}
  };
}

TEST_CASE(basic_tank_drive_clamp_and_invert){
  DummyMotor left, right;
  probot::chassis::SimpleTankDrive tank(&left, &right);
  tank.drive(2.0f, -2.0f);
  EXPECT_NEAR(left.lastPower, 1.0f, 1e-5f);
  EXPECT_NEAR(right.lastPower, -1.0f, 1e-5f);

  tank.setInverted(true, false);
  tank.drive(0.5f, 0.5f);
  EXPECT_NEAR(left.lastPower, -0.5f, 1e-5f);
  EXPECT_NEAR(right.lastPower, 0.5f, 1e-5f);

  tank.stop();
  EXPECT_NEAR(left.lastPower, 0.0f, 1e-5f);
  EXPECT_NEAR(right.lastPower, 0.0f, 1e-5f);
}

TEST_CASE(basic_mecanum_drive_normalizes_outputs){
  DummyMotor fl, fr, rl, rr;
  probot::chassis::SimpleMecanumDrive mech(&fl, &fr, &rl, &rr);
  mech.setInverted(false, true, false, true);

  float vx = 0.8f, vy = 0.4f, omega = 0.3f;
  mech.driveCartesian(vx, vy, omega);

  float raw_fl = vx - vy - omega;
  float raw_fr = vx + vy + omega;
  float raw_rl = vx + vy - omega;
  float raw_rr = vx - vy + omega;
  float maxMag = std::max({std::fabs(raw_fl), std::fabs(raw_fr), std::fabs(raw_rl), std::fabs(raw_rr), 1.0f});
  raw_fl /= maxMag;
  raw_fr /= maxMag;
  raw_rl /= maxMag;
  raw_rr /= maxMag;

  EXPECT_NEAR(fl.lastPower, raw_fl, 1e-5f);
  EXPECT_NEAR(fr.lastPower, -raw_fr, 1e-5f); // inverted
  EXPECT_NEAR(rl.lastPower, raw_rl, 1e-5f);
  EXPECT_NEAR(rr.lastPower, -raw_rr, 1e-5f);

  mech.stop();
  EXPECT_NEAR(fl.lastPower, 0.0f, 1e-5f);
  EXPECT_NEAR(fr.lastPower, 0.0f, 1e-5f);
  EXPECT_NEAR(rl.lastPower, 0.0f, 1e-5f);
  EXPECT_NEAR(rr.lastPower, 0.0f, 1e-5f);
}

static probot::control::PidConfig makeWheelPid(float kp){
  probot::control::PidConfig cfg{};
  cfg.kp = kp;
  cfg.ki = 0.0f;
  cfg.kd = 0.0f;
  cfg.out_min = -1.0f;
  cfg.out_max = 1.0f;
  return cfg;
}

TEST_CASE(nfr_tank_drive_closed_loop_should_command_power){
  DummyEncoder encL, encR;
  probot::test::TestMotor motorL;
  probot::test::TestMotor motorR;
  auto cfg = makeWheelPid(0.2f);
  probot::control::PID pidL(cfg);
  probot::control::PID pidR(cfg);
  probot::control::ClosedLoopMotor clL(&encL, &pidL, &motorL, 1.0f, 1.0f);
  probot::control::ClosedLoopMotor clR(&encR, &pidR, &motorR, 1.0f, 1.0f);
  clL.setTimeoutMs(0);
  clR.setTimeoutMs(0);
  EXPECT_TRUE(motorL.isClaimed());
  EXPECT_TRUE(motorR.isClaimed());

  probot::chassis::NfrAdvancedTankDrive chassis(&clL, &clR);
  chassis.resetPose(probot::control::Pose2d(), 0.0f, 0.0f);
  chassis.setTargetSpeeds(probot::control::ChassisSpeeds(1.0f, 0.0f, 0.0f));

  chassis.update(0.0f, 0.0f, 0.0f);
  chassis.update(0.1f, 0.0f, 0.0f);

  EXPECT_TRUE(std::fabs(motorL.appliedPower()) > 1e-4f);
  EXPECT_TRUE(std::fabs(motorR.appliedPower()) > 1e-4f);
}

TEST_CASE(nfr_mecanum_drive_closed_loop_should_command_power){
  DummyEncoder encFL, encFR, encRL, encRR;
  probot::test::TestMotor motorFL, motorFR, motorRL, motorRR;
  auto cfgWheel = makeWheelPid(0.4f);
  probot::control::PID pidFL(cfgWheel), pidFR(cfgWheel), pidRL(cfgWheel), pidRR(cfgWheel);
  probot::control::ClosedLoopMotor clFL(&encFL, &pidFL, &motorFL, 1.0f, 1.0f);
  probot::control::ClosedLoopMotor clFR(&encFR, &pidFR, &motorFR, 1.0f, 1.0f);
  probot::control::ClosedLoopMotor clRL(&encRL, &pidRL, &motorRL, 1.0f, 1.0f);
  probot::control::ClosedLoopMotor clRR(&encRR, &pidRR, &motorRR, 1.0f, 1.0f);
  clFL.setTimeoutMs(0);
  clFR.setTimeoutMs(0);
  clRL.setTimeoutMs(0);
  clRR.setTimeoutMs(0);
  EXPECT_TRUE(motorFL.isClaimed());
  EXPECT_TRUE(motorFR.isClaimed());
  EXPECT_TRUE(motorRL.isClaimed());
  EXPECT_TRUE(motorRR.isClaimed());

  probot::chassis::NfrAdvancedMecanumDrive chassis(&clFL, &clFR, &clRL, &clRR);
  probot::control::kinematics::WheelPositions4 wheels{0.0f, 0.0f, 0.0f, 0.0f};
  chassis.resetPose(probot::control::Pose2d(), 0.0f, wheels);
  chassis.setTargetSpeeds(probot::control::ChassisSpeeds(0.6f, 0.1f, 0.05f));

  chassis.update(0.0f, wheels);
  chassis.update(0.1f, wheels);

  EXPECT_TRUE(std::fabs(motorFL.appliedPower()) > 1e-4f);
  EXPECT_TRUE(std::fabs(motorFR.appliedPower()) > 1e-4f);
  EXPECT_TRUE(std::fabs(motorRL.appliedPower()) > 1e-4f);
  EXPECT_TRUE(std::fabs(motorRR.appliedPower()) > 1e-4f);
}

TEST_CASE(nfr_tank_drive_motion_profile_propagation){
  MockMotorController left;
  MockMotorController right;
  probot::chassis::NfrAdvancedTankDrive::Config cfg;
  cfg.useMotorControllerVelocity = true;
  cfg.wheelProfileType = probot::control::MotionProfileType::kTrapezoid;
  cfg.wheelProfileConfig = {1.2f, 3.4f, 0.0f};

  probot::chassis::NfrAdvancedTankDrive chassis(&left, &right, cfg);
  EXPECT_TRUE(left.profileType == probot::control::MotionProfileType::kTrapezoid);
  EXPECT_NEAR(left.profileCfg.maxVelocity, 1.2f, 1e-6f);
  EXPECT_NEAR(left.profileCfg.maxAcceleration, 3.4f, 1e-6f);
  EXPECT_TRUE(right.profileType == probot::control::MotionProfileType::kTrapezoid);

  // S-Curve disabled - test with Trapezoid instead
  probot::control::MotionProfileConfig newCfg{2.0f, 5.0f, 0.0f};
  chassis.setWheelMotionProfile(probot::control::MotionProfileType::kTrapezoid, newCfg);
  EXPECT_TRUE(left.profileType == probot::control::MotionProfileType::kTrapezoid);
  EXPECT_NEAR(left.profileCfg.maxVelocity, 2.0f, 1e-6f);
  EXPECT_TRUE(right.profileType == probot::control::MotionProfileType::kTrapezoid);

  chassis.resetPose(probot::control::Pose2d(), 0.0f, 0.0f);
  chassis.setTargetSpeeds(probot::control::ChassisSpeeds(0.5f, 0.0f, 0.0f));
  chassis.useMotorControllerVelocityLoop(true);
  chassis.update(0.0f, 0.0f, 0.0f);
  chassis.update(0.1f, 0.0f, 0.0f);

  EXPECT_TRUE(left.lastMode == probot::control::ControlType::kVelocity);
  EXPECT_TRUE(right.lastMode == probot::control::ControlType::kVelocity);
  EXPECT_TRUE(!left.setPowerCalled);
  EXPECT_TRUE(!right.setPowerCalled);
  EXPECT_TRUE(std::fabs(left.lastSetpointValue) > 0.0f);
  EXPECT_TRUE(std::fabs(right.lastSetpointValue) > 0.0f);
}

TEST_CASE(nfr_mecanum_drive_motion_profile_propagation){
  MockMotorController fl, fr, rl, rr;
  probot::chassis::NfrAdvancedMecanumDrive::Config cfg;
  cfg.useMotorControllerVelocity = true;
  cfg.wheelProfileType = probot::control::MotionProfileType::kTrapezoid;
  cfg.wheelProfileConfig = {0.8f, 2.5f, 0.0f};

  probot::chassis::NfrAdvancedMecanumDrive chassis(&fl, &fr, &rl, &rr, cfg);
  EXPECT_TRUE(fl.profileType == probot::control::MotionProfileType::kTrapezoid);
  EXPECT_NEAR(fr.profileCfg.maxAcceleration, 2.5f, 1e-6f);

  chassis.resetPose(probot::control::Pose2d(), 0.0f, probot::control::kinematics::WheelPositions4{});
  chassis.setTargetSpeeds(probot::control::ChassisSpeeds(0.4f, 0.1f, 0.0f));
  chassis.useMotorControllerVelocityLoop(true);
  probot::control::kinematics::WheelPositions4 wheels{};
  chassis.update(0.0f, wheels);
  chassis.update(0.1f, wheels);

  EXPECT_TRUE(fl.lastMode == probot::control::ControlType::kVelocity);
  EXPECT_TRUE(fr.lastMode == probot::control::ControlType::kVelocity);
  EXPECT_TRUE(rl.lastMode == probot::control::ControlType::kVelocity);
  EXPECT_TRUE(rr.lastMode == probot::control::ControlType::kVelocity);
  EXPECT_TRUE(!fl.setPowerCalled);
  EXPECT_TRUE(!fr.setPowerCalled);
  EXPECT_TRUE(!rl.setPowerCalled);
  EXPECT_TRUE(!rr.setPowerCalled);
}
