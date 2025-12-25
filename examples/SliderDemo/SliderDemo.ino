#include <probot.h>
#include <probot/test/null_encoder.hpp>
#include <probot/io/joystick_api.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>
#include <probot/mechanism/slider.hpp>

// Slider mekanizması için kullanılan VNH sürücü pinleri.
static constexpr int PIN_INA = 12;
static constexpr int PIN_INB = 13;
static constexpr int PIN_PWM = 14;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNHMotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);
static probot::test::NullEncoder          encoder;
static const probot::control::PidConfig      kPositionPid{.kp = 0.5f, .ki = 0.0f, .kd = 0.0f, .kf = 0.0f,
                                                          .out_min = -1.0f, .out_max = 1.0f};
static probot::control::PID                  pid(kPositionPid);
static probot::control::ClosedLoopMotor      controller(&encoder, &pid, &motor, 1.0f, 1.0f);
static probot::mechanism::Slider             slider(&controller);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(true);

  controller.selectDefaultSlot(probot::control::ControlType::kPosition, 0);
  controller.setPidSlotConfig(0, kPositionPid);
  controller.setSetpoint(0.0f, probot::control::ControlType::kPosition);

  slider.setLengthToTicks(200.0f);     // 1 cm için 200 encoder tıkı varsayalım
  slider.setLengthLimits(0.0f, 50.0f); // 0 - 50 cm arası güvenli bölge

  Serial.println("[SliderDemo] robotInit: Slider kontrolü hazır");
}

void robotEnd() {
  controller.setPowerDirect(0.0f);
  Serial.println("[SliderDemo] robotEnd: Motor sıfırlandı");
}

void teleopInit() {
  Serial.println("[SliderDemo] teleopInit:"
                 " D-pad ile 0/15/30/45 cm hedefleri seçilir");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  int pov = js.getPOV();

  if (pov == 0)   slider.setTargetLength(45.0f);  // yukarı
  if (pov == 90)  slider.setTargetLength(30.0f);  // sağ
  if (pov == 180) slider.setTargetLength(0.0f);   // aşağı
  if (pov == 270) slider.setTargetLength(15.0f);  // sol

  slider.update(millis(), 20);
  controller.update(millis(), 20);

  Serial.printf("[SliderDemo] hedef=%.1f cm ölçüm=%.1f cm çıkış=%.2f\n",
                slider.getTargetLength(),
                slider.getCurrentLength(),
                controller.lastOutput());

  delay(20);
}

void autonomousInit() {
  Serial.println("[SliderDemo] autonomousInit: 3 adımda preset dolaşımı");
}

void autonomousLoop() {
  static uint32_t stateStart = millis();
  static int state = 0;
  uint32_t now = millis();

  if (state == 0) {
    slider.setTargetLength(10.0f);
    stateStart = now;
    state = 1;
  } else if (state == 1 && now - stateStart > 1500) {
    slider.setTargetLength(40.0f);
    stateStart = now;
    state = 2;
  } else if (state == 2 && now - stateStart > 1500) {
    slider.setTargetLength(0.0f);
    state = 3;
  }

  slider.update(now, 20);
  controller.update(now, 20);
  delay(20);
}
