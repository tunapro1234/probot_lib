#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>

// Kullanılan Boardoza VNH sürücü pinleri (kendi kartınıza göre güncelleyin).
static constexpr int PIN_INA = 9;
static constexpr int PIN_INB = 10;
static constexpr int PIN_PWM = 11;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNHMotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);
static probot::sensors::NullEncoder          encoder;
static const probot::control::PidConfig      kVelocityPid{.kp = 0.35f, .ki = 0.02f, .kd = 0.0f,
                                                          .kf = 0.0f, .out_min = -1.0f, .out_max = 1.0f};
static probot::control::PID                  pid(kVelocityPid);
static probot::control::ClosedLoopMotor      controller(&encoder, &pid, &motor, 1.0f, 1.0f);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

static probot::control::ControlType g_mode = probot::control::ControlType::kVelocity;

void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(true);

  controller.setTimeoutMs(0); // joystick bırakıldığında çıkış sıfırlanmasın
  controller.setSetpoint(0.0f, g_mode);

  Serial.println("[MotorControllerDemo] robotInit: Kapalı çevrim denemesi için hazır");
}

void robotEnd() {
  controller.setPowerDirect(0.0f);
  Serial.println("[MotorControllerDemo] robotEnd: Motor kapatıldı");
}

void teleopInit() {
  Serial.println("[MotorControllerDemo] teleopInit:"
                 " sol eksen referans, A butonu mod değiştirir (yüzde/velocity)");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  if (js.getRawButton(0)) {
    g_mode = (g_mode == probot::control::ControlType::kVelocity)
               ? probot::control::ControlType::kPercent
               : probot::control::ControlType::kVelocity;
    controller.setSetpoint(0.0f, g_mode);
    Serial.printf("[MotorControllerDemo] Mod değişti: %s\n",
                  g_mode == probot::control::ControlType::kVelocity ? "HIZ" : "YÜZDE");
    delay(200); // buton debouncing
  }

  float axis = js.getLeftY();
  float target = axis * (g_mode == probot::control::ControlType::kVelocity ? 100.0f : 1.0f);
  controller.setSetpoint(target, g_mode);
  controller.update(millis(), 20);

  Serial.printf("[MotorControllerDemo] mode=%s target=%.2f measurement=%.2f out=%.2f\n",
                g_mode == probot::control::ControlType::kVelocity ? "VEL" : "PCT",
                target,
                controller.lastMeasurement(),
                controller.lastOutput());

  delay(20);
}

void autonomousInit() {
  Serial.println("[MotorControllerDemo] autonomousInit: 2 saniyelik hız profili");
  controller.setSetpoint(80.0f, probot::control::ControlType::kVelocity);
}

void autonomousLoop() {
  static uint32_t start = millis();
  controller.update(millis(), 20);
  if (millis() - start > 2000) {
    controller.setSetpoint(0.0f, probot::control::ControlType::kVelocity);
  }
  delay(20);
}
