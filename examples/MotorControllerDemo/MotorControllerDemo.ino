#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_driver.hpp>
#include <probot/sensors/encoder.hpp>

// Kullanılan Boardoza VNH sürücü pinleri (kendi kartınıza göre güncelleyin).
static constexpr int PIN_INA = 9;
static constexpr int PIN_INB = 10;
static constexpr int PIN_PWM = 11;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNH5019MotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);
static const probot::control::PidConfig      kVelocityPid{.kp = 0.35f, .ki = 0.02f, .kd = 0.0f,
                                                          .kf = 0.0f, .out_min = -1.0f, .out_max = 1.0f};
static probot::sensors::IEncoder* encoder = nullptr;

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::control::ControlType g_mode = probot::control::ControlType::kVelocity;
static bool g_has_encoder = false;

void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(true);

  motor.setTimeoutMs(0); // joystick bırakıldığında çıkış sıfırlanmasın
  if (encoder) {
    motor.attachEncoder(encoder);
    motor.setVelocityPidConfig(kVelocityPid);
    motor.setVelocity(0.0f);
    g_has_encoder = true;
  } else {
    g_mode = probot::control::ControlType::kPercent;
    g_has_encoder = false;
  }

  Serial.println("[IMotorControllerDemo] robotInit: Kapalı çevrim denemesi için hazır");
  if (!g_has_encoder) {
    Serial.println("[IMotorControllerDemo] Encoder yok, sadece yüzde modunda çalışır");
  }
}

void robotEnd() {
  motor.setPower(0.0f);
  Serial.println("[IMotorControllerDemo] robotEnd: Motor kapatıldı");
}

void teleopInit() {
  Serial.println("[IMotorControllerDemo] teleopInit:"
                 " sol eksen referans, A butonu mod değiştirir (encoder varsa velocity)");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  if (js.getRawButton(0)) {
    if (g_has_encoder) {
      g_mode = (g_mode == probot::control::ControlType::kVelocity)
                 ? probot::control::ControlType::kPercent
                 : probot::control::ControlType::kVelocity;
    } else {
      g_mode = probot::control::ControlType::kPercent;
    }
    if (g_mode == probot::control::ControlType::kVelocity) {
      motor.setVelocity(0.0f);
    } else {
      motor.setPower(0.0f);
    }
    Serial.printf("[IMotorControllerDemo] Mod değişti: %s\n",
                  g_mode == probot::control::ControlType::kVelocity ? "HIZ" : "YÜZDE");
    delay(200); // buton debouncing
  }

  float axis = js.getLeftY();
  float target = axis * (g_mode == probot::control::ControlType::kVelocity ? 100.0f : 1.0f);
  if (g_mode == probot::control::ControlType::kVelocity) {
    motor.setVelocity(target);
  } else {
    motor.setPower(target);
  }
  motor.update(millis(), 20);

  Serial.printf("[IMotorControllerDemo] mode=%s target=%.2f measurement=%.2f out=%.2f\n",
                g_mode == probot::control::ControlType::kVelocity ? "VEL" : "PCT",
                target,
                motor.lastMeasurement(),
                motor.lastOutput());

  delay(20);
}

void autonomousInit() {
  Serial.println("[IMotorControllerDemo] autonomousInit: 2 saniyelik hız profili");
  if (g_has_encoder) {
    motor.setVelocity(80.0f);
  } else {
    motor.setPower(0.6f);
  }
}

void autonomousLoop() {
  static uint32_t start = millis();
  motor.update(millis(), 20);
  if (millis() - start > 2000) {
    if (g_has_encoder) {
      motor.setVelocity(0.0f);
    } else {
      motor.setPower(0.0f);
    }
  }
  delay(20);
}
