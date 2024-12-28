#include "PIDController.h"

// Existing Constructor
PIDController::PIDController(float kp, float ki, float kd)
    : kp(kp), ki(ki), kd(kd), setpoint(0), integral(0), previousError(0) {}

// New Constructor with PIDCoefficients
PIDController::PIDController(const PIDCoefficients& coefficients)
    : kp(coefficients.getKp()), ki(coefficients.getKi()), kd(coefficients.getKd()), 
      setpoint(0), integral(0), previousError(0) {}

// Overloaded setTunings with individual parameters
void PIDController::setTunings(float kp, float ki, float kd) {
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
}

// Overloaded setTunings with PIDCoefficients
void PIDController::setTunings(const PIDCoefficients& coefficients) {
    this->kp = coefficients.getKp();
    this->ki = coefficients.getKi();
    this->kd = coefficients.getKd();
}

// Compute PID Output
float PIDController::compute(float input) {
    float error = setpoint - input;
    integral += error;
    float derivative = error - previousError;
    previousError = error;

    return (kp * error) + (ki * integral) + (kd * derivative);
}

// Reset State
void PIDController::reset() {
    integral = 0;
    previousError = 0;
}
