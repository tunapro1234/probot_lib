#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/test/test_motor.hpp>
#include <probot/devices/motors/motor_handle.hpp>
// Not: Donanımı bağlayana kadar NullMotor kullanabilirsiniz (yer tutucu).
// Gerçek projede bu yer tutucuyu gerçek sürücülerle (örn. NFRMotor) değiştirin.
// Desteklenen sürücüler için: https://docs.probotstudio.com/

// Bu örnek, joystick'ten gelen bir eksen değerini (-1..1)
// ham motor gücüne (normalize -1..1) direkt olarak eşler.
// Amaç: Motor bağlantısını test etmek ve yön/invert kontrolünü doğrulamak.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::motor::NullMotor motorHW;           // yer tutucu; gerçek sürücü ile değiştirin
static probot::motor::MotorHandle motor(motorHW);  // sahipliği içeride yönetir

void robotInit() {
  Serial.println("[MotorTest] robotInit: Motor testi başlıyor");
}

void robotEnd() {
  motor.setPower(0.0f);
  Serial.println("[MotorTest] robotEnd: Bitti");
}

void teleopInit() {
  // Mapping değiştirmek için (varsayılan: "logitech-f310"):
  // probot::io::joystick_mapping::setActiveByName("standard");
  // probot::io::joystick_mapping::setActiveByName("logitech-f310");
  // probot::io::joystick_mapping::setActiveByName("axis9-dpad");
  Serial.println("[MotorTest] teleopInit: Joystick ekseni motora güç olarak yazılacak");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  float axis = js.getLeftY(); // Örn: sol çubuk Y
  motor.setPower(axis);
  Serial.printf("[MotorTest] axis=%.2f power=%.2f\n", axis, axis);
  delay(50);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 
