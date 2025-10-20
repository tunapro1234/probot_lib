#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>
#include <probot/mechanism/turret.hpp>

// Taret döndürme motoru için Boardoza VNH pinleri.
static constexpr int PIN_INA = 30;
static constexpr int PIN_INB = 31;
static constexpr int PIN_PWM = 32;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNHMotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);
static probot::sensors::NullEncoder          encoder;
static const probot::control::PidConfig      kPositionPid{.kp = 0.6f, .ki = 0.0f, .kd = 0.0f,
                                                          .kf = 0.0f, .out_min = -1.0f, .out_max = 1.0f};
static probot::control::PID                  pid(kPositionPid);
static probot::control::ClosedLoopMotor      turretMotor(&encoder, &pid, &motor, 1.0f, 1.0f);
static probot::mechanism::Turret             turret(&turretMotor);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(true);

  turretMotor.selectDefaultSlot(probot::control::ControlType::kPosition, 0);
  turretMotor.setPidSlotConfig(0, kPositionPid);
  turretMotor.setTimeoutMs(0);

  turret.setDegreesToTicks(100.0f);       // encoder dönüşümünü kendi mekanizmanıza göre ayarlayın
  turret.setAngleLimits(-120.0f, 120.0f); // güvenli dönüş aralığı
  turret.setTargetAngleDeg(0.0f);
  turret.setSlewRateLimit(150.0f);        // hızlı bile olsa yumuşatma olsun

  Serial.println("[TurretDemo] robotInit: Taret kontrolü hazır");
}

void robotEnd() {
  turretMotor.setPowerDirect(0.0f);
  Serial.println("[TurretDemo] robotEnd: Taret durduruldu");
}

void teleopInit() {
  Serial.println("[TurretDemo] teleopInit:"
                 " sağ çubuk X hedef açıyı belirler, B butonu sıfırlar");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  if (js.getRawButton(1)) { // B butonu
    turret.setTargetAngleDeg(0.0f);
  } else {
    float stick = js.getRightX();        // -1..1
    turret.setTargetAngleDeg(stick * 90.0f); // ±90 derece
  }

  turret.update(millis(), 20);
  turretMotor.update(millis(), 20);

  Serial.printf("[TurretDemo] hedef=%.1f ölçüm=%.1f çıkış=%.2f\n",
                turret.getTargetAngleDeg(),
                turret.getCurrentAngleDeg(),
                turretMotor.lastOutput());

  delay(20);
}

void autonomousInit() {
  Serial.println("[TurretDemo] autonomousInit: Rastgele preset taraması");
}

void autonomousLoop() {
  static uint32_t stateStart = millis();
  static int state = 0;
  uint32_t now = millis();

  if (state == 0) {
    turret.setTargetAngleDeg(-60.0f);
    stateStart = now;
    state = 1;
  } else if (state == 1 && now - stateStart > 1500) {
    turret.setTargetAngleDeg(60.0f);
    stateStart = now;
    state = 2;
  } else if (state == 2 && now - stateStart > 1500) {
    turret.setTargetAngleDeg(0.0f);
    state = 3;
  }

  turret.update(now, 20);
  turretMotor.update(now, 20);
  delay(20);
}
