#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/sim/null_motor.hpp>
#include <probot/sim/null_encoder.hpp>

// Bu örnek, tank sürüş şasesi için basit bir otonom senaryoyu gösterir.
// Sırasıyla: X cm ileri git, Y derece dön, tekrar X cm ileri git gibi bir akış.
// driveDistance ve turnDegrees komutları konum hedeflerini gönderir.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static const probot::control::PidConfig kPidCfg{ .kp=0.2f, .ki=0.0f, .kd=0.0f, .out_min=-1.0f, .out_max=1.0f };
static probot::control::PID pidL(kPidCfg), pidR(kPidCfg);
static probot::sensors::NullEncoder leftEnc, rightEnc;
static probot::motor::NullMotor     leftHW, rightHW;
static probot::controllers::ClosedLoopMotor left(&leftEnc, &pidL, &leftHW, 1.0f, 1.0f);
static probot::controllers::ClosedLoopMotor right(&rightEnc, &pidR, &rightHW, 1.0f, 1.0f);
static probot::controllers::BasicTankDrive  chassis(&left, &right);

static uint32_t g_step = 0;
static uint32_t g_last_ms = 0;

void robotInit() {
  Serial.println("[TankAuto] robotInit");
  // chassis.setWheelCircumference(...); chassis.setTrackWidth(...);
}

void robotEnd() { Serial.println("[TankAuto] robotEnd"); }

void teleopInit() { Serial.println("[TankAuto] teleopInit"); }
void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  float left_axis  = js.getLeftY();
  float right_axis = js.getRightY();
  chassis.setVelocity(left_axis*100.0f, right_axis*100.0f);
  chassis.update(millis(), 20);
  delay(20);
}

void autonomousInit() { Serial.println("[TankAuto] autonomousInit"); }
void autonomousLoop() {
  chassis.driveDistance(50.0f);
  delay(1000);
} 
