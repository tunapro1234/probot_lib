#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/command.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// ============================================================================
// Basit Shooter Subsystem (acik cevrim)
// ============================================================================
class ShooterSubsystem : public probot::command::SubsystemBase {
public:
  explicit ShooterSubsystem(probot::motor::IMotorController* motor)
    : SubsystemBase("Shooter"), motor_(motor) {}

  void setPower(float power) {
    power_ = constrain(power, 0.0f, 1.0f);  // shooter sadece ileri doner
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
static constexpr int PIN_INA = 15;
static constexpr int PIN_INB = 16;
static constexpr int PIN_PWM = 17;

static probot::motor::BoardozaVNH5019MotorController motor(PIN_INA, PIN_INB, PIN_PWM, -1, -1);
static ShooterSubsystem shooter(&motor);

using Scheduler = probot::command::Scheduler;

void robotInit() {
  Serial.begin(115200);
  motor.begin();
  motor.setBrakeMode(false);  // shooter coast modunda

  Scheduler::instance().registerSubsystem(&shooter);
  Serial.println("[ShooterDemo] Shooter subsystem hazir");
}

void robotEnd() {
  Scheduler::instance().unregisterSubsystem(&shooter);
  shooter.stop();
  Serial.println("[ShooterDemo] Bitti");
}

void teleopInit() {
  Serial.println("[ShooterDemo] Sag tetik hizlandirir, sol tetik durdurur");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float accel = js.getRightTriggerAxis();
  float brake = js.getLeftTriggerAxis();

  float power = accel;
  if (brake > 0.2f) power = 0.0f;

  shooter.setPower(power);

  probot::telemetry::printf("Shooter: %.2f\n", shooter.getPower());
}

void autonomousInit() {
  shooter.stop();
}

void autonomousLoop() {
  // Otonom sekans eklenebilir
}
