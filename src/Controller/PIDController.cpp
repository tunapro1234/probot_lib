#include "PIDController.h"
#include <math.h>

// Constructor
PIDController::PIDController(const PIDCoefficients& coeffs, float tolerance)
    : coefficients(coeffs), setpoint(0.0f), integral(0.0f), lastError(0.0f),
      tolerance(tolerance) {}

// Set PID Coefficients
void PIDController::setCoefficients(const PIDCoefficients& coeffs) {
    coefficients = coeffs;
}

// Set Setpoint
void PIDController::setSetpoint(float setpoint) {
    this->setpoint = setpoint;
}

// Get Setpoint
float PIDController::getSetpoint() {
    return this->setpoint;
}

// Reset State
void PIDController::reset() {
    integral = 0.0f;
    lastError = 0.0f;
}

// calculate Control Output
float PIDController::calculate(float currentMeasurement) {
    float error = setpoint - currentMeasurement;
    integral += error;
    float derivative = error - lastError;
    lastError = error;

    return (coefficients.getKp() * error) + 
           (coefficients.getKi() * integral) + 
           (coefficients.getKd() * derivative);
}

// Set Position and Velocity Tolerance
void PIDController::setTolerance(float tolerance) {
    this->tolerance = tolerance;
}

// Set Position and Velocity Tolerance
float PIDController::getTolerance() {
    return this->tolerance;
}

// Check if at Setpoint
bool PIDController::isAtSetpoint(float currentMeasurement) const {
    float positionError = fabs(setpoint - currentMeasurement);

    return (positionError <= tolerance);
}

// Get Current Coefficients
PIDCoefficients PIDController::getCoefficients() const {
    return coefficients;
}
