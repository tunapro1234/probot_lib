#ifndef FeedbackTankdrive_h
#define FeedbackTankdrive_h

#include "../BaseClass/BaseFeedbackMotor.h"
#include <Arduino.h>
 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class FeedbackTankdrive {
  public:
    FeedbackTankdrive(
      float gearboxRatio,
      float wheelDiameterCM,
      BaseFeedbackMotor& leftMotor,
      BaseFeedbackMotor& rightMotor
    );
    void begin();
    void update();
    void reset();
    void stop();

    void drive(float forwardSpeed, float rotationalSpeed);
    void setMotorVelocities(float leftVelocity, float rightVelocity); // Eski metod

  private:
    float gearboxRatio;
    float wheelDiameterCM;
    BaseFeedbackMotor& leftMotor;
    BaseFeedbackMotor& rightMotor;

    float calculateRPM(float velocity); // Yardımcı metod
};

#endif
