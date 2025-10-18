#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>
#include <probot/mechanism/nfr/shooter.hpp>

// Shooter tekeri için Boardoza VNH pin konfigürasyonu.
static constexpr int PIN_INA = 15;
static constexpr int PIN_INB = 16;
static constexpr int PIN_PWM = 17;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNHMotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);
static probot::sensors::NullEncoder          encoder;
static const probot::control::PidConfig      kVelocityPid{.kp = 0.4f, .ki = 0.02f, .kd = 0.0f,
                                                          .kf = 0.0f, .out_min = -1.0f, .out_max = 1.0f};
static probot::control::PID                  pid(kVelocityPid);
static probot::control::ClosedLoopMotor      shooterMotor(&encoder, &pid, &motor, 1.0f, 1.0f);
static probot::mechanism::nfr::NfrShooter    shooter(&shooterMotor);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(false); // shooter çarkında coast tercihi yapılabilir

  shooterMotor.setTimeoutMs(0);
  shooterMotor.selectDefaultSlot(probot::control::ControlType::kVelocity, 0);
  shooterMotor.setPidSlotConfig(0, kVelocityPid);
  shooter.setTicksPerRevolution(4096.0f); // kullandığınız enkoder değerini girin
  shooter.setRpm(0.0f, 0.0f);

  Serial.println("[ShooterDemo] robotInit: Shooter kontrolü başlatıldı");
}

void robotEnd() {
  shooter.stop();
  Serial.println("[ShooterDemo] robotEnd: Teker kapatıldı");
}

void teleopInit() {
  Serial.println("[ShooterDemo] teleopInit:"
                 " sağ tetik hızlandırır, sol tetik fren yapar");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float accel = js.getRightTriggerAxis(); // 0..1
  float brake = js.getLeftTriggerAxis();  // 0..1

  float targetRpm = accel * 3200.0f;      // istenen maksimum RPM
  if (brake > 0.2f) targetRpm = 0.0f;     // fren tetiklendiğinde durdur

  shooter.setPrimaryRpm(targetRpm);
  shooterMotor.update(millis(), 20);

  Serial.printf("[ShooterDemo] hedef=%.0f rpm ölçüm=%.1f rpm çıkış=%.2f\n",
                targetRpm,
                shooterMotor.lastMeasurement(),
                shooterMotor.lastOutput());

  delay(20);
}

void autonomousInit() {
  Serial.println("[ShooterDemo] autonomousInit: 2 saniye spool, sonra durdur");
  shooter.setPrimaryRpm(3000.0f);
}

void autonomousLoop() {
  static uint32_t start = millis();
  shooterMotor.update(millis(), 20);
  if (millis() - start > 2000) {
    shooter.stop();
  }
  delay(20);
}
