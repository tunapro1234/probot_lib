#ifndef NFRSlider_h
#define NFRSlider_h

#include "../BaseClass/BaseSlider.h"
#include "../Controller/PIDCoefficients.h"
#include "../MotorController/NFRFeedbackMotor.h"

#define NFRSLIDER_LENGTH_LIMIT 50.0f // Predefined length in cm
#define NFRSLIDER_COUNTS_PER_CM 100.0f // Predefined encoder counts per cm

class NFRSlider : public BaseSlider {
public:
    NFRSlider(int motorID);

    void begin() override;
    void setTargetLength(float length) override;
    void update() override;
    float getLength() override;
    float getTargetLength() override;
    float getMaxLength() override;
    bool isAtTarget() override;

private:
    NFRFeedbackMotor *motor;
    void safetyCheck();
};

#endif
