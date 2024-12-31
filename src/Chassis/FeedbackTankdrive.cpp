#include "FeedbackTankdrive.h"
#include <math.h>

FeedbackTankdrive::FeedbackTankdrive(
    float gearboxRatio,
    float wheelDiameterCM,
    BaseFeedbackMotor &lm,
    BaseFeedbackMotor &rm) : gearboxRatio(gearboxRatio), wheelDiameterCM(wheelDiameterCM), leftMotor(lm), rightMotor(rm) {}

void FeedbackTankdrive::begin()
{
  leftMotor.begin();
  rightMotor.begin();

  leftMotor.setVelocityTarget(0);
  rightMotor.setVelocityTarget(0);
}

void FeedbackTankdrive::update()
{
  leftMotor.update();
  rightMotor.update();
}

void FeedbackTankdrive::reset()
{
  leftMotor.reset();
  rightMotor.reset();
}

void FeedbackTankdrive::stop()
{
  leftMotor.stop();
  rightMotor.stop();
}

float FeedbackTankdrive::calculateRPM(float velocity) {
  return (velocity * 60.0) / (M_PI * wheelDiameterCM) * gearboxRatio;
}

void FeedbackTankdrive::setMotorVelocities(float leftVelocity, float rightVelocity)
{
  float leftMotorRPM = calculateRPM(leftVelocity);
  float rightMotorRPM = calculateRPM(rightVelocity);

  leftMotor.setVelocityTarget(leftMotorRPM);
  rightMotor.setVelocityTarget(rightMotorRPM);
}

void FeedbackTankdrive::drive(float forwardSpeed, float rotationalSpeed)
{
  // Motor hızlarını hesapla
  float leftVelocity = forwardSpeed + rotationalSpeed;
  float rightVelocity = forwardSpeed - rotationalSpeed;

  // Normalize ederek değerlerin sınırları aşmamasını sağla
  float maxVelocity = max(abs(leftVelocity), abs(rightVelocity));
  if (maxVelocity > 1.0) {
    leftVelocity /= maxVelocity;
    rightVelocity /= maxVelocity;
  }

  // Hesaplanan hızları motorlara uygula
  setMotorVelocities(leftVelocity, rightVelocity);
}
