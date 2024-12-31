#ifndef GenericSlider_h
#define GenericSlider_h

#include "../BaseClass/BaseSlider.h"
#include "../MotorController/NFRFeedbackMotor.h"

#define NFRSLIDER_LENGTH_LIMIT 50.0f // Predefined length in cm
#define NFRSLIDER_COUNTS_PER_CM 100.0f // Predefined encoder counts per cm
#define NFRSLIDER_POSITION_PID_COEFFICIENTS PIDCoefficients(1.0, 0.1, 0.05)

class NFRSlider : public BaseSlider {
public:
    NFRSlider(const unsigned int &motorID);

    void begin() override;
    void setTargetLength(float length) override;
    void update() override;
    float getLength() override;
    float getTargetLength() override;
    float getMaxLength() override;
    bool isAtTarget() override;
    float mapTicksToLength(float ticks);
    float mapLengthToTicks(float length);

private:
    NFRFeedbackMotor *motor;
    void safetyCheck();
};

#endif
