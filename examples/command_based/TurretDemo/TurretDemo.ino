#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/command.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// ============================================================================
// Basit Turret Subsystem (acik cevrim)
// ============================================================================
class TurretSubsystem : public probot::command::SubsystemBase {
public:
  explicit TurretSubsystem(probot::motor::IMotorController* motor)
    : SubsystemBase("Turret"), motor_(motor) {}

  void setPower(float power) {
    power_ = constrain(power, -1.0f, 1.0f);
  }

  float getPower() const { return power_; }

  void stop() { power_ = 0.0f; }

  void periodic(uint32_t, uint32_t) override {
    if (motor_) motor_->setPower(power_);
  }

private:
  probot::motor::IMotorController* motor_;
  float power_ = 0.0f;
};

// ============================================================================
// Hardware
// ============================================================================
static constexpr int PIN_INA = 30;
static constexpr int PIN_INB = 31;
static constexpr int PIN_PWM = 32;

static probot::motor::BoardozaVNH5019MotorController motor(PIN_INA, PIN_INB, PIN_PWM, -1, -1);
static TurretSubsystem turret(&motor);

using Scheduler = probot::command::Scheduler;

void robotInit() {
  Serial.begin(115200);
  motor.begin();
  motor.setBrakeMode(true);

  Scheduler::instance().registerSubsystem(&turret);
  Serial.println("[TurretDemo] Turret subsystem hazir");
}

void robotEnd() {
  Scheduler::instance().unregisterSubsystem(&turret);
  turret.stop();
  Serial.println("[TurretDemo] Bitti");
}

void teleopInit() {
  Serial.println("[TurretDemo] Sag stick X ile taret dondur, B ile durdur");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float cmd = js.getRightX();
  if (js.getB()) cmd = 0.0f;  // B butonu durdurur

  turret.setPower(cmd);

  probot::telemetry::printf("Turret: %.2f\n", turret.getPower());
}

void autonomousInit() {
  turret.stop();
}

void autonomousLoop() {
  // Otonom sekans eklenebilir
}
