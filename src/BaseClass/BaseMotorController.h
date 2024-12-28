#ifndef BaseMotorController_h
#define BaseMotorController_h

#include <Arduino.h>

class BaseMotorController {
  public:
    virtual void begin() = 0;
    virtual void setSpeed(float speed) = 0; // -1.0 to 1.0
    virtual void stop() = 0;
};

#endif
