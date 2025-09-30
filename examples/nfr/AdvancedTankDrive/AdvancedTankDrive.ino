#include <probot.h>
#include <probot/control/pid.hpp>
#include <probot/control/feedforward/simple_motor_ff.hpp>
#include <probot/chassis/nfr_advanced_tank_drive.hpp>
#include <probot/sensors/imu/mpu6050.hpp>
#include <probot/devices/motors/motor_handle.hpp>
#include <probot/test/test_motor.hpp>

// Example demonstrating the NFRAdvancedTankDrive structure with simulated hardware.

static probot::motor::NullMotor leftHW;
static probot::motor::NullMotor rightHW;
static probot::motor::MotorHandle leftHandle(leftHW);
static probot::motor::MotorHandle rightHandle(rightHW);

static probot::control::PID pidLeft({0.4f,0.0f,0.0f,-1.0f,1.0f});
static probot::control::PID pidRight({0.4f,0.0f,0.0f,-1.0f,1.0f});

// Null controllers stand in for real motor controllers – swap with real ones on hardware.
class NullMotorController : public probot::control::IMotorController {
public:
  bool claim(void* owner) override { return handle_.underlying().claim(owner); }
  void release(void* owner) override { handle_.underlying().release(owner); }
  bool setPower(float power, void* owner) override { return handle_.underlying().setPower(power, owner); }
  bool isClaimed() const override { return handle_.underlying().isClaimed(); }
  void* currentOwner() const override { return handle_.underlying().currentOwner(); }
  void setInverted(bool inv) override { handle_.setInverted(inv); }
  bool getInverted() const override { return handle_.getInverted(); }

  void setSetpoint(float value, probot::control::ControlType mode, int slot = -1) override {}
  void setTimeoutMs(uint32_t) override {}
  void setPidSlotConfig(int, const probot::control::PidConfig&) override {}
  void selectDefaultSlot(probot::control::ControlType, int) override {}
  int defaultSlot(probot::control::ControlType) const override { return 0; }
  float lastSetpoint() const override { return 0.0f; }
  float lastMeasurement() const override { return 0.0f; }
  float lastOutput() const override { return 0.0f; }
  probot::control::ControlType activeMode() const override { return probot::control::ControlType::kVelocity; }
  bool isAtTarget(float) const override { return false; }
  void setMotionProfile(probot::control::MotionProfileType type) override { profileType_ = type; }
  probot::control::MotionProfileType motionProfile() const override { return profileType_; }
  void setMotionProfileConfig(const probot::control::MotionProfileConfig& cfg) override { profileCfg_ = cfg; }
  probot::control::MotionProfileConfig motionProfileConfig() const override { return profileCfg_; }
  void update(uint32_t, uint32_t) override {}

  NullMotorController(probot::motor::MotorHandle& handle) : handle_(handle) {}
private:
  probot::motor::MotorHandle& handle_;
  probot::control::MotionProfileType profileType_{probot::control::MotionProfileType::kNone};
  probot::control::MotionProfileConfig profileCfg_{};
};

static NullMotorController leftController(leftHandle);
static NullMotorController rightController(rightHandle);

static probot::chassis::NfrAdvancedTankDrive::Config config;
static probot::chassis::NfrAdvancedTankDrive chassis(&leftController, &rightController, config);
static probot::sensors::imu::Mpu6050 imu;

// Simulated encoder positions
static float leftPosition = 0.0f;
static float rightPosition = 0.0f;

void robotInit(){
  Serial.println("[AdvancedTankDrive] robotInit");
  chassis.setImu(&imu);
  chassis.setFeedforwardGains(0.1f, 1.0f, 0.1f);
  chassis.setWheelMotionProfile(probot::control::MotionProfileType::kTrapezoid,
                                {0.8f, 2.0f, 0.0f});
  chassis.useMotorControllerVelocityLoop(true);
  chassis.setVelocityPidConfig({0.5f,0.0f,0.0f,0.0f,-1.0f,1.0f});
  chassis.setRamseteParams(2.0f, 0.7f);
  chassis.resetPose(probot::control::Pose2d(), leftPosition, rightPosition);
}

void teleopInit(){ Serial.println("[AdvancedTankDrive] teleopInit"); }

void teleopLoop(){
  static uint32_t last = 0;
  uint32_t now = millis();
  float dt = (now - last) * 0.001f;
  if (dt <= 0.0f) dt = 0.02f;
  last = now;

  float desiredV = 0.5f;
  leftPosition += desiredV * dt;
  rightPosition += desiredV * dt;

  chassis.setTargetSpeeds(probot::control::ChassisSpeeds(desiredV, 0.0f, 0.0f));
  chassis.update(now * 0.001f, leftPosition, rightPosition);
  delay(20);
}

void autonomousInit(){ Serial.println("[AdvancedTankDrive] autonomousInit"); }
void autonomousLoop(){ delay(20); }
void robotEnd(){ Serial.println("[AdvancedTankDrive] robotEnd"); }
