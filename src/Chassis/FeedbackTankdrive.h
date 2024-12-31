#ifndef FeedbackTankdrive_h
#define FeedbackTankdrive_h

#include "../BaseClass/BaseFeedbackMotor.h"

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

    void setMotorVelocities(float leftVelocity, float rightVelocity);

  private:
    float gearboxRatio;
    float wheelDiameterCM;
    BaseFeedbackMotor& leftMotor;
    BaseFeedbackMotor& rightMotor;
};

#endif
