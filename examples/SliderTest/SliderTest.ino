#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/sim/null_motor.hpp>
#include <probot/sim/null_encoder.hpp>
// Not: Donanımı bağlayana kadar NullMotor/NullEncoder kullanabilirsiniz (yer tutucu).
// Gerçek projede bu yer tutucuları gerçek sürücülerle (örn. NFRMotor) değiştirin.
// Desteklenen sürücüler için: https://docs.probotstudio.com/

// Bu örnek, Slider nesnesini D-Pad ile 10/20/30/40 cm hedeflerine taşımayı dener.
// Slider, bir ClosedLoopMotor üzerinden konum modunda sürülür.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static const probot::control::PidConfig kPidCfg{ .kp=0.2f, .ki=0.0f, .kd=0.0f, .out_min=-1.0f, .out_max=1.0f };
static probot::control::PID pid(kPidCfg);
static probot::sensors::NullEncoder encHW;  // yer tutucu
static probot::motor::NullMotor     motHW;  // yer tutucu
static probot::control::ClosedLoopMotor clm(&encHW, &pid, &motHW, 1.0f, 1.0f);
static probot::mechanism::Slider  slider(&clm);

void robotInit() {
  Serial.println("[SliderTest] robotInit: Slider testi");
  // slider.setLengthToTicks(...);
}

void robotEnd() {
  Serial.println("[SliderTest] robotEnd: Bitti");
}

void teleopInit() {
  // Mapping değiştirmek için (varsayılan: "logitech-f310"):
  // probot::io::joystick_mapping::setActiveByName("standard");
  // probot::io::joystick_mapping::setActiveByName("logitech-f310");
  // probot::io::joystick_mapping::setActiveByName("axis9-dpad");
  Serial.println("[SliderTest] teleopInit: D-Pad ile 10/20/30/40 cm hedefleri");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  // Basit D-Pad: Up/Down/Left/Right (POV 0/180/270/90)
  int pov = js.getPOV();
  bool up    = (pov == 0);
  bool down  = (pov == 180);
  bool left  = (pov == 270);
  bool right = (pov == 90);

  if (up)    { slider.setTargetLength(10.0f); Serial.println("[SliderTest] Hedef: 10 cm"); }
  if (down)  { slider.setTargetLength(20.0f); Serial.println("[SliderTest] Hedef: 20 cm"); }
  if (left)  { slider.setTargetLength(30.0f); Serial.println("[SliderTest] Hedef: 30 cm"); }
  if (right) { slider.setTargetLength(40.0f); Serial.println("[SliderTest] Hedef: 40 cm"); }

  slider.update(millis(), 20);
  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 
