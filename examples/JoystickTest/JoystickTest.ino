#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// Pin atamalarını kendi kartınıza göre güncelleyin.
static constexpr int PIN_INA = 47;
static constexpr int PIN_INB = 46;
static constexpr int PIN_PWM = 48;
static constexpr int PIN_ENA = -1; // EN pinleri 3V3'e bağlıysa -1 bırakmak yeterli.
static constexpr int PIN_ENB = -1;

// Tüm örneklerde aynı temel motor-stub setini kullanıyoruz.
static probot::motor::BoardozaVNH5019MotorController motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);


void robotInit() {
  Serial.begin(115200);
  delay(100);

  // Kart ve motor kontrolcusunu hazirla.
  motor.begin();
  motor.setBrakeMode(true);         // boşta tam fren uygula
  motor.setPower(0.0f);

  Serial.println("[JoystickTest] robotInit: Joystick ve motor izleme başlatıldı");
}

void robotEnd() {
  motor.setPower(0.0f);  // çıkışları güvenle sıfırla
  Serial.println("[JoystickTest] robotEnd: Motor sıfırlandı");
}

void teleopInit() {
  Serial.println("[JoystickTest] teleopInit: Ekseni okuyup motora aktaracağız");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  // Sol çubuk Y eksenini oku (varsayılan eşleme).
  float axis = js.getLeftY();

  // Okuduğumuz değeri doğrudan motora gönderiyoruz.
  motor.setPower(axis);

  // Seri log ile joystick verisini ve motor komutunu gözlemleyin.
  Serial.printf("[JoystickTest] seq=%lu axisY=%.2f motorCmd=%.2f\n",
                static_cast<unsigned long>(js.getSeq()),
                axis,
                motor.getPower());

  delay(50);
}

void autonomousInit() {
  Serial.println("[JoystickTest] autonomousInit: Bu örnek otonomda motoru durdurur");
  motor.setPower(0.0f);
}

void autonomousLoop() {
  // Joystick okumadığımız için otonomda motoru sıfırda tutuyoruz.
  delay(20);
}
