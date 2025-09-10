#include <probot.h>

// Bu örnek, web arayüzünden gelen joystick verilerini okuyup
// Serial Monitör'e yazdırır. Amacımız öğrencilerin joystick'in
// doğru şekilde bağlanıp bağlanmadığını hızlıca doğrulamasıdır.
// Not: Web arayüzünde joystick verisi "Stop" durumunda gönderilir.
// UI'de Init→Start→Stop sırasını izleyin.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.println("[JoystickTest] robotInit: Başlatılıyor");
}

void robotEnd() {
  Serial.println("[JoystickTest] robotEnd: Bitti");
}

void teleopInit() {
  Serial.println("[JoystickTest] teleopInit: Joystick verileri yazdırılacak");
}

void teleopLoop() {
  auto s = probot::io::gamepad().read();
  Serial.printf("[JoystickTest] seq=%lu ms=%lu axes=%lu buttons=%lu\n",
                (unsigned long)s.seq, (unsigned long)s.ms,
                (unsigned long)s.axisCount, (unsigned long)s.buttonCount);

  // Eksenleri yazdır
  for (uint32_t i = 0; i < s.axisCount; ++i) {
    Serial.printf("  axis[%lu]=%.2f\n", (unsigned long)i, s.axes[i]);
  }

  // Butonları yazdır
  for (uint32_t i = 0; i < s.buttonCount; ++i) {
    Serial.printf("  button[%lu]=%s\n", (unsigned long)i, s.buttons[i] ? "BASILI" : "YOK");
  }

  delay(200);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 