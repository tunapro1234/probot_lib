#include "FeedbackTankdrive.h"

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

void FeedbackTankdrive::setMotorVelocities(float leftVelocity, float rightVelocity)
{
  float leftMotorRPM = (leftVelocity * 60.0) / (M_PI * wheelDiameterCM) * gearboxRatio;
  float rightMotorRPM = (rightVelocity * 60.0) / (M_PI * wheelDiameterCM) * gearboxRatio;

  leftMotor.setVelocityTarget(leftMotorRPM);
  rightMotor.setVelocityTarget(rightMotorRPM);
}
