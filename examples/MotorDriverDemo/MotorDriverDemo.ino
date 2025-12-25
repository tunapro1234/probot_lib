#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_driver.hpp>

// Boardoza VNH sürücüsünü kullanırken kendi pinlerinizi mutlaka kontrol edin.
static constexpr int PIN_INA = 39;
static constexpr int PIN_INB = 40;
static constexpr int PIN_PWM = 41;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNH5019MotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(false);  // sürücü serbest bırakıldı, coast modunda
  motor.setInverted(false);

  Serial.println("[MotorDriverDemo] robotInit: IMotorController arayüzünü test etmek için hazır");
}

void robotEnd() {
  motor.setPower(0.0f);
  Serial.println("[MotorDriverDemo] robotEnd: Motor durduruldu");
}

void teleopInit() {
  Serial.println("[MotorDriverDemo] teleopInit:"
                 " sol eksen gücü, sağ tetik ise yön tersleme yapar");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  // Sol çubuktan ham güç, sağ tetikten tersleme isteği okuyoruz.
  float power = js.getLeftY();
  bool invert = js.getRightTriggerAxis() > 0.5f;

  motor.setInverted(invert);

  motor.setPower(power);

  Serial.printf("[MotorDriverDemo] power=%.2f invert=%d motorOut=%.2f\n",
                power, invert ? 1 : 0, motor.getPower());

  delay(40);
}

void autonomousInit() {
  Serial.println("[MotorDriverDemo] autonomousInit: Motoru yavaşça durduruyoruz");
}

void autonomousLoop() {
  motor.setPower(0.0f);
  delay(20);
}
