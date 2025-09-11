#include <probot.h>
#include <probot/io/joystick_api.hpp>

// Bu örnek, Slider nesnesini D-Pad ile 10/20/30/40 cm hedeflerine taşımayı dener.
// Slider, bir ClosedLoopMotor (veya grup) üzerinden konum modunda sürülür.
// Ticks/uzunluk dönüşümü için uygun çarpan ayarlayın.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::controllers::Slider* g_slider = nullptr; // kullanıcı bağlamalı

void robotInit() {
  Serial.println("[SliderTest] robotInit: Slider testi");
  // Örnek: dışarıda oluşturulmuş ClosedLoopMotor üzerinden Slider örneği bağlanmalı.
  // static probot::controllers::ClosedLoopMotor clm(...);
  // static probot::controllers::Slider slider(&clm);
  // slider.setLengthToTicks(100.0f); // 1 cm = 100 tick örneği
  // g_slider = &slider;
}

void robotEnd() {
  Serial.println("[SliderTest] robotEnd: Bitti");
}

void teleopInit() {
  // Mapping değiştirmek için (varsayılan: "logitech-f310"):
  // probot::io::joystick_mapping::setActiveByName("standard");
  // probot::io::joystick_mapping::setActiveByName("logitech-f310");
  // probot::io::joystick_mapping::setActiveByName("axis9-dpad");
  Serial.println("[SliderTest] teleopInit: D-Pad ile 10/20/30/40 cm hedefleri");
}

void teleopLoop() {
  if (!g_slider) { delay(200); return; }
  auto js = probot::io::joystick_api::makeDefault();

  // Basit D-Pad: Up/Down/Left/Right (POV 0/180/270/90)
  int pov = js.getPOV();
  bool up    = (pov == 0);
  bool down  = (pov == 180);
  bool left  = (pov == 270);
  bool right = (pov == 90);

  if (up)    { g_slider->setTargetLength(10.0f); Serial.println("[SliderTest] Hedef: 10 cm"); }
  if (down)  { g_slider->setTargetLength(20.0f); Serial.println("[SliderTest] Hedef: 20 cm"); }
  if (left)  { g_slider->setTargetLength(30.0f); Serial.println("[SliderTest] Hedef: 30 cm"); }
  if (right) { g_slider->setTargetLength(40.0f); Serial.println("[SliderTest] Hedef: 40 cm"); }

  g_slider->update(millis(), 20);
  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 