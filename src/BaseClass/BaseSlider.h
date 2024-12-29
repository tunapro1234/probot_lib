#ifndef BaseSlider_h
#define BaseSlider_h

#include "BaseFeedbackMotor.h"

class BaseSlider {
public:
    virtual ~BaseSlider() {}
    
    // Initialize the slider
    virtual void begin() = 0;
    
    // Set target position
    virtual void setTargetPosition(long position) = 0;
    
    // Update slider state
    virtual void update() = 0;
    
    // Get current position
    virtual long getPosition() = 0;
};

#endif
