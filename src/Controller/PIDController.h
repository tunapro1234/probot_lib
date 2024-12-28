#ifndef PIDController_h
#define PIDController_h

#include "PIDCoefficients.h"

class PIDController {
public:
    // Existing Constructor
    PIDController(float kp = 0.0, float ki = 0.0, float kd = 0.0);

    // New Constructor with PIDCoefficients
    PIDController(const PIDCoefficients& coefficients);

    // Set PID Tunings
    void setTunings(float kp, float ki, float kd);
    void setTunings(const PIDCoefficients& coefficients); // Overloaded method

    // Compute Output
    float compute(float input);

    // Reset Controller State
    void reset();

private:
    float kp; // Proportional Gain
    float ki; // Integral Gain
    float kd; // Derivative Gain

    float setpoint;   // Desired Value
    float integral;   // Integral Term
    float previousError; // Last Error
};

#endif
