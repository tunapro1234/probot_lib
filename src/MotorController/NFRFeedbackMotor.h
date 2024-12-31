#ifndef NFRFeedbackMotor_h
#define NFRFeedbackMotor_h

#include "../BaseClass/BaseFeedbackMotor.h"
#include "../Controller/PIDController.h"
#include "../Controller/PIDCoefficients.h"
#include "../Util/UpdateHelper.h"

#define NFRFM_CPR 360
#define NFRFM_VEL_PID_COEFS (PIDCoefficients(1.0, 0.1, 0.05))
#define NFRFM_POS_PID_COEFS (PIDCoefficients(1.0, 0.1, 0.05))

#define NFRFM_DEFAULT_VEL_POL 10
#define NFRFM_DEFAULT_UPDATE_INTERVAL 10
#define NFRFM_DEFAULT_MAX_DELAY 10
#define NFRFM_DEFAULT_POS_TOL 10
#define NFRFM_DEFAULT_VEL_POL 10


class NFRFeedbackMotor : public BaseFeedbackMotor {
public:
    NFRFeedbackMotor(
        const unsigned int &id,
        unsigned long updateInterval = NFRFM_DEFAULT_UPDATE_INTERVAL, 
        unsigned long maxDelay = NFRFM_DEFAULT_MAX_DELAY,
        float positionTolerance = NFRFM_DEFAULT_POS_TOL,
        float velocityTolerance = NFRFM_DEFAULT_VEL_POL
    );

    void begin() override;
    void update() override;
    void reset() override;

    void setPositionTarget(long target) override;
    void setVelocityTarget(float target) override;

    bool isAtTarget() override;
    void setPositionTolerance(unsigned int tolerance) override;
    void setVelocityTolerance(float tolerance) override;

    float getVelocity() override;
    float getPosition() override;

    float getVelocityTarget() override;
    float getPositionTarget() override;

    void setPositionPID(const PIDCoefficients& coeffs) override;
    void setVelocityPID(const PIDCoefficients& coeffs) override;

    bool isPositional() const override;
    bool isVelocity() const override;

private:
    PIDController pid;

    PIDCoefficients positionCoeffs;
    PIDCoefficients velocityCoeffs;

    UpdateHelper updateHelper;

    bool positionalMode;
    float positionTolerance;
    float velocityTolerance;

    void applyCoefficients(const PIDCoefficients& coeffs);
    void handlePositionMode();
    void handleVelocityMode();

    void setMotorPower(float power);
    long getEncoderCounts();
};

#endif
