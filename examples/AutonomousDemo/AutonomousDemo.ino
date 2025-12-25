#include <probot.h>
#include <probot/test/null_encoder.hpp>
#include <probot/io/joystick_api.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>
#include <probot/chassis/basic_tank_drive.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>

// Otonom tank demo için iki adet VNH sürücüsü.
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

static probot::motor::BoardozaVNHMotorDriver leftDriver(L_INA, L_INB, L_PWM, L_ENA, L_ENB);
static probot::motor::BoardozaVNHMotorDriver rightDriver(R_INA, R_INB, R_PWM, R_ENA, R_ENB);
static probot::test::NullEncoder          leftEncoder;
static probot::test::NullEncoder          rightEncoder;

static const probot::control::PidConfig      kPid{.kp = 0.32f, .ki = 0.015f, .kd = 0.0f, .kf = 0.0f,
                                                  .out_min = -1.0f, .out_max = 1.0f};
static probot::control::PID                  pidLeft(kPid);
static probot::control::PID                  pidRight(kPid);
static probot::control::ClosedLoopMotor      motorLeft(&leftEncoder, &pidLeft, &leftDriver, 1.0f, 1.0f);
static probot::control::ClosedLoopMotor      motorRight(&rightEncoder, &pidRight, &rightDriver, 1.0f, 1.0f);
static probot::drive::BasicTankDrive         chassis(&motorLeft, &motorRight);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

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

  leftDriver.begin();
  rightDriver.begin();
  leftDriver.setBrakeMode(true);
  rightDriver.setBrakeMode(true);

  motorLeft.setTimeoutMs(0);
  motorRight.setTimeoutMs(0);

  chassis.setWheelCircumference(32.0f);
  chassis.setTrackWidth(29.0f);

  Serial.println("[AutonomousDemo] robotInit: Otonom örneği hazır");
}

void robotEnd() {
  chassis.setVelocity(0.0f, 0.0f);
  Serial.println("[AutonomousDemo] robotEnd: Motorlar durdu");
}

void teleopInit() {
  Serial.println("[AutonomousDemo] teleopInit: Bu örnekte teleop, tank sürüşü sağlar");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  float left = js.getLeftY() * 100.0f;
  float right = js.getRightY() * 100.0f;
  chassis.setVelocity(left, right);
  chassis.update(millis(), 20);
  delay(20);
}

void autonomousInit() {
  Serial.println("[AutonomousDemo] autonomousInit: İleri -> Bekle -> 90° dön -> Hedefe git");
  g_step = AutoStep::kDriveForward;
  g_stepStart = millis();
  chassis.driveDistance(80.0f); // 80 cm ileri
}

void autonomousLoop() {
  uint32_t now = millis();

  switch (g_step) {
    case AutoStep::kDriveForward:
      if (now - g_stepStart > 2500) {
        g_step = AutoStep::kPause;
        g_stepStart = now;
        chassis.setVelocity(0.0f, 0.0f);
        Serial.println("[AutonomousDemo] Duraklama");
      }
      break;

    case AutoStep::kPause:
      if (now - g_stepStart > 800) {
        g_step = AutoStep::kTurn;
        g_stepStart = now;
        chassis.turnDegrees(90.0f);
        Serial.println("[AutonomousDemo] 90 derece dönüş başlıyor");
      }
      break;

    case AutoStep::kTurn:
      if (now - g_stepStart > 2200) {
        g_step = AutoStep::kDriveToGoal;
        g_stepStart = now;
        chassis.driveDistance(40.0f);
        Serial.println("[AutonomousDemo] Hedefe son itiş");
      }
      break;

    case AutoStep::kDriveToGoal:
      if (now - g_stepStart > 2000) {
        g_step = AutoStep::kFinished;
        chassis.setVelocity(0.0f, 0.0f);
        Serial.println("[AutonomousDemo] Otonom tamamlandı");
      }
      break;

    case AutoStep::kFinished:
    default:
      chassis.setVelocity(0.0f, 0.0f);
      break;
  }

  chassis.update(now, 20);
  delay(20);
}
