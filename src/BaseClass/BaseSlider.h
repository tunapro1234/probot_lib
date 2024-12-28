#ifndef BaseSlider_h
#define BaseSlider_h

#include "BaseFeedbackMotor.h"

// Enumeration for slider modes if needed
enum class SliderMode {
    Manual,
    Automatic
};

class BaseSlider {
public:
    // Constructor with dependencies
    BaseSlider(BaseFeedbackMotor& feedbackMotor, 
               unsigned long updateInterval = 100, 
               unsigned long maxDelay = 200, 
               bool enableLogging = false)
        : motor(feedbackMotor), 
          sliderMode(SliderMode::Manual),
          updateHelper(updateInterval, maxDelay, enableLogging) {}
    
    virtual ~BaseSlider() {}
    
    // Initialize the slider
    virtual void begin() = 0;
    
    // Set target position
    virtual void setTargetPosition(long position) = 0;
    
    // Update slider state
    virtual void update() = 0;
    
    // Get current position
    virtual long getPosition() = 0;
    
    // Set slider mode
    void setSliderMode(SliderMode mode) {
        sliderMode = mode;
    }
    
protected:
    BaseFeedbackMotor& motor;
    SliderMode sliderMode;
    UpdateHelper updateHelper;
};

#endif
