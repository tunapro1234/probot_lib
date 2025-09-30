#include <probot.h>
PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

// TestTankDrive: demonstrate BasicTankDrive chassis and Slider with sims

static probot::sim::SimMotor   motL, motR;
static probot::sim::SimEncoder encL, encR;
static probot::control::PidConfig cfgV{ 0.0009f, 0.0003f, 0.00002f, 0.0f, -1.0f, 1.0f };
static probot::control::PidConfig cfgP{ 0.0008f, 0.0f, 0.00001f, 0.0f, -1.0f, 1.0f };
static probot::control::PID pidL(cfgV), pidR(cfgV);
static probot::control::ClosedLoopMotor *axisL=nullptr, *axisR=nullptr;
static probot::drive::BasicTankDrive* chassis=nullptr;

void robotInit() {
  control::setGlobalPeriodMs(20);
  static probot::control::ClosedLoopMotor xL(&encL, &pidL, &motL, 1.0f, 1.0f);
  static probot::control::ClosedLoopMotor xR(&encR, &pidR, &motR, 1.0f, 1.0f);
  axisL = &xL; axisR = &xR;
  axisL->setPidSlotConfig(0, cfgV); axisL->setPidSlotConfig(1, cfgP);
  axisR->setPidSlotConfig(0, cfgV); axisR->setPidSlotConfig(1, cfgP);
  axisL->selectDefaultSlot(probot::control::ControlType::kVelocity, 0);
  axisR->selectDefaultSlot(probot::control::ControlType::kVelocity, 0);

  static probot::sim::SimPlant pL(&motL, &encL);
  static probot::sim::SimPlant pR(&motR, &encR);

  control::attach(axisL);
  control::attach(axisR);
  control::attach(&pL);
  control::attach(&pR);

  static probot::drive::BasicTankDrive ch(axisL, axisR);
  chassis = &ch;
  chassis->setWheelCircumference(20.0f); // arbitrary units
  chassis->setTrackWidth(30.0f);
}

void robotEnd() {}

void teleopInit() {
  Serial.println("[TANK] teleopInit: velocity drive");
}

void teleopLoop() {
  // Alternate velocity and position commands
  static uint32_t last=0; uint32_t now=joystick::get_ms();
  if (now - last >= 2000){
    last = now;
    static int step=0; step = (step+1)%4;
    if (step==0){ chassis->setVelocity(200.0f, 200.0f); Serial.println("[TANK] vel fwd"); }
    else if (step==1){ chassis->setVelocity(-200.0f, -200.0f); Serial.println("[TANK] vel back"); }
    else if (step==2){ chassis->driveDistance(10.0f); Serial.println("[TANK] drive +10"); }
    else { chassis->turnDegrees(90.0f); Serial.println("[TANK] turn +90"); }
  }
  delay(20);
}

void autonomousInit() { Serial.println("[TANK] auto: drive 10, turn 90"); chassis->driveDistance(10.0f); }
void autonomousLoop() { delay(1000); } 
