#include <probot.h>
#include <probot/chassis/nfr_advanced_mecanum_drive.hpp>
#include <probot/sensors/imu/mpu6050.hpp>
#include <probot/test/test_motor.hpp>

static probot::motor::NullMotor flHW, frHW, rlHW, rrHW;

class NullMotorController : public probot::control::IMotorController {
public:
  explicit NullMotorController(probot::motor::IMotorDriver& driver) : driver_(driver) {}
  bool setPower(float power) override { return driver_.setPower(power); }
  void setInverted(bool inv) override { driver_.setInverted(inv); }
  bool getInverted() const override { return driver_.getInverted(); }
  void setSetpoint(float, probot::controllers::ControlType, int) override {}
  void setTimeoutMs(uint32_t) override {}
  void setPidSlotConfig(int, const probot::control::PidConfig&) override {}
  void selectDefaultSlot(probot::controllers::ControlType, int) override {}
  int defaultSlot(probot::controllers::ControlType) const override { return 0; }
  float lastSetpoint() const override { return 0.0f; }
  float lastMeasurement() const override { return 0.0f; }
  float lastOutput() const override { return 0.0f; }
  probot::controllers::ControlType activeMode() const override { return probot::controllers::ControlType::kVelocity; }
  bool isAtTarget(float) const override { return false; }
  void setMotionProfile(probot::control::MotionProfileType type) override { profileType_ = type; }
  probot::control::MotionProfileType motionProfile() const override { return profileType_; }
  void setMotionProfileConfig(const probot::control::MotionProfileConfig& cfg) override { profileCfg_ = cfg; }
  probot::control::MotionProfileConfig motionProfileConfig() const override { return profileCfg_; }
  void update(uint32_t, uint32_t) override {}
private:
  probot::motor::IMotorDriver& driver_;
  probot::control::MotionProfileType profileType_{probot::control::MotionProfileType::kNone};
  probot::control::MotionProfileConfig profileCfg_{};
};

static NullMotorController fl(flHW), fr(frHW), rl(rlHW), rr(rrHW);
static probot::chassis::NfrAdvancedMecanumDrive::Config config;
static probot::chassis::NfrAdvancedMecanumDrive chassis(&fl, &fr, &rl, &rr, config);
static probot::sensors::imu::Mpu6050 imu;

static probot::control::kinematics::WheelPositions4 wheelPositions{};

void robotInit(){
  Serial.println("[AdvancedMecanumDrive] robotInit");
  chassis.setImu(&imu);
  chassis.setFeedforwardGains(0.05f, 0.9f, 0.1f);
  chassis.setWheelMotionProfile(probot::control::MotionProfileType::kTrapezoid,
                                {0.6f, 1.8f, 0.0f});
  chassis.useMotorControllerVelocityLoop(true);
  chassis.resetPose(probot::control::Pose2d(), 0.0f, wheelPositions);
}

void teleopInit(){ Serial.println("[AdvancedMecanumDrive] teleopInit"); }

void teleopLoop(){
  static uint32_t lastMs = 0;
  uint32_t now = millis();
  float dt = (now - lastMs) * 0.001f;
  if (dt <= 0.0f) dt = 0.02f;
  lastMs = now;

  // Simulated wheel motion for demo (forward motion)
  float delta = 0.3f * dt;
  wheelPositions.frontLeft += delta;
  wheelPositions.frontRight += delta;
  wheelPositions.rearLeft += delta;
  wheelPositions.rearRight += delta;

  chassis.setTargetSpeeds(probot::control::ChassisSpeeds(0.3f, 0.0f, 0.0f));
  chassis.update(now * 0.001f, wheelPositions);
  delay(20);
}

void autonomousInit(){ Serial.println("[AdvancedMecanumDrive] autonomousInit"); }
void autonomousLoop(){ delay(20); }
void robotEnd(){ Serial.println("[AdvancedMecanumDrive] robotEnd"); }
