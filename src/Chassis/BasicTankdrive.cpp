#include "BasicTankdrive.h"

BasicTankdrive::BasicTankdrive(BaseMotorController& lm, BaseMotorController& rm)
  : leftMotor(lm), rightMotor(rm) {}

void BasicTankdrive::begin() {
  leftMotor.begin();
  rightMotor.begin();
  stop();
}

void BasicTankdrive::setMotorPowers(float leftPower, float rightPower) {
  leftMotor.setPower(leftPower);
  rightMotor.setPower(rightPower);
}

void BasicTankdrive::stop() {
  leftMotor.stop();
  rightMotor.stop();
}
