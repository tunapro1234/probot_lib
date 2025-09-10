#include <probot.h>
PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

// TestSlider: demonstrate Slider controlling position of a single axis

static probot::sim::SimMotor   mot;
static probot::sim::SimEncoder enc;
static probot::control::PidConfig cfgP{ 1.2f, 0.0f, 0.02f, -1000.0f, 1000.0f };
static probot::control::PID pid(cfgP);
static probot::controllers::ClosedLoopMotor* axis=nullptr;
static probot::controllers::Slider* slider=nullptr;

void robotInit(){
  control::setGlobalPeriodMs(20);
  static probot::controllers::ClosedLoopMotor x(&enc, &pid, &mot, 1.0f, 1.0f);
  axis = &x;
  axis->setPidSlotConfig(1, cfgP);
  axis->selectDefaultSlot(probot::controllers::ControlType::kPosition, 1);
  static probot::sim::SimPlant plant(&mot, &enc);
  control::attach(axis);
  control::attach(&plant);

  static probot::controllers::Slider s(axis);
  slider = &s;
  slider->setLengthToTicks(100.0f); // 1 unit -> 100 ticks
  control::attach(slider);
}

void robotEnd() {}
void teleopInit(){ Serial.println("[SLID] teleopInit"); }

void teleopLoop(){
  static uint32_t last=0; uint32_t now=joystick::get_ms();
  if (now - last >= 2000){
    last = now;
    static bool toggle=false; toggle=!toggle;
    slider->setTargetLength(toggle ? 5.0f : 0.0f); // units
    Serial.printf("[SLID] target=%.1f units\n", slider->getTargetLength());
  }
  delay(20);
}

void autonomousInit(){}
void autonomousLoop(){ delay(1000); } 