#ifndef GAMEPAD_H
#define GAMEPAD_H

// Axis Enumeration
enum class Axis {
    LeftJoystickX,
    LeftJoystickY,
    RightJoystickX,
    RightJoystickY,
    LT,
    RT
};

// Button Enumeration
enum class Button {
    A,
    B,
    X,
    Y,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    LB,
    RB,
    Options,
    Start,
    LeftJoystickTrigger,
    RightJoystickTrigger
};

class BaseGamepad {
public:
    virtual ~BaseGamepad() = default;

    // Update gamepad state (e.g., from network or hardware)
    virtual void update() = 0;

    // Calibrate Gamepad (if applicable)
    virtual void calibrate() = 0;
    
    // Check if the driver station has the correct joystick
    virtual bool checkDriverStationJoystick() const = 0;

    // Get Axis Value (-1.0 to 1.0)
    virtual float getAxis(Axis axis) const = 0;

    // Get Button State (true = pressed, false = released)
    virtual bool getButton(Button button) const = 0;

    // Debug Print State
    virtual void debugPrint() const = 0;
};

#endif // GAMEPAD_H
