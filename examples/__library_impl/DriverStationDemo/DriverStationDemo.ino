#include <Arduino.h>
#include <probot.h>
#include <platform/esp32s3/web/driver_station_esp32.hpp>

static probot::robot::StateService   g_state;
static probot::io::GamepadService    g_gamepad;
static probot::platform::esp32::DriverStation g_ds(g_state, g_gamepad);

PROBOT_SET_DRIVER_STATION_PASSWORD("ProBot1234");

// TestDriverStationDemo

void robotInit() {}
void robotEnd() {}
void teleopInit() {}
void teleopLoop() { delay(100); }
void autonomousInit() {}
void autonomousLoop() { delay(1000); } 