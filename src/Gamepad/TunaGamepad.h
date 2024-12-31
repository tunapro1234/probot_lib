#ifndef TUNA_GAMEPAD_H
#define TUNA_GAMEPAD_H

#include "../BaseClass/BaseGamepad.h"
#include "../DriverStation/DriverStationNodeMCU.h"

#define TUNA_GAMEPAD_AXIS_COUNT 10
#define TUNA_GAMEPAD_BUTTON_COUNT 12

class TunaGamepad : public BaseGamepad {
private:
    const DriverStation* driverStation; // Pointer to DriverStation

    // Axis Calibration Offsets
    float axisOffsets[4]; // Stores calibration offsets for 6 axes

    // Private Mapping Tables
    static constexpr int axisMapping[6] = {
        0, // LeftJoystickX → DriverStation Axis 0
        1, // LeftJoystickY → DriverStation Axis 1
        2, // RightJoystickX → DriverStation Axis 2
        5, // RightJoystickY → DriverStation Axis 3
        -1, // LT → DriverStation Axis 4
        -1, // RT → DriverStation Axis 5
    };

    static constexpr int buttonMapping[14] = {
        2, // A → DriverStation Button 0
        1, // B → DriverStation Button 1
        3, // X → DriverStation Button 2
        0, // Y → DriverStation Button 3
        -1, // DPadUp → DriverStation Button 4
        -1, // DPadDown → DriverStation Button 5
        -1, // DPadLeft → DriverStation Button 6
        -1, // DPadRight → DriverStation Button 7
        4, // LB → DriverStation Button 8
        5, // LT → DriverStation Button 9
        8, // RB → DriverStation Button 10
        9, // RT → DriverStation Button 11
        10, // Options → DriverStation Button 12
        11  // Start → DriverStation Button 13
    };


public:
    explicit TunaGamepad(const DriverStation* ds);

    void update() override;
    float getAxis(Axis axis) const override;
    bool getButton(Button button) const override;
    bool checkDriverStationJoystick() const override;

    void calibrate() override;
    void debugPrint() const override;
};

#endif // TUNAGAMEPAD_H
