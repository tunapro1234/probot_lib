#include <probot.h>

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
  Serial.println("[SliderTest] teleopInit: D-Pad ile 10/20/30/40 cm hedefleri");
}

void teleopLoop() {
  if (!g_slider) { delay(200); return; }
  auto s = probot::io::gamepad().read();

  // D-Pad buton indeksleri: örnek değerler (UI kaynaklı olabilir)
  // Up=4, Down=5, Left=6, Right=7 mapping'ini TunaGamepad DPad kısmı üzerinden paylaştık.
  bool up    = (s.buttonCount>4) ? s.buttons[4] : false;
  bool down  = (s.buttonCount>5) ? s.buttons[5] : false;
  bool left  = (s.buttonCount>6) ? s.buttons[6] : false;
  bool right = (s.buttonCount>7) ? s.buttons[7] : false;

  if (up)    { g_slider->setTargetLength(10.0f); Serial.println("[SliderTest] Hedef: 10 cm"); }
  if (down)  { g_slider->setTargetLength(20.0f); Serial.println("[SliderTest] Hedef: 20 cm"); }
  if (left)  { g_slider->setTargetLength(30.0f); Serial.println("[SliderTest] Hedef: 30 cm"); }
  if (right) { g_slider->setTargetLength(40.0f); Serial.println("[SliderTest] Hedef: 40 cm"); }

  g_slider->update(millis(), 20);
  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 