/*
  Boardoza VNH Motor Demo (Joystick Kontrollü)
  --------------------------------------------
  Bu örnek, VNH5019 tabanlı Boardoza motor sürücüsünü Probot Driver Station
  joystick verisiyle nasıl sürebileceğinizi gösterir.

  Adımlar:
    1. Aşağıdaki pin tanımlarını kendi kartınıza göre düzenleyin.
    2. Şifreyi (en az 8 karakter) değiştirin ve kaydedin.
    3. S3 kartınızı çalıştırın, bilgisayardan Probot-XXXX ağına bağlanın.
    4. Tarayıcıdan 192.168.4.1 adresine gidip joystick’i hareket ettirin.
    5. Joystick Y ekseni (varsayılan: axes[1]) motora -1..1 arasında güç gönderir.

  Notlar:
    - Sürücü her zaman fren modunda bekler. Joystick bırakıldığında motor sıkı durur.
    - Joystick ölçeği -1..1 olduğundan Probot motor gücüyle birebir uyumludur.
    - `GamepadSnapshot` eksen sırası driver station’daki girişe göre değişebilir.
      Gerekirse `AXIS_INDEX` sabitini uyarlayın (örn. 0 veya 3).
*/

#include <probot.h>
#include <cmath>

// --- Motor sürücü pinleri (kartınızın bağlantısına göre değiştirin) ---
static constexpr int PIN_INA = 47;
static constexpr int PIN_INB = 46;
static constexpr int PIN_PWM = 48;
static constexpr int PIN_ENA = -1; // ENA pini 3V3'e bağlıysa -1 bırakın
static constexpr int PIN_ENB = -1; // ENB pini 3V3'e bağlıysa -1 bırakın

// Joystick ekseni: çoğu gamepad'de 1 = sol çubuğun Y ekseni
static constexpr uint8_t AXIS_INDEX = 1;
static constexpr float   DEADZONE   = 0.05f; // küçük titreşimleri yok say

// Driver Station erişimi için şifre (minimum 8 karakter)
PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

// Motor nesnemiz ve kontrol sapı
static probot::motor::BoardozaVNHMotorDriver g_motor(
  PIN_INA,
  PIN_INB,
  PIN_PWM,
  PIN_ENA,
  PIN_ENB
);
static probot::motor::MotorHandle g_motorHandle(g_motor);
static float g_lastReportedPower = 0.0f;

static float applyDeadband(float value){
  if (std::fabs(value) < DEADZONE) return 0.0f;
  return value;
}

static float clampUnit(float v){
  if (v > 1.0f) return 1.0f;
  if (v < -1.0f) return -1.0f;
  return v;
}

// --- Probot yaşam döngüsü fonksiyonları ---
void robotInit(){
  Serial.begin(115200);
  delay(100);
  Serial.println("[BoardozaVNH] Demo starting (20 kHz PWM, brake mode)");
  g_motor.begin();
  g_motor.setBrakeMode(true);     // boşta tam fren
  g_motor.setBrakeStrength(1.0f); // PWM %100 ile fren uygula
  g_motorHandle.setPower(0.0f);
  g_lastReportedPower = 0.0f;
}

void robotEnd(){
  g_motorHandle.setPower(0.0f);
  Serial.println("[BoardozaVNH] Demo stopped");
}

void teleopInit(){
  g_motorHandle.setPower(0.0f);
  g_lastReportedPower = 0.0f;
}

void teleopLoop(){
  auto snapshot = probot::io::gamepad().read();

  float power = 0.0f;
  if (snapshot.axisCount > AXIS_INDEX){
    power = clampUnit(snapshot.axes[AXIS_INDEX]);
    power = -power; // Logitech tabanlı gamepadlerde ileri hareket pozitif olsun
    power = applyDeadband(power);
  }

  g_motorHandle.setPower(power);

  // İsteğe bağlı: bir düğmeye basıldığında yönü ters çevirmek
  /*
  if (snapshot.buttonCount > 0 && snapshot.buttons[0]){
    g_motorHandle.setInverted(true);
  } else {
    g_motorHandle.setInverted(false);
  }
  */

  if (std::fabs(power - g_lastReportedPower) > 0.02f){
    Serial.printf("[BoardozaVNH] Power command: %.2f (seq=%lu)\n",
                  static_cast<double>(power),
                  static_cast<unsigned long>(snapshot.seq));
    g_lastReportedPower = power;
  }

  delay(10); // 100 Hz kontrol döngüsü
}

void autonomousInit(){}
void autonomousLoop(){}
