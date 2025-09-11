#include <probot.h>
#include <probot/io/joystick_api.hpp>

// Bu örnek, tank şasi (BasicTankDrive) ile teleop sürüşünü gösterir.
// Sol çubuk Y sol teker, sağ çubuk Y sağ teker hızını belirler.
// İki ClosedLoopMotor (sol ve sağ) ve şasi parametreleri (teker çevresi, iz genişliği) ayarlanmalıdır.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::controllers::ClosedLoopMotor* g_left  = nullptr; // kullanıcı bağlar
static probot::controllers::ClosedLoopMotor* g_right = nullptr; // kullanıcı bağlar
static probot::controllers::BasicTankDrive*  g_chassis = nullptr;

void robotInit() {
  Serial.println("[TankTeleop] robotInit: Tank sürüşü");
  // Örnek bağlama:
  // static probot::controllers::ClosedLoopMotor left(...), right(...);
  // static probot::controllers::BasicTankDrive chassis(&left, &right);
  // chassis.setWheelCircumference(31.4f); // örn. 10 cm çap → 31.4 cm çevre
  // chassis.setTrackWidth(25.0f); // şasi iz genişliği cm
  // g_left=&left; g_right=&right; g_chassis=&chassis;
}

void robotEnd() {
  Serial.println("[TankTeleop] robotEnd: Bitti");
}

void teleopInit() {
  // Mapping değiştirmek için (varsayılan: "logitech-f310"):
  // probot::io::joystick_mapping::setActiveByName("standard");
  // probot::io::joystick_mapping::setActiveByName("logitech-f310");
  // probot::io::joystick_mapping::setActiveByName("axis9-dpad");
  Serial.println("[TankTeleop] teleopInit: Joystick ile tank sürüş");
}

void teleopLoop() {
  if (!g_chassis) { delay(50); return; }
  auto js = probot::io::joystick_api::makeDefault();
  float left_axis  = js.getLeftY();  // sol Y
  float right_axis = js.getRightY(); // sağ Y

  float max_vel = 100.0f; // birim/s örnek
  g_chassis->setVelocity(left_axis*max_vel, right_axis*max_vel);
  g_chassis->update(millis(), 20);
  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 