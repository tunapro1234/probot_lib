#include "NFRFeedbackMotor.h"
#include <Arduino.h>

// Constructor
NFRFeedbackMotor::NFRFeedbackMotor(int id)
    : motorID(id), positionPID(1.0, 0.1, 0.05), speedPID(1.0, 0.1, 0.05), pidController(1.0, 0.1, 0.05), target(0), positionalMode(true) {}

// Initialize Motor
void NFRFeedbackMotor::begin() {
    Serial.println(F("NFRFeedbackMotor Initialized with ID: "));
    Serial.println(motorID);
}

// Update Motor State
void NFRFeedbackMotor::update() {
    float currentFeedback = positionalMode ? getPosition() : getVelocity();
    float output = pidController.calculate(currentFeedback, target);
    Serial.print(F("Motor Output: "));
    Serial.println(output);
}

// Reset Motor
void NFRFeedbackMotor::reset() {
    target = 0;
    pidController.reset();
}

// Set Position Target
void NFRFeedbackMotor::setPositionTarget(long target) {
    this->target = target;
    positionalMode = true;
    pidController.setSetpoint(target);
}

// Set Velocity Target
void NFRFeedbackMotor::setVelocityTarget(float target) {
    this->target = target;
    positionalMode = false;
    pidController.setSetpoint(target);
}

// Check if at Target
bool NFRFeedbackMotor::isAtTarget() {
    return pidController.isAtSetpoint();
}

// Set Position Tolerance
void NFRFeedbackMotor::setPositionTolerance(unsigned int tolerance) {
    positionPID.tolerance = tolerance;
    pidController.setTolerance(tolerance);
}

// Set Velocity Tolerance
void NFRFeedbackMotor::setVelocityTolerance(float tolerance) {
    speedPID.tolerance = tolerance;
    pidController.setTolerance(tolerance);
}

// Get Current Velocity
float NFRFeedbackMotor::getVelocity() {
    return 0.0f;
}

// Get Current Position
float NFRFeedbackMotor::getPosition() {
    return target;
}

// Get Velocity Target
float NFRFeedbackMotor::getVelocityTarget() {
    return target;
}

// Get Position Target
float NFRFeedbackMotor::getPositionTarget() {
    return target;
}

// Set Position PID Coefficients
void NFRFeedbackMotor::setPositionPID(const PIDCoefficients& coeffs) {
    positionPID = coeffs;
    pidController.setPID(coeffs.kP, coeffs.kI, coeffs.kD);
}

// Set Velocity PID Coefficients
void NFRFeedbackMotor::setVelocityPID(const PIDCoefficients& coeffs) {
    speedPID = coeffs;
    pidController.setPID(coeffs.kP, coeffs.kI, coeffs.kD);
}

// Check if Positional Mode
bool NFRFeedbackMotor::isPositional() const {
    return positionalMode;
}

// Check if Velocity Mode
bool NFRFeedbackMotor::isVelocity() const {
    return !positionalMode;
}

// Get Motor ID
int NFRFeedbackMotor::getID() const {
    return motorID;
}