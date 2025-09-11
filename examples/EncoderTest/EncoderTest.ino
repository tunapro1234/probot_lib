#include <probot.h>
#include <probot/sim/null_encoder.hpp>

// Bu örnek, enkoder değerlerini (tik ve hız) Serial Monitör'e yazdırır.
// Donanımı bağlayana kadar NullEncoder kullanabilirsiniz (yer tutucu).
// Gerçek projede bu yer tutucuyu gerçek enkoder sürücüsüyle değiştirin.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::sensors::NullEncoder encoderHW; // yer tutucu; gerçek sürücü ile değiştirin

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
  int32_t ticks = encoderHW.readTicks();
  int32_t tps   = encoderHW.readTicksPerSecond();
  Serial.printf("[EncoderTest] ticks=%ld tps=%ld\n", (long)ticks, (long)tps);
  delay(200);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 