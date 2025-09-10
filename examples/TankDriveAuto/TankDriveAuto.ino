#include <probot.h>

// Bu örnek, tank sürüş şasesi için basit bir otonom senaryoyu gösterir.
// Sırasıyla: X cm ileri git, Y derece dön, tekrar X cm ileri git gibi bir akış.
// driveDistance ve turnDegrees komutları konum hedeflerini gönderir.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::controllers::ClosedLoopMotor* g_left  = nullptr; // kullanıcı bağlar
static probot::controllers::ClosedLoopMotor* g_right = nullptr; // kullanıcı bağlar
static probot::controllers::BasicTankDrive*  g_chassis = nullptr;

static uint32_t g_step = 0;
static uint32_t g_last_ms = 0;

void robotInit() {
  Serial.println("[TankAuto] robotInit: Otonom");
  // Bağlantı:
  // static probot::controllers::ClosedLoopMotor left(...), right(...);
  // static probot::controllers::BasicTankDrive chassis(&left, &right);
  // chassis.setWheelCircumference(31.4f);
  // chassis.setTrackWidth(25.0f);
  // g_left=&left; g_right=&right; g_chassis=&chassis;
}

void teleopInit() {}
void teleopLoop() { delay(100); }

void autonomousInit() {
  Serial.println("[TankAuto] autonomousInit: Senaryo başlayacak");
  g_step = 0;
  g_last_ms = millis();
}

void autonomousLoop() {
  if (!g_chassis) { delay(1000); return; }
  uint32_t now = millis();
  switch (g_step) {
    case 0:
      Serial.println("[TankAuto] 1) 50 cm ileri");
      g_chassis->driveDistance(50.0f);
      g_step = 1; g_last_ms = now; break;
    case 1:
      if (now - g_last_ms > 3000) { // örnek bekleme süresi
        Serial.println("[TankAuto] 2) 90 derece dön");
        g_chassis->turnDegrees(90.0f);
        g_step = 2; g_last_ms = now;
      }
      break;
    case 2:
      if (now - g_last_ms > 2000) {
        Serial.println("[TankAuto] 3) 50 cm ileri");
        g_chassis->driveDistance(50.0f);
        g_step = 3; g_last_ms = now;
      }
      break;
    default:
      // Bitti: pasif bekle
      break;
  }
  delay(20);
}

void robotEnd() {} 