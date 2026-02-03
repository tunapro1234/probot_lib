#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"
#define PROBOT_WIFI_AP_CHANNEL 3  // Opsiyonel (varsayilan 1)

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/command.hpp>  // Includes scheduler, command, subsystem, command_group
#include <probot/command/examples/tank_drive.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// Otonom tank demo icin iki adet VNH motor kontrolcusu.
static constexpr int L_INA = 33;
static constexpr int L_INB = 34;
static constexpr int L_PWM = 35;
static constexpr int L_ENA = -1;
static constexpr int L_ENB = -1;

static constexpr int R_INA = 36;
static constexpr int R_INB = 37;
static constexpr int R_PWM = 38;
static constexpr int R_ENA = -1;
static constexpr int R_ENB = -1;

static probot::motor::BoardozaVNH5019MotorController leftMotor(L_INA, L_INB, L_PWM, L_ENA, L_ENB);
static probot::motor::BoardozaVNH5019MotorController rightMotor(R_INA, R_INB, R_PWM, R_ENA, R_ENB);
static probot::command::examples::TankDrive            chassis(&leftMotor, &rightMotor);

using Scheduler = probot::command::Scheduler;
using namespace probot::command;

// ============================================================================
// Ornek Command'lar - WPILib tarzi command-based otonom
// ============================================================================

// Belirli sure ileri git
class DriveForwardCmd : public CommandBase {
public:
  DriveForwardCmd(probot::command::examples::TankDrive* drive, float power, uint32_t duration_ms)
    : CommandBase("DriveForward"), drive_(drive), power_(power), duration_ms_(duration_ms) {
    addRequirement(drive);
  }

  void initialize() override {
    start_time_ = millis();
    Serial.printf("[Cmd] DriveForward basladi (%.2f, %lums)\n", power_, (unsigned long)duration_ms_);
  }

  void execute(uint32_t, uint32_t) override {
    drive_->drivePower(power_, power_);
  }

  void end(bool interrupted) override {
    drive_->stop();
    Serial.printf("[Cmd] DriveForward bitti (interrupted=%d)\n", interrupted);
  }

  bool isFinished() const override {
    return (millis() - start_time_) >= duration_ms_;
  }

private:
  probot::command::examples::TankDrive* drive_;
  float power_;
  uint32_t duration_ms_;
  uint32_t start_time_ = 0;
};

// Belirli sure don
class TurnCmd : public CommandBase {
public:
  TurnCmd(probot::command::examples::TankDrive* drive, float power, uint32_t duration_ms)
    : CommandBase("Turn"), drive_(drive), power_(power), duration_ms_(duration_ms) {
    addRequirement(drive);
  }

  void initialize() override {
    start_time_ = millis();
    Serial.printf("[Cmd] Turn basladi (%.2f, %lums)\n", power_, (unsigned long)duration_ms_);
  }

  void execute(uint32_t, uint32_t) override {
    drive_->drivePower(power_, -power_);
  }

  void end(bool interrupted) override {
    drive_->stop();
    Serial.printf("[Cmd] Turn bitti (interrupted=%d)\n", interrupted);
  }

  bool isFinished() const override {
    return (millis() - start_time_) >= duration_ms_;
  }

private:
  probot::command::examples::TankDrive* drive_;
  float power_;
  uint32_t duration_ms_;
  uint32_t start_time_ = 0;
};

// Global command instances
static DriveForwardCmd driveForward1(&chassis, 0.5f, 2500);
static WaitCommand     pause1(800);
static TurnCmd         turn90(&chassis, 0.4f, 2200);
static DriveForwardCmd driveToGoal(&chassis, 0.5f, 2000);

// SequentialCommandGroup ile otonom sekans
static SequentialCommandGroup autoSequence("AutoSequence");
static bool g_auto_built = false;

void robotInit() {
  Serial.begin(115200);
  delay(100);

  leftMotor.begin();
  rightMotor.begin();
  leftMotor.setBrakeMode(true);
  rightMotor.setBrakeMode(true);

  // NOTE: Closed-loop/odometry helpers are disabled (not tested yet).
  // chassis.setWheelRadius(32.0f / (2.0f * 3.1415926535f));
  // chassis.setTrackWidth(29.0f);

  // Otonom sekansini olustur: Ileri -> Bekle -> Don -> Ileri
  if (!g_auto_built) {
    autoSequence.addCommands(&driveForward1, &pause1, &turn90, &driveToGoal);
    g_auto_built = true;
  }

  // Subsystem'i kaydet
  Scheduler::instance().registerSubsystem(&chassis);
  Serial.println("[AutonomousDemo] robotInit: Otonom ornegi hazir");
}

void robotEnd() {
  Scheduler::instance().unregisterSubsystem(&chassis);
  chassis.stop();
  Serial.println("[AutonomousDemo] robotEnd: Motorlar durdu");
}

void teleopInit() {
  Serial.println("[AutonomousDemo] teleopInit: Bu ornekte teleop, tank surusu saglar");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  float leftAxis = js.getLeftY();
  float rightAxis = js.getRightY();
  chassis.drivePower(leftAxis, rightAxis);
  delay(20);
}

void autonomousInit() {
  Serial.println("[AutonomousDemo] autonomousInit: SequentialCommandGroup ile otonom");
  Serial.println("  Sekans: Ileri(2.5s) -> Bekle(0.8s) -> Don(2.2s) -> Ileri(2s)");

  // Otonom command'i schedule et
  Scheduler::instance().schedule(&autoSequence);
}

void autonomousLoop() {
  // Command-based sistemde autonomousLoop bos kalabilir
  // Tum is scheduler tarafindan yapiliyor
  delay(20);
}
