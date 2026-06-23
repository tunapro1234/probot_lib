// ServoTest - Sol joystick Y ekseni ile servo kontrolü.
//
// probot bir servo SINIFI SAĞLAMAZ — kütüphane iletişim + yaşam döngüsü
// katmanıdır; çıkış donanımını sen sürersin. Aşağıdaki kalıp titreme-güvenli:
//
//   1) Servo 50 Hz / 14-bit bir LEDC sinyali ister (20 ms çerçeve içinde
//      0.5-2.5 ms darbe). analogWrite ~1 kHz verir, servoya UYMAZ.
//   2) Timer çakışması: LEDC'de 4 timer var; aynı timer'daki kanallar aynı
//      frekansı paylaşır. analogWrite (motorlar) kanalları ALTTAN (0,1,2…)
//      kullanır; servoya YÜKSEK bir kanal (7) verirsek aynı timer'a düşmez
//      → jitter (titreme nedeni #1) yapısal olarak olmaz.
//
// Bağlantı:
//   - Servo sinyal teli -> SERVO_PIN (GPIO 4)
//   - Servo gücü -> AYRI 5-6 V kaynak (BEC). ESP32 pininden BESLEMEYİN —
//     WiFi anlık akım çekişleri servoyu titretir. GND'leri ortak bağlayın.

#define PROBOT_WIFI_AP_SSID     "Probot"
#define PROBOT_WIFI_AP_PASSWORD "Probot1234"
#define PROBOT_WIFI_AP_CHANNEL  1

#include <probot.h>

#define SERVO_PIN       4
#define SERVO_LEDC_CH   7        // yüksek kanal → motor PWM'iyle çakışmaz
#define SERVO_FREQ_HZ   50
#define SERVO_RES_BITS  14
static const uint32_t SERVO_PERIOD_US = 1000000UL / SERVO_FREQ_HZ;     // 20000
static const uint32_t SERVO_DUTY_MAX  = (1UL << SERVO_RES_BITS) - 1;   // 16383

void servoWriteUs(uint16_t us) {
  if (us < 500)  us = 500;
  if (us > 2500) us = 2500;
  ledcWrite(SERVO_PIN, (uint32_t)((uint64_t)us * SERVO_DUTY_MAX / SERVO_PERIOD_US));
}

void servoWriteAngle(float deg) {
  if (deg < 0)   deg = 0;
  if (deg > 180) deg = 180;
  servoWriteUs((uint16_t)(500 + (deg / 180.0f) * 2000));   // 0-180° -> 500-2500 µs
}

void robotInit() {
  // 50 Hz / 14-bit on a fixed high channel. No pulse until the first write
  // -> the servo doesn't jump on boot. A fixed channel means re-running
  // robotInit on each Init press just re-uses it (no channel exhaustion).
  ledcAttachChannel(SERVO_PIN, SERVO_FREQ_HZ, SERVO_RES_BITS, SERVO_LEDC_CH);
}

void robotEnd() {
  ledcWrite(SERVO_PIN, 0);   // stop the pulse train — safe state
}

void teleopInit() {}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float angle = (js.getLeftY() + 1.0f) * 90.0f;   // -1..+1 -> 0-180
  if (js.getA()) angle = 90.0f;                    // A centers
  servoWriteAngle(angle);

  probot::clearTelemetry();
  probot::printf("Servo: %.0f derece\n", angle);
}

void autonomousInit() {}
void autonomousLoop() {}
