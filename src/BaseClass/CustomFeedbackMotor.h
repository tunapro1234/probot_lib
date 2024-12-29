#ifndef CustomFeedbackMotor_h
#define CustomFeedbackMotor_h

#include "../BaseClass/BaseFeedbackMotor.h"
#include "../Controller/PIDController.h"
#include "../Controller/PIDCoefficients.h"
#include "../Util/UpdateHelper.h"
#include "../BaseClass/BaseMotorController.h"
#include "../BaseClass/BaseEncoder.h"

class CustomFeedbackMotor : public BaseFeedbackMotor {
public:
    CustomFeedbackMotor(BaseMotorController& motorController, 
                        BaseEncoder& encoder,
                        PIDCoefficients positionCoeffs, 
                        PIDCoefficients velocityCoeffs,
                        unsigned int ticksPerRotation,
                        unsigned long updateInterval = 100, 
                        unsigned long maxDelay = 200,
                        float positionTolerance = 0.0f,
                        float velocityTolerance = 0.0f
                        );

    void begin() override;
    void update() override;
    void reset() override;

    void setPositionTarget(float target) override;
    void setVelocityTarget(float target) override;

    bool isAtTarget() override;
    void setPositionTolerance(float tolerance) override;
    void setVelocityTolerance(float tolerance) override;

    double getVelocity() override;
    double getPosition() override;

    double getVelocityTarget() override;
    double getPositionTarget() override;

    void setPositionPID(const PIDCoefficients& coeffs) override;
    void setVelocityPID(const PIDCoefficients& coeffs) override;

    bool isPositional() const override;
    bool isVelocity() const override;

private:
    BaseMotorController& motor;
    BaseEncoder& encoder;
    PIDController pid;

    PIDCoefficients positionCoeffs;
    PIDCoefficients velocityCoeffs;

    UpdateHelper updateHelper;

    bool positionalMode;
    float targetValue;
    float positionTolerance;
    float velocityTolerance;

    unsigned int ticksPerRotation; // CPR

    void applyCoefficients(const PIDCoefficients& coeffs);
    void handlePositionMode();
    void handleVelocityMode();
    double calculateRPM() const;
};

#endif
