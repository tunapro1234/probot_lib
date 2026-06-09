// ServoTest - Sol joystick Y ekseni ile servo kontrolü.
//
// Bağlantı:
//   - Servo sinyal teli -> SERVO_PIN (varsayılan GPIO 4)
//   - Servo güç (kırmızı/kahverengi) -> AYRI 5-6V kaynak (BEC). ESP32'nin
//     5V/3V3 pininden servo BESLEMEYİN — WiFi anlık akım çekişleri servoyu
//     titretir. Toprakları (GND) ortak bağlayın.

#define PROBOT_WIFI_AP_SSID     "Probot"
#define PROBOT_WIFI_AP_PASSWORD "Probot1234"
#define PROBOT_WIFI_AP_CHANNEL  1

#include <probot.h>

#define SERVO_PIN 4

probot::devices::Servo servo;

void robotInit() {
  servo.attach(SERVO_PIN);   // 500-2500us, 50Hz
}

void robotEnd() {
  servo.detach();
}

void teleopInit() {}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  // Sol Y ekseni (-1..+1) -> 0-180 derece
  float angle = (js.getLeftY() + 1.0f) * 90.0f;

  // A basılıyken ortala
  if (js.getA()) angle = 90.0f;

  servo.write(angle);

  probot::clearTelemetry();
  probot::printf("Servo: %.0f derece\n", angle);

  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(100); }
