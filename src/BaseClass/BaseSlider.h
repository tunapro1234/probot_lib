#ifndef BaseSlider_h
#define BaseSlider_h

#include "BaseFeedbackMotor.h"

class BaseSlider {
public:
    virtual ~BaseSlider() {}
    
    // Initialize the slider
    virtual void begin() = 0;
    
    // Set target position
    virtual void setTargetLength(float length) = 0;
    
    // Update slider state
    virtual void update() = 0;
    
    // Get current position
    virtual float getLength() = 0;
    virtual float getTargetLength() = 0;

    // Get maximum length of the slider
    virtual float getMaxLength() = 0;

    // Check if slider is at the target
    virtual bool isAtTarget() = 0;
};

#endif
