#ifndef BasicMotorController_h
#define BasicMotorController_h

#include "../BaseClass/BaseMotorController.h"

class BasicMotorController : public BaseMotorController {
  public:
    BasicMotorController(int pwmPin, int dirPin1, int dirPin2, bool isReversed = false);
    void begin() override;
    void setPower(float speed) override; // -1.0 to 1.0
    float getPower() override; // -1.0 to 1.0
    void stop() override;
  
  private:
    bool isReversed;
    float speed;
    int pwmPin;
    int dirPin1;
    int dirPin2;
};

#endif
