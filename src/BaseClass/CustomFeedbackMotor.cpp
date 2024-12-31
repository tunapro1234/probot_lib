#include "CustomFeedbackMotor.h"

// Constructor
CustomFeedbackMotor::CustomFeedbackMotor(BaseMotorController& motorController, 
                                         BaseEncoder& encoder,
                                         PIDCoefficients positionCoeffs, 
                                         PIDCoefficients velocityCoeffs,
                                         unsigned int ticksPerRotation,
                                         unsigned long updateInterval, 
                                         unsigned long maxDelay,
                                         float positionTolerance,
                                         float velocityTolerance
                                         )
    : motor(motorController),
      encoder(encoder),
      positionCoeffs(positionCoeffs),
      velocityCoeffs(velocityCoeffs),
      pid(velocityCoeffs, velocityTolerance), // Initialize PID with velocity coefficients
      updateHelper(updateInterval, maxDelay),
      positionalMode(false),
      positionTolerance(positionTolerance),
      velocityTolerance(velocityTolerance),
      ticksPerRotation(ticksPerRotation) {}

// Initialization
void CustomFeedbackMotor::begin() {
    motor.begin();
    encoder.begin();
    pid.reset();
    updateHelper.setLastOutput(0.0f);
    positionalMode = false; // Default to velocity mode
}

// Update Loop
void CustomFeedbackMotor::update() {
    if (!updateHelper.shouldUpdate()) {
        motor.setPower(updateHelper.getLastOutput());
        return;
    }

    updateHelper.checkTimeout();

    if (positionalMode) {
        handlePositionMode();
    } else {
        handleVelocityMode();
    }
}

// Reset Motor State
void CustomFeedbackMotor::reset() {
    stop();
    positionalMode = false;
}

// Stop Motor
void CustomFeedbackMotor::stop() {
    pid.reset();
    motor.setPower(0);
    updateHelper.setLastOutput(0.0f);
}

// Set Position Target
void CustomFeedbackMotor::setPositionTarget(long target) {
    if (!positionalMode) {
        positionalMode = true;
        pid.setCoefficients(positionCoeffs);
        pid.reset();
    }
    pid.setSetpoint(target);
}

// Set Velocity Target (in RPM)
void CustomFeedbackMotor::setVelocityTarget(float target) {
    if (positionalMode) {
        positionalMode = false;
        pid.setCoefficients(velocityCoeffs);
        pid.reset();
    }
    pid.setSetpoint(target);
}

// Check if at Target
bool CustomFeedbackMotor::isAtTarget() {
    if (positionalMode) {
        return pid.isAtSetpoint(getPosition()); // Velocity is ignored in position mode
    } else {
        return pid.isAtSetpoint(getVelocity());
    }
}

// Set Position Tolerance
void CustomFeedbackMotor::setPositionTolerance(unsigned int tolerance) {
    positionTolerance = tolerance;
    pid.setTolerance(positionTolerance);
}

// Set Velocity Tolerance
void CustomFeedbackMotor::setVelocityTolerance(float tolerance) {
    velocityTolerance = tolerance;
    pid.setTolerance(positionTolerance);
}

// Get Current Velocity (RPM)
float CustomFeedbackMotor::getVelocity() {
    float deltaTicks = static_cast<float>(encoder.getCount());
    float deltaTime = static_cast<float>(updateHelper.getLastUpdateTime()) / 1000.0f; // ms to seconds
    return (deltaTicks / ticksPerRotation) * (60.0f / deltaTime);
}

// Get Current Position (Ticks)
float CustomFeedbackMotor::getPosition() {
    return static_cast<float>(encoder.getCount());
}

// Get Velocity Target
float CustomFeedbackMotor::getVelocityTarget() {
    return positionalMode ? 0.0 : pid.getSetpoint();
}

// Get Position Target
float CustomFeedbackMotor::getPositionTarget() {
    return positionalMode ? pid.getSetpoint() : 0.0;
}

// Set Position PID Coefficients
void CustomFeedbackMotor::setPositionPID(const PIDCoefficients& coeffs) {
    positionCoeffs = coeffs;
    if (positionalMode) {
        pid.setCoefficients(positionCoeffs);
    }
}

// Set Velocity PID Coefficients
void CustomFeedbackMotor::setVelocityPID(const PIDCoefficients& coeffs) {
    velocityCoeffs = coeffs;
    if (!positionalMode) {
        pid.setCoefficients(velocityCoeffs);
    }
}

// Check if in Positional Mode
bool CustomFeedbackMotor::isPositional() const {
    return positionalMode;
}

// Check if in Velocity Mode
bool CustomFeedbackMotor::isVelocity() const {
    return !positionalMode;
}

// Apply PID Coefficients
void CustomFeedbackMotor::applyCoefficients(const PIDCoefficients& coeffs) {
    pid.setCoefficients(coeffs);
    pid.reset();
}

// Handle Position Mode
void CustomFeedbackMotor::handlePositionMode() {
    float currentPosition = static_cast<float>(encoder.getCount());
    float output = pid.calculate(currentPosition);
    motor.setPower(output);
    updateHelper.setLastOutput(output);
}

// Handle Velocity Mode (RPM-Based)
void CustomFeedbackMotor::handleVelocityMode() {
    float currentSpeed = getVelocity();
    float output = pid.calculate(currentSpeed);
    motor.setPower(output);
    updateHelper.setLastOutput(output);
}
