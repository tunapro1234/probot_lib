#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// Slider mekanizmasi icin kullanilan VNH motor kontrolcusu pinleri.
static constexpr int PIN_INA = 12;
static constexpr int PIN_INB = 13;
static constexpr int PIN_PWM = 14;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNH5019MotorController motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);


void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(true);
  motor.setPower(0.0f);

  Serial.println("[SliderDemo] robotInit: Slider kontrolü hazır");
}

void robotEnd() {
  motor.setPower(0.0f);
  Serial.println("[SliderDemo] robotEnd: Motor sıfırlandı");
}

void teleopInit() {
  Serial.println("[SliderDemo] teleopInit:"
                 " D-pad yukarı/aşağı slider gücü gönderir");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  int pov = js.getPOV();

  float cmd = 0.0f;
  if (pov == 0) cmd = 0.6f;       // yukarı
  if (pov == 180) cmd = -0.6f;    // aşağı
  motor.setPower(cmd);

  Serial.printf("[SliderDemo] cmd=%.2f out=%.2f\n",
                cmd,
                motor.getPower());

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
    motor.setPower(0.6f);
    stateStart = now;
    state = 1;
  } else if (state == 1 && now - stateStart > 1200) {
    motor.setPower(-0.6f);
    stateStart = now;
    state = 2;
  } else if (state == 2 && now - stateStart > 1200) {
    motor.setPower(0.0f);
    state = 3;
  }

  delay(20);
}
