#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/command/scheduler.hpp>
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


enum class AutoStep {
  kDriveForward,
  kPause,
  kTurn,
  kDriveToGoal,
  kFinished
};

static AutoStep g_step = AutoStep::kDriveForward;
static uint32_t g_stepStart = 0;

void robotInit() {
  Serial.begin(115200);
  delay(100);

  leftMotor.begin();
  rightMotor.begin();
  leftMotor.setBrakeMode(true);
  rightMotor.setBrakeMode(true);

  chassis.setWheelRadius(32.0f / (2.0f * 3.1415926535f));
  chassis.setTrackWidth(29.0f);

  probot::command::scheduler::attach(&chassis);
  Serial.println("[AutonomousDemo] robotInit: Otonom örneği hazır");
}

void robotEnd() {
  probot::command::scheduler::detach(&chassis);
  chassis.stop();
  Serial.println("[AutonomousDemo] robotEnd: Motorlar durdu");
}

void teleopInit() {
  Serial.println("[AutonomousDemo] teleopInit: Bu örnekte teleop, tank sürüşü sağlar");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  float leftAxis = js.getLeftY();
  float rightAxis = js.getRightY();
  chassis.drivePower(leftAxis, rightAxis);
  delay(20);
}

void autonomousInit() {
  Serial.println("[AutonomousDemo] autonomousInit: İleri -> Bekle -> Dön -> İleri");
  g_step = AutoStep::kDriveForward;
  g_stepStart = millis();
  chassis.drivePower(0.5f, 0.5f);
}

void autonomousLoop() {
  uint32_t now = millis();

  switch (g_step) {
    case AutoStep::kDriveForward:
      chassis.drivePower(0.5f, 0.5f);
      if (now - g_stepStart > 2500) {
        g_step = AutoStep::kPause;
        g_stepStart = now;
        chassis.stop();
        Serial.println("[AutonomousDemo] Duraklama");
      }
      break;

    case AutoStep::kPause:
      chassis.stop();
      if (now - g_stepStart > 800) {
        g_step = AutoStep::kTurn;
        g_stepStart = now;
        chassis.drivePower(0.4f, -0.4f);
        Serial.println("[AutonomousDemo] 90 derece dönüş başlıyor");
      }
      break;

    case AutoStep::kTurn:
      chassis.drivePower(0.4f, -0.4f);
      if (now - g_stepStart > 2200) {
        g_step = AutoStep::kDriveToGoal;
        g_stepStart = now;
        chassis.drivePower(0.5f, 0.5f);
        Serial.println("[AutonomousDemo] Hedefe son itiş");
      }
      break;

    case AutoStep::kDriveToGoal:
      chassis.drivePower(0.5f, 0.5f);
      if (now - g_stepStart > 2000) {
        g_step = AutoStep::kFinished;
        chassis.stop();
        Serial.println("[AutonomousDemo] Otonom tamamlandı");
      }
      break;

    case AutoStep::kFinished:
    default:
      chassis.stop();
      break;
  }

  delay(20);
}
