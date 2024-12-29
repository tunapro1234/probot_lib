#ifndef GenericSlider_h
#define GenericSlider_h

#include "../BaseClass/BaseSlider.h"
#include "../BaseClass/BaseFeedbackMotor.h"

class GenericSlider : public BaseSlider {
public:
    // Constructor
    GenericSlider(BaseFeedbackMotor& motor, float lengthLimit, float countsPerCm);

    // Initialize the slider
    void begin() override;

    // Set target length (in cm)
    void setTargetLength(float length) override;

    // Update slider state
    void update() override;

    // Get current length (in cm)
    float getLength() override;

    // Get target length (in cm)
    float getTargetLength() override;

    // Get maximum length (in cm)
    float getMaxLength() override;

    // Check if slider is at the target
    bool isAtTarget() override;

    // Map ticks to length (in cm)
    float mapTicksToLength(float ticks);

    // Map length (in cm) to ticks
    float mapLengthToTicks(float length);

private:
    BaseFeedbackMotor& motor;
    float lengthLimit; // Maximum allowable length for the slider (in cm)
    float countsPerCm; // Encoder counts per centimeter

    // Internal Safety Check
    void safetyCheck();
};

#endif
