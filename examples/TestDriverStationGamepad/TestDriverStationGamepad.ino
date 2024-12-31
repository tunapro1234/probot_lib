#include <probot.h>

DriverStation driverStation();
TunaGamepad tuna_pad(&driverStation);
unsigned long last_print = 0;

void printGamepadData() {
    if (!tuna_pad.checkDriverStationJoystick()) {
        Serial.println("No Joystick Available.");
        return;
    }

    Serial.print("Joystick Left X: ");
    Serial.print(tuna_pad.getAxis(Axis::LeftJoystickX));
    Serial.print(" | Joystick Left Y: ");
    Serial.println(tuna_pad.getAxis(Axis::LeftJoystickY));

    Serial.print("Joystick Right X: ");
    Serial.print(tuna_pad.getAxis(Axis::RightJoystickX));
    Serial.print(" | Joystick Right Y: ");
    Serial.println(tuna_pad.getAxis(Axis::RightJoystickY));

    Serial.print("A: ");
    Serial.print(tuna_pad.getButton(Button::A));
    Serial.print(" B: ");
    Serial.print(tuna_pad.getButton(Button::B));
    Serial.print(" X: ");
    Serial.print(tuna_pad.getButton(Button::X));
    Serial.print(" Y: ");
    Serial.println(tuna_pad.getButton(Button::Y));

    Serial.print("DPad Up: ");
    Serial.print(tuna_pad.getButton(Button::DPadUp));
    Serial.print(" DPad Down: ");
    Serial.print(tuna_pad.getButton(Button::DPadDown));
    Serial.print(" DPad Left: ");
    Serial.print(tuna_pad.getButton(Button::DPadLeft));
    Serial.print(" DPad Right: ");
    Serial.println(tuna_pad.getButton(Button::DPadRight));

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
        printGamepadData();
    }
}