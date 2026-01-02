#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/command.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// ============================================================================
// Basit Slider Subsystem (acik cevrim)
// ============================================================================
class SliderSubsystem : public probot::command::SubsystemBase {
public:
  explicit SliderSubsystem(probot::motor::IMotorController* motor)
    : SubsystemBase("Slider"), motor_(motor) {}

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
static constexpr int PIN_INA = 12;
static constexpr int PIN_INB = 13;
static constexpr int PIN_PWM = 14;

static probot::motor::BoardozaVNH5019MotorController motor(PIN_INA, PIN_INB, PIN_PWM, -1, -1);
static SliderSubsystem slider(&motor);

using Scheduler = probot::command::Scheduler;

void robotInit() {
  Serial.begin(115200);
  motor.begin();
  motor.setBrakeMode(true);

  Scheduler::instance().registerSubsystem(&slider);
  Serial.println("[SliderDemo] Slider subsystem hazir");
}

void robotEnd() {
  Scheduler::instance().unregisterSubsystem(&slider);
  slider.stop();
  Serial.println("[SliderDemo] Bitti");
}

void teleopInit() {
  Serial.println("[SliderDemo] D-pad yukari/asagi ile slider kontrol edin");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();
  int pov = js.getPOV();

  float cmd = 0.0f;
  if (pov == 0) cmd = 0.6f;      // yukari
  if (pov == 180) cmd = -0.6f;   // asagi

  slider.setPower(cmd);

  probot::telemetry::printf("Slider: %.2f\n", slider.getPower());
}

void autonomousInit() {
  slider.stop();
}

void autonomousLoop() {
  // Otonom sekans eklenebilir
}
