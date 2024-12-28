#ifndef BaseFeedbackMotor_h
#define BaseFeedbackMotor_h

#include "BaseMotorController.h"
#include "BaseEncoder.h"
#include "../Controllers/PIDController.h"
#include "../UpdateHelper/UpdateHelper.h"

// Enumeration for drive modes
enum class DriveMode {
    PowerDrive,
    SpeedDrive,
    PositionalDrive
};

class BaseFeedbackMotor {
public:
    // Constructor with dependencies
    BaseFeedbackMotor(BaseMotorController& motorController, BaseEncoder& encoder, PIDController& pidController,
                     unsigned long updateInterval = 100, unsigned long maxDelay = 200)
        : motor(motorController), encoder(encoder), pid(pidController),
          currentMode(DriveMode::PowerDrive), targetValue(0.0f),
          updateHelper(updateInterval, maxDelay) {}

    virtual ~BaseFeedbackMotor() {}

    // Initialize the motor and encoder
    virtual void begin() = 0;

    // Set target based on mode
    virtual void setTarget(float target) = 0;

    // Update motor control based on current mode
    virtual void update() = 0;

    // Set the current drive mode
    void setDriveMode(DriveMode mode) {
        currentMode = mode;
        pid.reset();
    }

    // Get the current position from the encoder
    long getPosition() {
        return encoder.getCount();
    }

    // Set PID parameters
    void setPID(float kp, float ki, float kd) {
        pid.setTunings(kp, ki, kd);
    }

protected:
    BaseMotorController& motor;
    BaseEncoder& encoder;
    PIDController& pid;
    DriveMode currentMode;
    float targetValue;
    UpdateHelper updateHelper;

    // Compute PID output
    float computePID(float currentValue) {
        pid.setSetpoint(targetValue);
        return pid.compute(currentValue);
    }
};

#endif
