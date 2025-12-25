#include <probot.h>
#include <probot/test/null_encoder.hpp>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh_motor_driver.hpp>
#include <probot/control/closed_loop_motor.hpp>
#include <probot/control/pid.hpp>

// Boardoza VNH sürücüsünü kullanırken kendi pinlerinizi mutlaka kontrol edin.
static constexpr int PIN_INA = 39;
static constexpr int PIN_INB = 40;
static constexpr int PIN_PWM = 41;
static constexpr int PIN_ENA = -1;
static constexpr int PIN_ENB = -1;

static probot::motor::BoardozaVNHMotorDriver motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);
static probot::test::NullEncoder          encoder;
static const probot::control::PidConfig      kPid{.kp = 0.30f, .ki = 0.0f, .kd = 0.0f, .kf = 0.0f,
                                                  .out_min = -1.0f, .out_max = 1.0f};
static probot::control::PID                  pid(kPid);
static probot::control::ClosedLoopMotor      controller(&encoder, &pid, &motor, 1.0f, 1.0f);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

void robotInit() {
  Serial.begin(115200);
  delay(100);

  motor.begin();
  motor.setBrakeMode(false);  // sürücü serbest bırakıldı, coast modunda
  controller.setInverted(false);

  Serial.println("[MotorDriverDemo] robotInit: IMotorDriver arayüzünü test etmek için hazır");
}

void robotEnd() {
  motor.setPower(0.0f);
  Serial.println("[MotorDriverDemo] robotEnd: Motor durduruldu");
}

void teleopInit() {
  Serial.println("[MotorDriverDemo] teleopInit:"
                 " sol eksen gücü, sağ tetik ise yön tersleme yapar");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  // Sol çubuktan ham güç, sağ tetikten tersleme isteği okuyoruz.
  float power = js.getLeftY();
  bool invert = js.getRightTriggerAxis() > 0.5f;

  motor.setInverted(invert);

  // IMotorDriver doğrudan PWM uygular; ClosedLoopMotor'u sadece izleme için kullanıyoruz.
  motor.setPower(power);
  controller.setPowerDirect(power); // lastOutput() bilgisi için eşleştiriyoruz.

  Serial.printf("[MotorDriverDemo] power=%.2f invert=%d motorOut=%.2f\n",
                power, invert ? 1 : 0, controller.lastOutput());

  delay(40);
}

void autonomousInit() {
  Serial.println("[MotorDriverDemo] autonomousInit: Motoru yavaşça durduruyoruz");
}

void autonomousLoop() {
  motor.setPower(0.0f);
  controller.update(millis(), 20);
  delay(20);
}
