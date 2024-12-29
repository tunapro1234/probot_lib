#ifndef PIDController_h
#define PIDController_h

#include "PIDCoefficients.h"

class PIDController {
public:
    PIDController(const PIDCoefficients& coeffs, float tolerance=0);

    void setCoefficients(const PIDCoefficients& coeffs);
    void setSetpoint(float setpoint);
    void reset();

    float compute(float currentMeasurement);

    // Tolerance Methods
    void setTolerance(float tolerance);
    bool isAtSetpoint(float currentMeasurement) const;

    // Get Current Coefficients
    PIDCoefficients getCoefficients() const;

private:
    PIDCoefficients coefficients; // Store PID coefficients
    float setpoint;
    float integral;
    float lastError;

    float tolerance;
};

#endif
