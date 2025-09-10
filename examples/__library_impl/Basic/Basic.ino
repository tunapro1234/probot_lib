#include <probot.h>

static probot::controllers::BlinkPid* g_pid = nullptr;

void robotInit() {
  Serial.printf("[USER ] robotInit (core %d)\n", xPortGetCoreID());
}

void robotEnd() {
  Serial.printf("[USER ] robotEnd (core %d)\n", xPortGetCoreID());
}

void teleopInit() {
  static probot::controllers::BlinkPid pid_instance; // program ömrü boyunca
  g_pid = &pid_instance;
  control::setGlobalPeriodMs(20);
  control::attach(g_pid);
  Serial.println("[USER ] teleopInit: BlinkPid attached (20 ms global)");
}

void teleopLoop() {
  uint32_t last_sec = joystick::get_ms() / 1000u;
  while (true) {
    uint32_t ms  = joystick::get_ms();
    uint32_t sec = ms / 1000u;
    if (sec != last_sec && g_pid){
      last_sec = sec;
      g_pid->setReference(sec);
      Serial.printf("[USER] setReference(sec=%lu)\n", (unsigned long)sec);
    }
    auto js = joystick::read();
    Serial.printf("[USER/teleopLoop] seq=%lu ms=%lums (core %d)\n",
                  (unsigned long)js.seq, (unsigned long)js.ms, xPortGetCoreID());
    delay(500);
  }
}

void autonomousInit() {
  Serial.printf("[USER ] autonomousInit (core %d)\n", xPortGetCoreID());
}

void autonomousLoop() {
  Serial.printf("[USER ] autonomousLoop (core %d)\n", xPortGetCoreID());
  delay(1000);
}
