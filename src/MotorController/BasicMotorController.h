#ifndef BasicMotorController_h
#define BasicMotorController_h

#include "../BaseClass/BaseMotorController.h"

class BasicMotorController : public BaseMotorController {
  public:
    BasicMotorController(int pwmPin, int dirPin1, int dirPin2);
    void begin() override;
    void setSpeed(float speed) override; // -1.0 to 1.0
    float getSpeed() override; // -1.0 to 1.0
    void stop() override;
  
  private:
    float speed;
    int pwmPin;
    int dirPin1;
    int dirPin2;
};

#endif
