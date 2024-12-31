#ifndef DRIVER_STATION_NODEMCU_H
#define DRIVER_STATION_NODEMCU_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <atomic>
#include "index_html.h"

#define JOYSTICK_MAX_AXIS_COUNT 20
#define JOYSTICK_MAX_BUTTON_COUNT 20

// Enum for Robot Status
enum class RobotStatus {
    INIT,
    START,
    STOP
};

class DriverStationNodeMCU {
public:
    DriverStationNodeMCU(const char* apPass = "robotpro1234");

    void begin();
    void handleClient();

    // Getters
    float getJoystickAxis(int axis);
    bool getJoystickButton(int button);
    RobotStatus getRobotStatus();
    int getClientCount();
    int getJoystickAxisCount();
    int getJoystickButtonCount();


    // Setter
    void setBatteryVoltage(float voltage);

private:
    const char* AP_PASS;
    ESP8266WebServer server;

    // Robot state
    std::atomic<RobotStatus> robotStatus;
    std::atomic<bool> enableAutonomous;
    std::atomic<int> autoPeriod;
    std::atomic<float> batteryVoltage;

    // Joystick state
    std::atomic<float> joystickAxes[JOYSTICK_MAX_AXIS_COUNT];
    std::atomic<bool> joystickButtons[JOYSTICK_MAX_BUTTON_COUNT];
    std::atomic<unsigned int> joystickAxisCount;
    std::atomic<unsigned int> joystickButtonCount;

    // Client count
    std::atomic<int> clientCount;

    // Private Methods
    String generateSSID();
    void handleRoot();
    void handleUpdateController();
    void handleRobotControl();
    void handleGetBattery();

    String robotStatusToString(RobotStatus status); // Helper for status conversion
};

#endif // DRIVER_STATION_NODEMCU_H
