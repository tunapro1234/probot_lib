#ifndef BasicTankdrive_h
#define BasicTankdrive_h

#include "../BaseClass/BaseMotorController.h"

class BasicTankdrive {
  public:
    BasicTankdrive(BaseMotorController& leftMotor, BaseMotorController& rightMotor);
    
    void begin();
    void drive(float forwardSpeed, float rotationalSpeed); // -1.0 to 1.0
    void stop();
  
  private:
    BaseMotorController& leftMotor;
    BaseMotorController& rightMotor;

    void setMotorPowers(float leftPower, float rightPower); // İçsel motor gücü ayarı
};

#endif
