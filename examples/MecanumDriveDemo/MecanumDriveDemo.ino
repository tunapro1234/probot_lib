#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/command/scheduler.hpp>
#include <probot/command/examples/mecanum_drive.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_driver.hpp>

// Mecanum sürüş için dört motorun pin atamaları.
struct MotorPins {
  int ina, inb, pwm, ena, enb;
};

static constexpr MotorPins PINS_FL{18, 19, 20, -1, -1};
static constexpr MotorPins PINS_FR{21, 22, 23, -1, -1};
static constexpr MotorPins PINS_RL{24, 25, 26, -1, -1};
static constexpr MotorPins PINS_RR{27, 28, 29, -1, -1};

static probot::motor::BoardozaVNH5019MotorDriver drvFL(PINS_FL.ina, PINS_FL.inb, PINS_FL.pwm, PINS_FL.ena, PINS_FL.enb);
static probot::motor::BoardozaVNH5019MotorDriver drvFR(PINS_FR.ina, PINS_FR.inb, PINS_FR.pwm, PINS_FR.ena, PINS_FR.enb);
static probot::motor::BoardozaVNH5019MotorDriver drvRL(PINS_RL.ina, PINS_RL.inb, PINS_RL.pwm, PINS_RL.ena, PINS_RL.enb);
static probot::motor::BoardozaVNH5019MotorDriver drvRR(PINS_RR.ina, PINS_RR.inb, PINS_RR.pwm, PINS_RR.ena, PINS_RR.enb);

static probot::command::examples::MecanumDrive mecanum(&drvFL, &drvFR, &drvRL, &drvRR);


void robotInit() {
  Serial.begin(115200);
  delay(100);

  drvFL.begin(); drvFR.begin(); drvRL.begin(); drvRR.begin();
  drvFL.setBrakeMode(true);
  drvFR.setBrakeMode(true);
  drvRL.setBrakeMode(true);
  drvRR.setBrakeMode(true);

  mecanum.setInverted(false, true, false, true); // sağ taraf ters kabloluysa düzelt
  mecanum.setWheelBase(30.0f);
  mecanum.setTrackWidth(28.0f);

  probot::command::scheduler::attach(&mecanum);
  Serial.println("[MecanumDriveDemo] robotInit: Mecanum sürüşe hazır");
}

void robotEnd() {
  probot::command::scheduler::detach(&mecanum);
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

  mecanum.drivePower(vx, vy, omega);

  Serial.printf("[MecanumDriveDemo] vx=%.2f vy=%.2f w=%.2f fl=%.2f fr=%.2f rl=%.2f rr=%.2f\n",
                vx, vy, omega,
                drvFL.getPower(), drvFR.getPower(),
                drvRL.getPower(), drvRR.getPower());

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

  mecanum.drivePower(vx, vy, omega);
  delay(20);
}
