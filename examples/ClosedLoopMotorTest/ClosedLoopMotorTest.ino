#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/sim/null_motor.hpp>
#include <probot/sim/null_encoder.hpp>
// Not: Donanımı bağlayana kadar NullMotor/SimEncoder kullanabilirsiniz (yer tutucu).
// Gerçek projede bu yer tutucuları gerçek sürücülerle (örn. NFRMotor) değiştirin.
// Desteklenen sürücüler için: https://docs.probotstudio.com/

// Bu örnek, kapalı çevrim (ClosedLoopMotor) bir motoru joystick ile sürer.
// Sol çubuk Y ekseni hız referansı, D-Pad (ör. butonlar) ise mod geçişi gibi kullanılabilir.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::sensors::SimEncoder encoderHW;   // yer tutucu
static probot::motor::NullMotor     motorHW;     // yer tutucu
static const probot::control::PidConfig kPidCfg{ .kp=0.2f, .ki=0.0f, .kd=0.0f, .kf=0.0f, .out_min=-1.0f, .out_max=1.0f };
static probot::control::PID         pid(kPidCfg);
static probot::control::ClosedLoopMotor clm(&encoderHW, &pid, &motorHW, 1.0f, 1.0f);

void robotInit() {
  Serial.println("[CLMTest] robotInit: ClosedLoopMotor testi");
}

void robotEnd() {
  Serial.println("[CLMTest] robotEnd: Bitti");
}

void teleopInit() {
  // Mapping değiştirmek için (varsayılan: "logitech-f310"):
  // probot::io::joystick_mapping::setActiveByName("standard");
  // probot::io::joystick_mapping::setActiveByName("logitech-f310");
  // probot::io::joystick_mapping::setActiveByName("axis9-dpad");
  Serial.println("[CLMTest] teleopInit: Joystick ile hız/konum kontrolü");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  float axis = js.getLeftY(); // sol çubuk Y
  float vel_ref = axis * 100.0f; // örnek: 100 birim/s maksimum hız
  clm.setSetpoint(vel_ref, probot::control::ControlType::kVelocity);
  clm.update(millis(), 20);
  Serial.printf("[CLMTest] vel_ref=%.2f\n", vel_ref);
  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 
