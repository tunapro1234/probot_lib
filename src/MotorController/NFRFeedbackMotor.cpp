#include "NFRFeedbackMotor.h"

// Constructor
NFRFeedbackMotor::NFRFeedbackMotor(
                                   const unsigned int &id,
                                   unsigned long updateInterval = NFRFM_DEFAULT_UPDATE_INTERVAL,
                                   unsigned long maxDelay = NFRFM_DEFAULT_MAX_DELAY,
                                   float positionTolerance = NFRFM_DEFAULT_POS_TOL,
                                   float velocityTolerance = NFRFM_DEFAULT_VEL_POL
                                   )
    : positionCoeffs(NFRFM_POS_PID_COEFS),
      velocityCoeffs(NFRFM_VEL_PID_COEFS),
      pid(velocityCoeffs, velocityTolerance),
      updateHelper(updateInterval, maxDelay),
      positionalMode(false),
      positionTolerance(positionTolerance),
      velocityTolerance(velocityTolerance)
{
}

// Initialization
void NFRFeedbackMotor::begin()
{
    // motor.begin();
    // encoder.begin();
    pid.reset();
    updateHelper.setLastOutput(0.0f);
    positionalMode = false; // Default to velocity mode
}

void NFRFeedbackMotor::setMotorPower(float power) {
    // to be implemented (when the motor controller finishes)
}

long NFRFeedbackMotor::getEncoderCounts() {
    // to be implemented (when the motor controller finishes)
    return 0;
}

// Update Loop
void NFRFeedbackMotor::update()
{
    if (!updateHelper.shouldUpdate())
    {
        // motor.setPower(updateHelper.getLastOutput());
        setMotorPower(updateHelper.getLastOutput());
        return;
    }

    updateHelper.checkTimeout();

    if (positionalMode)
    {
        handlePositionMode();
    }
    else
    {
        handleVelocityMode();
    }
}

// Reset Motor State
void NFRFeedbackMotor::reset()
{
    pid.reset();
    // motor.setPower(0);
    setMotorPower(0);
    positionalMode = false;
    updateHelper.setLastOutput(0.0f);
}

// Set Position Target
void NFRFeedbackMotor::setPositionTarget(long target)
{
    if (!positionalMode)
    {
        positionalMode = true;
        pid.setCoefficients(positionCoeffs);
        pid.reset();
    }
    pid.setSetpoint(target);
}

// Set Velocity Target (in RPM)
void NFRFeedbackMotor::setVelocityTarget(float target)
{
    if (positionalMode)
    {
        positionalMode = false;
        pid.setCoefficients(velocityCoeffs);
        pid.reset();
    }
    pid.setSetpoint(target);
}

// Check if at Target
bool NFRFeedbackMotor::isAtTarget()
{
    if (positionalMode)
    {
        return pid.isAtSetpoint(getPosition()); // Velocity is ignored in position mode
    }
    else
    {
        return pid.isAtSetpoint(getVelocity());
    }
}

// Set Position Tolerance
void NFRFeedbackMotor::setPositionTolerance(unsigned int tolerance)
{
    positionTolerance = tolerance;
    pid.setTolerance(positionTolerance);
}

// Set Velocity Tolerance
void NFRFeedbackMotor::setVelocityTolerance(float tolerance)
{
    velocityTolerance = tolerance;
    pid.setTolerance(positionTolerance);
}

// Get Current Velocity (RPM)
float NFRFeedbackMotor::getVelocity()
{
    // float deltaTicks = static_cast<float>(encoder.getCount());
    float deltaTicks = static_cast<float>(getEncoderCounts());
    float deltaTime = static_cast<float>(updateHelper.getLastUpdateTime()) / 1000.0f; // ms to seconds
    return (deltaTicks / NFRFM_CPR) * (60.0f / deltaTime);
}

// Get Current Position (Ticks)
float NFRFeedbackMotor::getPosition()
{
    // return static_cast<float>(encoder.getCount());
    return static_cast<float>(getEncoderCounts());
}

// Get Velocity Target
float NFRFeedbackMotor::getVelocityTarget()
{
    return positionalMode ? 0.0 : pid.getSetpoint();
}

// Get Position Target
float NFRFeedbackMotor::getPositionTarget()
{
    return positionalMode ? pid.getSetpoint() : 0.0;
}

// Set Position PID Coefficients
void NFRFeedbackMotor::setPositionPID(const PIDCoefficients &coeffs)
{
    positionCoeffs = coeffs;
    if (positionalMode)
    {
        pid.setCoefficients(positionCoeffs);
    }
}

// Set Velocity PID Coefficients
void NFRFeedbackMotor::setVelocityPID(const PIDCoefficients &coeffs)
{
    velocityCoeffs = coeffs;
    if (!positionalMode)
    {
        pid.setCoefficients(velocityCoeffs);
    }
}

// Check if in Positional Mode
bool NFRFeedbackMotor::isPositional() const
{
    return positionalMode;
}

// Check if in Velocity Mode
bool NFRFeedbackMotor::isVelocity() const
{
    return !positionalMode;
}

// Apply PID Coefficients
void NFRFeedbackMotor::applyCoefficients(const PIDCoefficients &coeffs)
{
    pid.setCoefficients(coeffs);
    pid.reset();
}

// Handle Position Mode
void NFRFeedbackMotor::handlePositionMode()
{
    // float currentPosition = static_cast<float>(encoder.getCount());
    float currentPosition = static_cast<float>(getEncoderCounts());
    float output = pid.calculate(currentPosition);
    // motor.setPower(output);
    setMotorPower(output);
    updateHelper.setLastOutput(output);
}

// Handle Velocity Mode (RPM-Based)
void NFRFeedbackMotor::handleVelocityMode()
{
    float currentSpeed = getVelocity();
    float output = pid.calculate(currentSpeed);
    // motor.setPower(output);
    setMotorPower(output);
    updateHelper.setLastOutput(output);
}
