#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_driver.hpp>

// Taret döndürme motoru için Boardoza VNH pinleri.
static constexpr int PIN_INA = 30;
static constexpr int PIN_INB = 31;
static constexpr int PIN_PWM = 32;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNH5019MotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(true);
  motor.setPower(0.0f);

  Serial.println("[TurretDemo] robotInit: Taret kontrolü hazır");
}

void robotEnd() {
  motor.setPower(0.0f);
  Serial.println("[TurretDemo] robotEnd: Taret durduruldu");
}

void teleopInit() {
  Serial.println("[TurretDemo] teleopInit:"
                 " sağ çubuk X taret gücünü belirler, B butonu durdurur");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float cmd = js.getRightX();        // -1..1
  if (js.getRawButton(1)) {          // B butonu
    cmd = 0.0f;
  }
  motor.setPower(cmd);

  Serial.printf("[TurretDemo] cmd=%.2f out=%.2f\n",
                cmd,
                motor.getPower());

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
    motor.setPower(-0.4f);
    stateStart = now;
    state = 1;
  } else if (state == 1 && now - stateStart > 1200) {
    motor.setPower(0.4f);
    stateStart = now;
    state = 2;
  } else if (state == 2 && now - stateStart > 1200) {
    motor.setPower(0.0f);
    state = 3;
  }

  delay(20);
}
