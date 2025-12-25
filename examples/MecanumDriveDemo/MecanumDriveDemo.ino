#include <probot.h>
#include <probot/test/null_encoder.hpp>
#include <probot/io/joystick_api.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>
#include <probot/chassis/simple_mecanum.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>

// Mecanum sürüş için dört motorun pin atamaları.
struct MotorPins {
  int ina, inb, pwm, ena, enb;
};

static constexpr MotorPins PINS_FL{18, 19, 20, -1, -1};
static constexpr MotorPins PINS_FR{21, 22, 23, -1, -1};
static constexpr MotorPins PINS_RL{24, 25, 26, -1, -1};
static constexpr MotorPins PINS_RR{27, 28, 29, -1, -1};

static probot::motor::BoardozaVNHMotorDriver drvFL(PINS_FL.ina, PINS_FL.inb, PINS_FL.pwm, PINS_FL.ena, PINS_FL.enb);
static probot::motor::BoardozaVNHMotorDriver drvFR(PINS_FR.ina, PINS_FR.inb, PINS_FR.pwm, PINS_FR.ena, PINS_FR.enb);
static probot::motor::BoardozaVNHMotorDriver drvRL(PINS_RL.ina, PINS_RL.inb, PINS_RL.pwm, PINS_RL.ena, PINS_RL.enb);
static probot::motor::BoardozaVNHMotorDriver drvRR(PINS_RR.ina, PINS_RR.inb, PINS_RR.pwm, PINS_RR.ena, PINS_RR.enb);

static probot::test::NullEncoder encFL;
static probot::test::NullEncoder encFR;
static probot::test::NullEncoder encRL;
static probot::test::NullEncoder encRR;

static const probot::control::PidConfig kPid{.kp = 0.30f, .ki = 0.01f, .kd = 0.0f, .kf = 0.0f,
                                             .out_min = -1.0f, .out_max = 1.0f};

static probot::control::PID pidFL(kPid);
static probot::control::PID pidFR(kPid);
static probot::control::PID pidRL(kPid);
static probot::control::PID pidRR(kPid);

static probot::control::ClosedLoopMotor motorFL(&encFL, &pidFL, &drvFL, 1.0f, 1.0f);
static probot::control::ClosedLoopMotor motorFR(&encFR, &pidFR, &drvFR, 1.0f, 1.0f);
static probot::control::ClosedLoopMotor motorRL(&encRL, &pidRL, &drvRL, 1.0f, 1.0f);
static probot::control::ClosedLoopMotor motorRR(&encRR, &pidRR, &drvRR, 1.0f, 1.0f);

static probot::chassis::SimpleMecanumDrive mecanum(&motorFL, &motorFR, &motorRL, &motorRR);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  drvFL.begin(); drvFR.begin(); drvRL.begin(); drvRR.begin();
  drvFL.setBrakeMode(true);
  drvFR.setBrakeMode(true);
  drvRL.setBrakeMode(true);
  drvRR.setBrakeMode(true);

  mecanum.setInverted(false, true, false, true); // sağ taraf ters kabloluysa düzelt

  Serial.println("[MecanumDriveDemo] robotInit: Mecanum sürüşe hazır");
}

void robotEnd() {
  mecanum.stop();
  Serial.println("[MecanumDriveDemo] robotEnd: Motorlar kapatıldı");
}

void teleopInit() {
  Serial.println("[MecanumDriveDemo] teleopInit:"
                 " sol çubuk Y ileri-geri, X yan, sağ X dönüş");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float vx = js.getLeftY();    // ileri geri
  float vy = js.getLeftX();    // yan hareket
  float omega = js.getRightX();// dönüş

  mecanum.driveCartesian(vx, vy, omega);

  Serial.printf("[MecanumDriveDemo] vx=%.2f vy=%.2f w=%.2f fl=%.2f fr=%.2f rl=%.2f rr=%.2f\n",
                vx, vy, omega,
                motorFL.lastOutput(), motorFR.lastOutput(),
                motorRL.lastOutput(), motorRR.lastOutput());

  delay(20);
}

void autonomousInit() {
  Serial.println("[MecanumDriveDemo] autonomousInit: kare rota denemesi");
}

void autonomousLoop() {
  static uint32_t start = millis();
  uint32_t elapsed = millis() - start;
  float vx = 0.0f, vy = 0.0f, omega = 0.0f;

  if (elapsed < 2000) {
    vx = 0.6f; // ileri
  } else if (elapsed < 4000) {
    vy = 0.6f; // sağa kay
  } else if (elapsed < 6000) {
    vx = -0.6f; // geri
  } else if (elapsed < 8000) {
    vy = -0.6f; // sola kay
  } else {
    start = millis();
  }

  mecanum.driveCartesian(vx, vy, omega);
  delay(20);
}
