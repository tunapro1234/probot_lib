#include "TunaGamepad.h"

// Constructor
TunaGamepad::TunaGamepad(const Robot& robot) : TunaGamepad(robot.getDriverStation()) { }

TunaGamepad::TunaGamepad(const DriverStation* ds) : driverStation(ds) {
    if (!driverStation) {
        Serial.println("Error: Null pointer passed to TunaGamepad constructor!");
    }
    // Initialize calibration offsets to 0
    for (int i = 0; i < 6; i++) {
        axisOffsets[i] = 0.0f;
    }
}

bool TunaGamepad::checkDriverStationJoystick() const {
    // Check if joystick data is available
    if (driverStation->getJoystickAxisCount() == 0 && driverStation->getJoystickButtonCount() == 0) {
        Serial.println("No Joystick Available!");
        return false;
    }
    if (driverStation->getJoystickAxisCount() != TUNA_GAMEPAD_AXIS_COUNT || driverStation->getJoystickButtonCount() != TUNA_GAMEPAD_BUTTON_COUNT) {
        Serial.println("This is not a Tuna Gamepad!");
        return false;
    }
    return true;
}

// Update State (if required)
void TunaGamepad::update() {
    // Placeholder: Update logic if needed (e.g., pull latest data)
}

// Get Axis Value with Calibration Applied
float TunaGamepad::getAxis(Axis axis) const {
    if (!checkDriverStationJoystick()) { return 0.0f; }
    int logicalIndex = static_cast<int>(axis); // Convert enum to int
    
    // normalde 4 değil 6 olmalı benim gamepad biraz ilginç çalışıyor
    if (logicalIndex >= 0 && logicalIndex < 4) {
        int driverStationIndex = axisMapping[logicalIndex]; // Map to DS index
        return driverStation->getJoystickAxis(driverStationIndex) - axisOffsets[logicalIndex];
    } 
    // triggerlar için axis girdim 
    else if (logicalIndex == 4) {
        return driverStation->getJoystickButton(6) ? 1.0f : 0.0f;
    } else if (logicalIndex == 5) {
        return driverStation->getJoystickButton(7) ? 1.0f : 0.0f;
    }

    return 0.0f; // Default if invalid index
}

// Get Button State
bool TunaGamepad::getButton(Button button) const {
    if (!checkDriverStationJoystick()) { return 0.0f; }
    int logicalIndex = static_cast<int>(button); // Convert enum to int

    // Benim gamepad biraz ilginç çalışıyor ondan biraz değişiklik yapmam gerekiyor
    if (4 <= logicalIndex && logicalIndex <= 7) { 
        float dpadValue = driverStation->getJoystickAxis(9); 

        // DpadUp
        if (logicalIndex == 4 && -1 <= dpadValue && dpadValue < -0.5) { 
            return true;
        }
        // DpadDown
        if (logicalIndex == 5 && 0 < dpadValue && dpadValue < 0.5) { 
            return true;
        }
        // DpadLeft
        if (logicalIndex == 6 && 0.5 < dpadValue && dpadValue <= 1) {
            return true;
        }
        // DpadRight
        if (logicalIndex == 7 && -0.5 < dpadValue && dpadValue < 0) { 
            return true;
        }
        return false;
    }

    if (logicalIndex >= 0 && logicalIndex < 14) {
        int driverStationIndex = buttonMapping[logicalIndex]; // Map to DS index
        return driverStation->getJoystickButton(driverStationIndex);
    }
    return false; // Default if invalid index
}

// Calibrate Axes
void TunaGamepad::calibrate() {
    Serial.println("Calibrating Axes...");

    for (int i = 0; i < 4; i++) {
        axisOffsets[i] = driverStation->getJoystickAxis(axisMapping[i]); // Save mapped raw values as offsets
        Serial.print("Axis ");
        Serial.print(i);
        Serial.print(" Offset: ");
        Serial.println(axisOffsets[i]);
    }

    Serial.println("Calibration Complete!");
}

// Debug Print
void TunaGamepad::debugPrint() const {
    Serial.println("=== TunaGamepad State ===");

    if (!driverStation) {
        Serial.println("DriverStation not initialized!");
        return;
    }

    Serial.print("Axes: ");
    Serial.print("LX: "); Serial.print(getAxis(Axis::LeftJoystickX));
    Serial.print(" | LY: "); Serial.print(getAxis(Axis::LeftJoystickY));
    Serial.print(" | RX: "); Serial.print(getAxis(Axis::RightJoystickX));
    Serial.print(" | RY: "); Serial.print(getAxis(Axis::RightJoystickY));
    Serial.print(" | LT: "); Serial.print(getAxis(Axis::LT));
    Serial.print(" | RT: "); Serial.println(getAxis(Axis::RT));

    Serial.println("Buttons:");
    Serial.print("A: "); Serial.print(getButton(Button::A) ? "Pressed" : "Released");
    Serial.print(" | B: "); Serial.print(getButton(Button::B) ? "Pressed" : "Released");
    Serial.print(" | X: "); Serial.print(getButton(Button::X) ? "Pressed" : "Released");
    Serial.print(" | Y: "); Serial.print(getButton(Button::Y) ? "Pressed" : "Released");
    Serial.println("==========================");
}
