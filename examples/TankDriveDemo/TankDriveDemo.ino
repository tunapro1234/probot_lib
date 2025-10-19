#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>
#include <probot/chassis/basic_tank_drive.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>

// Tank şasi için iki adet VNH sürücünün pin eşlemesi (örnek değerler).
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

static probot::motor::BoardozaVNHMotorDriver leftDriver(LEFT_INA, LEFT_INB, LEFT_PWM, LEFT_ENA, LEFT_ENB);
static probot::motor::BoardozaVNHMotorDriver rightDriver(RIGHT_INA, RIGHT_INB, RIGHT_PWM, RIGHT_ENA, RIGHT_ENB);
static probot::sensors::NullEncoder          leftEncoder;
static probot::sensors::NullEncoder          rightEncoder;

static const probot::control::PidConfig      kPidCfg{.kp = 0.28f, .ki = 0.01f, .kd = 0.0f, .kf = 0.0f,
                                                     .out_min = -1.0f, .out_max = 1.0f};
static probot::control::PID                  pidLeft(kPidCfg);
static probot::control::PID                  pidRight(kPidCfg);
static probot::control::ClosedLoopMotor      motorLeft(&leftEncoder, &pidLeft, &leftDriver, 1.0f, 1.0f);
static probot::control::ClosedLoopMotor      motorRight(&rightEncoder, &pidRight, &rightDriver, 1.0f, 1.0f);
static probot::drive::BasicTankDrive         chassis(&motorLeft, &motorRight);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  leftDriver.begin();
  rightDriver.begin();
  leftDriver.setBrakeMode(true);
  rightDriver.setBrakeMode(true);

  motorLeft.setTimeoutMs(0);
  motorRight.setTimeoutMs(0);

  chassis.setWheelCircumference(31.4f);  // örnek teker çapı ayarı
  chassis.setTrackWidth(28.0f);          // örnek şasi genişliği

  Serial.println("[TankDriveDemo] robotInit: Tank şasi hazır");
}

void robotEnd() {
  chassis.setVelocity(0.0f, 0.0f);
  motorLeft.setPowerDirect(0.0f);
  motorRight.setPowerDirect(0.0f);
  Serial.println("[TankDriveDemo] robotEnd: Motorlar kapandı");
}

void teleopInit() {
  Serial.println("[TankDriveDemo] teleopInit: Sol/Sağ joystick ile tank sürüşü");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float leftCmd = js.getLeftY() * 120.0f;   // cm/s cinsinden hedef hız
  float rightCmd = js.getRightY() * 120.0f;

  chassis.setVelocity(leftCmd, rightCmd);

  uint32_t now = millis();
  chassis.update(now, 20);

  Serial.printf("[TankDriveDemo] left=%.1f right=%.1f outL=%.2f outR=%.2f\n",
                leftCmd, rightCmd,
                motorLeft.lastOutput(), motorRight.lastOutput());

  delay(20);
}

void autonomousInit() {
  Serial.println("[TankDriveDemo] autonomousInit: 3 adımda ilerleme testi");
}

void autonomousLoop() {
  static uint32_t stateStart = millis();
  static int state = 0;

  uint32_t now = millis();
  switch (state) {
    case 0: // 1 metre ileri
      chassis.driveDistance(100.0f);
      stateStart = now;
      state = 1;
      break;
    case 1:
      if (now - stateStart > 3000) {
        chassis.turnDegrees(90.0f);
        stateStart = now;
        state = 2;
      }
      break;
    case 2:
      if (now - stateStart > 2500) {
        chassis.driveDistance(50.0f);
        state = 3;
      }
      break;
    default:
      chassis.setVelocity(0.0f, 0.0f);
      break;
  }

  chassis.update(now, 20);
  delay(20);
}
