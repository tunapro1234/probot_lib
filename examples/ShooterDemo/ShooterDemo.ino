#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// Shooter tekeri için Boardoza VNH pin konfigürasyonu.
static constexpr int PIN_INA = 15;
static constexpr int PIN_INB = 16;
static constexpr int PIN_PWM = 17;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNH5019MotorController motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);


void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(false); // shooter çarkında coast tercihi yapılabilir
  motor.setPower(0.0f);

  Serial.println("[ShooterDemo] robotInit: Shooter kontrolü başlatıldı");
}

void robotEnd() {
  motor.setPower(0.0f);
  Serial.println("[ShooterDemo] robotEnd: Teker kapatıldı");
}

void teleopInit() {
  Serial.println("[ShooterDemo] teleopInit:"
                 " sağ tetik hızlandırır, sol tetik fren yapar");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float accel = js.getRightTriggerAxis(); // 0..1
  float brake = js.getLeftTriggerAxis();  // 0..1

  float power = accel;                   // 0..1 açık çevrim güç
  if (brake > 0.2f) power = 0.0f;        // fren tetiklendiğinde durdur

  motor.setPower(power);

  Serial.printf("[ShooterDemo] power=%.2f out=%.2f\n",
                power,
                motor.getPower());

  delay(20);
}

void autonomousInit() {
  Serial.println("[ShooterDemo] autonomousInit: 2 saniye spool, sonra durdur");
  motor.setPower(0.8f);
}

void autonomousLoop() {
  static uint32_t start = millis();
  if (millis() - start > 2000) {
    motor.setPower(0.0f);
  }
  delay(20);
}
