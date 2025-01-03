#ifndef BaseFeedbackMotor_h
#define BaseFeedbackMotor_h

#include "../Controller/PIDCoefficients.h"

class BaseFeedbackMotor {
public:
    virtual ~BaseFeedbackMotor() {}

    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void reset() = 0;
    virtual void stop() = 0;

    virtual void setPositionTarget(long target) = 0;
    virtual void setVelocityTarget(float target) = 0;

    virtual bool isAtTarget() = 0;
    virtual void setPositionTolerance(unsigned int tolerance) = 0;
    virtual void setVelocityTolerance(float tolerance) = 0;

    virtual float getVelocity() = 0;
    virtual float getPosition() = 0;

    virtual float getVelocityTarget() = 0;
    virtual float getPositionTarget() = 0;

    // Set PID Coefficients
    virtual void setPositionPID(const PIDCoefficients& coeffs) = 0;
    virtual void setVelocityPID(const PIDCoefficients& coeffs) = 0;

    // Get Active Mode
    virtual bool isPositional() const = 0;
    virtual bool isVelocity() const = 0;
};

#endif
