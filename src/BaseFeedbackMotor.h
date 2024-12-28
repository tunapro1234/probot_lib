#ifndef BaseFeedbackMotor_h
#define BaseFeedbackMotor_h

#include "BaseMotorController.h"
#include "BaseEncoder.h"
#include "PIDController.h"  // Adjust the path if necessary

// Enumeration for drive modes
enum class DriveMode {
    PowerDrive,
    SpeedDrive,
    PositionalDrive
};

class BaseFeedbackMotor : public BaseMotorController {
public:
    BaseFeedbackMotor(int pwmPin, int dirPin1, int dirPin2, BaseEncoder& encoder);

    // Initialize the motor and encoder
    void begin() override;

    // Set motor speed (-1.0 to 1.0) or target based on mode
    void setTarget(float target);

    // Update motor control based on current mode
    void update();

    // Set the current drive mode
    void setDriveMode(DriveMode mode);

    // Get the current position from the encoder
    long getPosition();

    // Set PID parameters
    void setPID(float kp, float ki, float kd);

private:
    BaseEncoder& encoder;
    DriveMode currentMode;
    float targetValue;
    PIDController pid;

    unsigned long lastUpdateTime;
};

#endif
