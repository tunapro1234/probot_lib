#include <probot.h>

DriverStationNodeMCU driverStation;
unsigned long last_print = 0;

void printJoystickData() {
    // Print current joystick axis and button counts
    int axisCount = driverStation.getJoystickAxisCount();
    int buttonCount = driverStation.getJoystickButtonCount();

    Serial.print("Current Axis Count: ");
    Serial.println(axisCount);

    Serial.print("Current Button Count: ");
    Serial.println(buttonCount);

    // Example: Print joystick axis values if any
    for (int i = 0; i < axisCount; i++) {
        Serial.print("Axis ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(driverStation.getJoystickAxis(i));
    }

    // Example: Print joystick button states if any
    for (int i = 0; i < buttonCount; i++) {
        Serial.print("Button ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(driverStation.getJoystickButton(i) ? "Pressed" : "Released");
    }

    Serial.println("-----------------------------");
}

void setup() {
    Serial.begin(115200);
    driverStation.begin();
    Serial.println("Driver Station Initialized!");
}

void loop() {
    driverStation.handleClient();

    if (millis() - last_print > 1000) {
        last_print = millis();
        printJoystickData();
    }
}