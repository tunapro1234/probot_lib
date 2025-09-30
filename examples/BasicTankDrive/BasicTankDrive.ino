#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/sim/null_motor.hpp>
#include <probot/sim/null_encoder.hpp>
// Not: Donanımı bağlayana kadar NullMotor/SimEncoder kullanabilirsiniz (yer tutucu).
// Gerçek projede bu yer tutucuları gerçek sürücülerle (örn. NFRMotor) değiştirin.
// Desteklenen sürücüler için: https://docs.probotstudio.com/

// Bu örnek, tank şasi (BasicTankDrive) ile teleop sürüşünü gösterir.
// Sol çubuk Y sol teker, sağ çubuk Y sağ teker hızını belirler.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static const probot::control::PidConfig kPidCfg{ .kp=0.2f, .ki=0.0f, .kd=0.0f, .kf=0.0f, .out_min=-1.0f, .out_max=1.0f };
static probot::control::PID pidL(kPidCfg), pidR(kPidCfg);
static probot::sensors::SimEncoder leftEnc, rightEnc;   // yer tutucu
static probot::motor::NullMotor     leftHW, rightHW;     // yer tutucu
static probot::control::ClosedLoopMotor left(&leftEnc, &pidL, &leftHW, 1.0f, 1.0f);
static probot::control::ClosedLoopMotor right(&rightEnc, &pidR, &rightHW, 1.0f, 1.0f);
static probot::drive::BasicTankDrive  chassis(&left, &right);

void robotInit() {
  Serial.println("[TankTeleop] robotInit: Tank sürüşü");
  // Örnek bağlama (gerçek sürücüler ile değiştirin):
  // chassis.setWheelCircumference(31.4f);
  // chassis.setTrackWidth(25.0f);
}

void robotEnd() {
  Serial.println("[TankTeleop] robotEnd: Bitti");
}

void teleopInit() {
  // Mapping değiştirmek için (varsayılan: "logitech-f310"):
  // probot::io::joystick_mapping::setActiveByName("standard");
  // probot::io::joystick_mapping::setActiveByName("logitech-f310");
  // probot::io::joystick_mapping::setActiveByName("axis9-dpad");
  Serial.println("[TankTeleop] teleopInit: Joystick ile tank sürüş");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  float left_axis  = js.getLeftY();  // sol Y
  float right_axis = js.getRightY(); // sağ Y

  float max_vel = 100.0f; // birim/s örnek
  chassis.setVelocity(left_axis*max_vel, right_axis*max_vel);
  chassis.update(millis(), 20);
  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 
