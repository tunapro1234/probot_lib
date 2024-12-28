#ifndef BasicTankdrive_h
#define BasicTankdrive_h

#include "BaseMotorController.h"

class BasicTankdrive {
  public:
    BasicTankdrive(BaseMotorController& leftMotor, BaseMotorController& rightMotor);
    void begin();
    void setMotorPowers(float leftPower, float rightPower); // -1.0 to 1.0
    void stop();
  
  private:
    BaseMotorController& leftMotor;
    BaseMotorController& rightMotor;
};

#endif
