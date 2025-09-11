#include <probot.h>
#include <probot/io/joystick_api.hpp>

// Bu örnek, web arayüzünden gelen joystick verilerini okuyup
// Serial Monitör'e yazdırır.
// Not: Artık joystick verisi her durumda gönderilir.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.println("[JoystickTest] robotInit: Başlatılıyor");
}

void robotEnd() {
  Serial.println("[JoystickTest] robotEnd: Bitti");
}

void teleopInit() {
  // Mapping değiştirmek için (varsayılan: "logitech-f310"):
  // probot::io::joystick_mapping::setActiveByName("standard");
  // probot::io::joystick_mapping::setActiveByName("logitech-f310");
  // probot::io::joystick_mapping::setActiveByName("axis9-dpad");
  Serial.println("[JoystickTest] teleopInit: Yeni joystick_api ile test");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  Serial.printf("[JoystickTest] seq=%lu axes=%lu buttons=%lu\n",
                (unsigned long)js.getSeq(),
                (unsigned long)js.getAxisCount(),
                (unsigned long)js.getButtonCount());

  Serial.printf("  L(%.2f, %.2f)  R(%.2f, %.2f)  LT=%.0f RT=%.0f  POV=%d\n",
                js.getLeftX(), js.getLeftY(),
                js.getRightX(), js.getRightY(),
                js.getLeftTriggerAxis(), js.getRightTriggerAxis(),
                js.getPOV());

  // Ham eksen/tuş örneği
  if (js.getAxisCount() > 0) {
    Serial.printf("  raw axis[0]=%.2f\n", js.getRawAxis(0));
  }
  if (js.getButtonCount() > 0) {
    Serial.printf("  raw button[0]=%s\n", js.getRawButton(0) ? "ON" : "OFF");
  }

  delay(150);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 