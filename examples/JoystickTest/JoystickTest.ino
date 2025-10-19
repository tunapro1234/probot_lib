#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>

// Pin atamalarını kendi kartınıza göre güncelleyin.
static constexpr int PIN_INA = 47;
static constexpr int PIN_INB = 46;
static constexpr int PIN_PWM = 48;
static constexpr int PIN_ENA = -1; // EN pinleri 3V3'e bağlıysa -1 bırakmak yeterli.
static constexpr int PIN_ENB = -1;

// Tüm örneklerde aynı temel motor-stub setini kullanıyoruz.
static probot::motor::BoardozaVNHMotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);
static probot::sensors::NullEncoder          encoder;
static const probot::control::PidConfig      kPid{.kp = 0.25f, .ki = 0.0f, .kd = 0.0f, .kf = 0.0f,
                                                  .out_min = -1.0f, .out_max = 1.0f};
static probot::control::PID                  pid(kPid);
static probot::control::ClosedLoopMotor      controller(&encoder, &pid, &motor, 1.0f, 1.0f);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  // Kart ve sürücüyü hazırla.
  motor.begin();
  motor.setBrakeMode(true);         // boşta tam fren uygula
  controller.setSetpoint(0.0f, probot::control::ControlType::kPercent);

  Serial.println("[JoystickTest] robotInit: Joystick ve motor izleme başlatıldı");
}

void robotEnd() {
  controller.setPowerDirect(0.0f);  // çıkışları güvenle sıfırla
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
  controller.setPowerDirect(axis);

  // Seri log ile joystick verisini ve motor komutunu gözlemleyin.
  Serial.printf("[JoystickTest] seq=%lu axisY=%.2f motorCmd=%.2f\n",
                static_cast<unsigned long>(js.getSeq()),
                axis,
                controller.lastOutput());

  delay(50);
}

void autonomousInit() {
  Serial.println("[JoystickTest] autonomousInit: Bu örnek otonomda motoru durdurur");
  controller.setPowerDirect(0.0f);
}

void autonomousLoop() {
  // Joystick okumadığımız için otonomda motoru sıfırda tutuyoruz.
  controller.update(millis(), 20);
  delay(20);
}
