#include <probot.h>

// Bu örnek, joystick'ten gelen bir eksen değerini (-1..1)
// ham motor gücüne (PWM ölçeği -1000..1000) direkt olarak eşler.
// Amaç: Motor bağlantısını test etmek ve yön/invert kontrolünü doğrulamak.
// Donanımınıza uygun IMotor implementasyonunu projeye bağlamanız gerekir.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

// Not: Örnek amacıyla sahte bir IMotor objesi kullandığınızı varsayın.
// Gerçek projede kendi motor sürücünüzü IMotor arayüzüne uyan bir sınıfla bağlamalısınız.
static probot::motor::IMotor* g_motor = nullptr; // kullanıcı doldurmalı
static void* g_owner = (void*)0x1234;

void robotInit() {
  Serial.println("[MotorTest] robotInit: Motor testi başlıyor");
  if (g_motor) {
    if (!g_motor->claim(g_owner)) {
      Serial.println("[MotorTest] Motor claim başarısız!");
    }
  }
}

void robotEnd() {
  if (g_motor) {
    g_motor->setPower(0, g_owner);
    g_motor->release(g_owner);
  }
  Serial.println("[MotorTest] robotEnd: Bitti");
}

void teleopInit() {
  Serial.println("[MotorTest] teleopInit: Joystick ekseni motora güç olarak yazılacak");
}

void teleopLoop() {
  auto s = probot::io::gamepad().read();
  float axis = (s.axisCount > 1) ? s.axes[1] : 0.0f; // Örn: sol çubuk Y
  int16_t power = (int16_t)(axis * 1000.0f);
  if (g_motor) {
    g_motor->setPower(power, g_owner);
  }
  Serial.printf("[MotorTest] axis=%.2f power=%d\n", axis, (int)power);
  delay(50);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 