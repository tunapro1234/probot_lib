#include "BaseFeedbackMotor.h"

BaseFeedbackMotor::BaseFeedbackMotor(int pwmPin, int dirPin1, int dirPin2, BaseEncoder& enc)
    : BaseMotorController(pwmPin, dirPin1, dirPin2), encoder(enc),
      currentMode(DriveMode::PowerDrive), targetValue(0.0), pid(1.0, 0.0, 0.0),
      lastUpdateTime(0) {}

void BaseFeedbackMotor::begin() {
    BaseMotorController::begin();
    encoder.begin();
    pid.reset();
    lastUpdateTime = millis();
}

void BaseFeedbackMotor::setDriveMode(DriveMode mode) {
    currentMode = mode;
    pid.reset();
}

void BaseFeedbackMotor::setTarget(float target) {
    targetValue = target;
    pid.setSetpoint(target);
}

long BaseFeedbackMotor::getPosition() {
    return encoder.getCount();
}

void BaseFeedbackMotor::setPID(float kp, float ki, float kd) {
    pid.setTunings(kp, ki, kd);
}

void BaseFeedbackMotor::update() {
    switch (currentMode) {
        case DriveMode::PowerDrive:
            // Direct power control
            setSpeed(targetValue);
            break;

        case DriveMode::SpeedDrive:
            {
                // Example: Assuming encoder count relates to speed
                float currentSpeed = (float)encoder.getCount();
                float output = pid.compute(currentSpeed);
                setSpeed(output);
            }
            break;

        case DriveMode::PositionalDrive:
            {
                float currentPosition = (float)encoder.getCount();
                float output = pid.compute(currentPosition);
                setSpeed(output);
            }
            break;
    }
}
