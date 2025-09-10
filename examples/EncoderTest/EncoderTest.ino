#include <probot.h>

// Bu örnek, enkoder değerlerini (tik ve hız) Serial Monitör'e yazdırır.
// Enkoderinizi IEncoder arayüzünü uygulayan bir sınıfla sisteme bağlamalısınız.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::sensors::IEncoder* g_encoder = nullptr; // kullanıcı doldurmalı

void robotInit() {
  Serial.println("[EncoderTest] robotInit: Enkoder testi");
}

void robotEnd() {
  Serial.println("[EncoderTest] robotEnd: Bitti");
}

void teleopInit() {
  Serial.println("[EncoderTest] teleopInit: Enkoder değerleri yazdırılacak");
}

void teleopLoop() {
  int32_t ticks = g_encoder ? g_encoder->readTicks() : 0;
  int32_t tps   = g_encoder ? g_encoder->readTicksPerSecond() : 0;
  Serial.printf("[EncoderTest] ticks=%ld tps=%ld\n", (long)ticks, (long)tps);
  delay(200);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 