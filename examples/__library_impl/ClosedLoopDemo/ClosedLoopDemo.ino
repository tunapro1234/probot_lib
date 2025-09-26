#include <probot.h>

// TestClosedLoopDemo
// Demonstrates ClosedLoopMotor with simulated components

static probot::sim::SimMotor   g_motor;
static probot::sim::SimEncoder g_encoder;
static probot::control::PidConfig g_cfg_vel{ 0.0008f, 0.0004f, 0.0f, -1.0f, 1.0f };
static probot::control::PID        g_pid(g_cfg_vel);
static probot::controllers::ClosedLoopMotor* g_axis = nullptr;
static probot::sim::SimPlant* g_plant = nullptr;

class LedVisualizer : public control::IUpdatable {
public:
  LedVisualizer(probot::sensors::IEncoder* enc, float full_scale_tps)
  : enc_(enc), full_scale_tps_(full_scale_tps) {}
  void update(uint32_t now_ms, uint32_t dt_ms) override {
    (void)now_ms; (void)dt_ms;
    int32_t tps = enc_ ? enc_->readTicksPerSecond() : 0;
    if (tps < 0) tps = -tps;
    float x = (float)tps / (full_scale_tps_ > 1.0f ? full_scale_tps_ : 1.0f);
    if (x > 1.0f) x = 1.0f;
    uint8_t br = (uint8_t)(x * 255.0f);
    probot::builtinled::setBrightness(br);
    probot::builtinled::set(true);
  }
private:
  probot::sensors::IEncoder* enc_;
  float full_scale_tps_;
};

static LedVisualizer* g_ledviz = nullptr;

void robotInit() {}
void robotEnd() {}

void teleopInit() {
  static probot::controllers::ClosedLoopMotor axis(&g_encoder, &g_pid, &g_motor, 1.0f, 1.0f);
  g_axis = &axis;

  g_axis->setPidSlotConfig(0, g_cfg_vel);
  g_axis->selectDefaultSlot(probot::controllers::ControlType::kVelocity, 0);

  g_motor.setInverted(false);

  static probot::sim::SimPlant plant(&g_motor, &g_encoder);
  g_plant = &plant;

  static LedVisualizer ledviz(&g_encoder, 400.0f); // full brightness at ~400 tps
  g_ledviz = &ledviz;

  control::setGlobalPeriodMs(20);
  control::attach(g_axis);
  control::attach(g_plant);
  control::attach(g_ledviz);

  Serial.println("[DEMO ] teleopInit: ClosedLoop + Plant + LED attached (20 ms)");
}

void teleopLoop() { delay(500); }

void autonomousInit() {}

void autonomousLoop() { delay(1000); } 
