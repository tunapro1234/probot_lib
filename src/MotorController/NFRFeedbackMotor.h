#ifndef NFRFeedbackMotor_h
#define NFRFeedbackMotor_h

#include "../BaseClass/BaseFeedbackMotor.h"
#include "../Controller/PIDCoefficients.h"
#include "../Controller/PIDController.h"

class NFRFeedbackMotor : public BaseFeedbackMotor {
public:
    NFRFeedbackMotor(int id);

    void begin() override;
    void update() override;
    void reset() override;

    void setPositionTarget(float target) override;
    void setVelocityTarget(float target) override;

    bool isAtTarget() override;
    void setPositionTolerance(float tolerance) override;
    void setVelocityTolerance(float tolerance) override;

    float getVelocity() override;
    float getPosition() override;

    float getVelocityTarget() override;
    float getPositionTarget() override;

    void setPositionPID(const PIDCoefficients& coeffs) override;
    void setVelocityPID(const PIDCoefficients& coeffs) override;

    bool isPositional() const override;
    bool isVelocity() const override;

    int getID() const;

private:
    int motorID;
    PIDCoefficients positionPID;
    PIDCoefficients speedPID;
    PIDController pidController;
    float target;
    bool positionalMode;
};

#endif
