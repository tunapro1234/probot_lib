#include <probot.h>

// Bu örnek, daha tamamlanmış bir robot iskeleti gösterir:
// - TankDrive şasi (teleop + otonom)
// - Intake (içeri alma) ve Shooter (fırlatma)
// - İki adet Slider ile tırmanma mekanizması (aç/kapa senaryosu)
// Bu örnekte donanım bağlama kısmı (motor/encoder objeleri) kullanıcıya bırakılmıştır.
// Amaç: Derslerde üst seviye davranışları ve kontrolleri göstermek.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

// Donanım bağlama (kullanıcı doldurmalı)
static probot::controllers::ClosedLoopMotor* g_left   = nullptr;
static probot::controllers::ClosedLoopMotor* g_right  = nullptr;
static probot::controllers::BasicTankDrive*  g_chassis= nullptr;

static probot::motor::IMotor* g_intakeMotor = nullptr;   // ham güç ile çalışır
static probot::motor::IMotor* g_shooterMotor= nullptr;   // ham güç ile çalışır
static void* g_owner = (void*)0xBEEF;

static probot::controllers::Slider* g_sliderL = nullptr; // sol tırmanma
static probot::controllers::Slider* g_sliderR = nullptr; // sağ tırmanma

// Tuş atamaları (UI tarafındaki buton indeksleri örnektir)
static const int BTN_INTAKE_IN   = 0; // A
static const int BTN_SHOOT       = 1; // B
static const int BTN_CLIMB_OPEN  = 8; // LB
static const int BTN_CLIMB_CLOSE = 9; // LT

// Otonom senaryo adımları
static uint32_t g_autoStep = 0;
static uint32_t g_autoMs   = 0;

void robotInit(){
  Serial.println("[FullRobot] robotInit: Başlatılıyor");
  // Şasi parametre örnekleri (kullanıcı uygun değerleri koymalı)
  // if (g_chassis){ g_chassis->setWheelCircumference(31.4f); g_chassis->setTrackWidth(25.0f); }

  // Intake/Shooter motor claim
  if (g_intakeMotor && g_intakeMotor->claim(g_owner)) {
    g_intakeMotor->setPower(0, g_owner);
  }
  if (g_shooterMotor && g_shooterMotor->claim(g_owner)) {
    g_shooterMotor->setPower(0, g_owner);
  }
}

void robotEnd(){
  if (g_intakeMotor) { g_intakeMotor->setPower(0, g_owner); g_intakeMotor->release(g_owner); }
  if (g_shooterMotor){ g_shooterMotor->setPower(0, g_owner); g_shooterMotor->release(g_owner); }
  Serial.println("[FullRobot] robotEnd: Bitti");
}

// Yardımcı fonksiyonlar
static void handleIntakeAndShooter(const probot::io::GamepadSnapshot& s){
  bool intake_in  = (s.buttonCount>BTN_INTAKE_IN)  ? s.buttons[BTN_INTAKE_IN]  : false;
  bool shoot      = (s.buttonCount>BTN_SHOOT)      ? s.buttons[BTN_SHOOT]      : false;

  // Intake: basılı iken içeri alma
  if (g_intakeMotor){ g_intakeMotor->setPower(intake_in ? 800 : 0, g_owner); }

  // Shooter: basılı iken yüksek güçte fırlatma
  if (g_shooterMotor){ g_shooterMotor->setPower(shoot ? 1000 : 0, g_owner); }
}

static void handleClimb(const probot::io::GamepadSnapshot& s){
  bool open  = (s.buttonCount>BTN_CLIMB_OPEN)  ? s.buttons[BTN_CLIMB_OPEN]  : false;
  bool close = (s.buttonCount>BTN_CLIMB_CLOSE) ? s.buttons[BTN_CLIMB_CLOSE] : false;

  if (!g_sliderL || !g_sliderR) return;

  // Açma: belirli uzunluğa git (ör. 40 cm), 2 sn bekle, sonra kapat (0 cm)
  if (open){
    g_sliderL->setTargetLength(40.0f); g_sliderR->setTargetLength(40.0f);
    uint32_t t0 = millis(); while (millis()-t0 < 2000){ g_sliderL->update(millis(), 20); g_sliderR->update(millis(), 20); delay(20);} 
    g_sliderL->setTargetLength(0.0f);  g_sliderR->setTargetLength(0.0f);
  }
  if (close){
    g_sliderL->setTargetLength(0.0f); g_sliderR->setTargetLength(0.0f);
  }

  g_sliderL->update(millis(), 20);
  g_sliderR->update(millis(), 20);
}

void teleopInit(){
  Serial.println("[FullRobot] teleopInit: Tank sürüş + intake/shooter + climb");
}

void teleopLoop(){
  auto s = probot::io::gamepad().read();

  // Tank sürüş: sol Y ve sağ Y eksenleri
  if (g_chassis){
    float left_axis  = (s.axisCount>1)? s.axes[1] : 0.0f;
    float right_axis = (s.axisCount>3)? s.axes[3] : 0.0f;
    float max_vel = 100.0f;
    g_chassis->setVelocity(left_axis*max_vel, right_axis*max_vel);
    g_chassis->update(millis(), 20);
  }

  handleIntakeAndShooter(s);
  handleClimb(s);

  delay(20);
}

void autonomousInit(){
  Serial.println("[FullRobot] autonomousInit: Otonom başlayacak");
  g_autoStep = 0; g_autoMs = millis();
}

void autonomousLoop(){
  if (!g_chassis){ delay(20); return; }
  uint32_t now = millis();
  switch (g_autoStep){
    case 0:
      Serial.println("[FullRobot/Auto] 1) 50 cm ileri");
      g_chassis->driveDistance(50.0f);
      g_autoStep=1; g_autoMs=now; break;
    case 1:
      if (now - g_autoMs > 3000){
        Serial.println("[FullRobot/Auto] 2) Shooter çalıştır");
        if (g_shooterMotor) g_shooterMotor->setPower(1000, g_owner);
        g_autoStep=2; g_autoMs=now;
      }
      break;
    case 2:
      if (now - g_autoMs > 2000){
        Serial.println("[FullRobot/Auto] 3) Shooter durdur");
        if (g_shooterMotor) g_shooterMotor->setPower(0, g_owner);
        g_autoStep=3; g_autoMs=now;
      }
      break;
    case 3:
      if (now - g_autoMs > 500){
        Serial.println("[FullRobot/Auto] 4) 30 cm ileri");
        g_chassis->driveDistance(30.0f);
        g_autoStep=4; g_autoMs=now;
      }
      break;
    default:
      break;
  }
  delay(20);
} 