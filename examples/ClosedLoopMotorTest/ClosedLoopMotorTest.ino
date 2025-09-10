#include <probot.h>

// Bu örnek, kapalı çevrim (ClosedLoopMotor) bir motoru joystick ile sürer.
// Sol çubuk Y ekseni hız referansı, D-Pad (ör. butonlar) ise mod geçişi gibi kullanılabilir.
// Encoder, motor ve PID nesneleri gerçek donanıma göre bağlanmalıdır.

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::sensors::IEncoder* g_encoder = nullptr;     // kullanıcı doldurmalı
static probot::motor::IMotor*     g_motor   = nullptr;     // kullanıcı doldurmalı
static control::PID               g_pid;                    // basit PID nesnesi
static probot::controllers::ClosedLoopMotor* g_clm = nullptr;

void robotInit() {
  Serial.println("[CLMTest] robotInit: ClosedLoopMotor testi");
  // Varsayılan PID ayarları (örnek değerler)
  control::PidConfig cfg{ .kp=200.0f, .ki=0.0f, .kd=0.0f, .outMin=-1000, .outMax=1000 };
  g_pid.setConfig(cfg);
  if (g_encoder && g_motor) {
    static probot::controllers::ClosedLoopMotor clm(g_encoder, &g_pid, g_motor, /*vel_ticks_to_units*/1.0f, /*pos_ticks_to_units*/1.0f);
    g_clm = &clm;
  }
}

void robotEnd() {
  Serial.println("[CLMTest] robotEnd: Bitti");
}

void teleopInit() {
  Serial.println("[CLMTest] teleopInit: Joystick ile hız/konum kontrolü");
}

void teleopLoop() {
  if (!g_clm) { delay(200); return; }
  auto s = probot::io::gamepad().read();
  float axis = (s.axisCount > 1) ? s.axes[1] : 0.0f; // sol çubuk Y
  float vel_ref = axis * 100.0f; // örnek: 100 birim/s maksimum hız
  g_clm->setSetpoint(vel_ref, probot::controllers::ControlType::kVelocity);
  g_clm->update(millis(), 20);
  Serial.printf("[CLMTest] vel_ref=%.2f\n", vel_ref);
  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(1000); } 