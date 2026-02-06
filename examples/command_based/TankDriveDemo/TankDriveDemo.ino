#define PROBOT_WIFI_AP_SSID     "ProBot"
#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"
#define PROBOT_WIFI_AP_CHANNEL  3

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/command.hpp>  // Includes scheduler, command, subsystem, command_group
#include <probot/command/examples/tank_drive.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// Tank sasi icin iki adet VNH motor kontrolcusunun pin eslemesi (ornek degerler).
static constexpr int LEFT_INA = 1;
static constexpr int LEFT_INB = 2;
static constexpr int LEFT_PWM = 3;
static constexpr int LEFT_ENA = -1;
static constexpr int LEFT_ENB = -1;

static constexpr int RIGHT_INA = 4;
static constexpr int RIGHT_INB = 5;
static constexpr int RIGHT_PWM = 6;
static constexpr int RIGHT_ENA = -1;
static constexpr int RIGHT_ENB = -1;

static probot::motor::BoardozaVNH5019MotorController leftMotor(LEFT_INA, LEFT_INB, LEFT_PWM, LEFT_ENA, LEFT_ENB);
static probot::motor::BoardozaVNH5019MotorController rightMotor(RIGHT_INA, RIGHT_INB, RIGHT_PWM, RIGHT_ENA, RIGHT_ENB);
static probot::command::examples::TankDrive            chassis(&leftMotor, &rightMotor);

using Scheduler = probot::command::Scheduler;

void robotInit() {
  Serial.begin(115200);
  delay(100);

  leftMotor.begin();
  rightMotor.begin();
  leftMotor.setBrakeMode(true);
  rightMotor.setBrakeMode(true);

  // NOTE: Closed-loop/odometry helpers are disabled (not tested yet).
  // chassis.setWheelRadius(31.4f / (2.0f * 3.1415926535f)); // cm cinsinden yaricap
  // chassis.setTrackWidth(28.0f);                           // cm

  // Yeni API: Subsystem'i scheduler'a kaydet
  Scheduler::instance().registerSubsystem(&chassis);
  Serial.println("[TankDriveDemo] robotInit: Tank sasi hazir");
}

void robotEnd() {
  Scheduler::instance().unregisterSubsystem(&chassis);
  chassis.stop();
  Serial.println("[TankDriveDemo] robotEnd: Motorlar kapandi");
}

void teleopInit() {
  Serial.println("[TankDriveDemo] teleopInit: Sol/Sag joystick ile tank surusu");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float leftAxis = js.getLeftY();
  float rightAxis = js.getRightY();
  chassis.drivePower(leftAxis, rightAxis);

  Serial.printf("[TankDriveDemo] left=%.2f right=%.2f outL=%.2f outR=%.2f\n",
                leftAxis, rightAxis,
                leftMotor.getPower(), rightMotor.getPower());

  delay(20);
}

void autonomousInit() {
  Serial.println("[TankDriveDemo] autonomousInit: 3 adimda ilerleme testi");
}

void autonomousLoop() {
  static uint32_t stateStart = millis();
  static int state = 0;

  uint32_t now = millis();
  switch (state) {
    case 0: // 1 metre ileri
      chassis.drivePower(0.5f, 0.5f);
      if (now - stateStart > 3000) {
        stateStart = now;
        state = 1;
      }
      break;
    case 1:
      chassis.drivePower(0.4f, -0.4f);
      if (now - stateStart > 2000) {
        stateStart = now;
        state = 2;
      }
      break;
    case 2:
      chassis.drivePower(0.5f, 0.5f);
      if (now - stateStart > 2000) {
        state = 3;
      }
      break;
    default:
      chassis.stop();
      break;
  }
  delay(20);
}
