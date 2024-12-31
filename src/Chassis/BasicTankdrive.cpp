#include "BasicTankdrive.h"

BasicTankdrive::BasicTankdrive(BaseMotorController& lm, BaseMotorController& rm)
  : leftMotor(lm), rightMotor(rm) {}

void BasicTankdrive::begin() {
  leftMotor.begin();
  rightMotor.begin();
  stop();
}

void BasicTankdrive::setMotorPowers(float leftPower, float rightPower) {
  // Güçleri sınırlıyoruz
  leftPower = constrain(leftPower, -1.0, 1.0);
  rightPower = constrain(rightPower, -1.0, 1.0);

  leftMotor.setPower(leftPower);
  rightMotor.setPower(rightPower);
}

void BasicTankdrive::drive(float forwardSpeed, float rotationalSpeed) {
  // Hız ve dönüş hızını motor güçlerine dönüştür
  float leftPower = forwardSpeed + rotationalSpeed;
  float rightPower = forwardSpeed - rotationalSpeed;

  // Normalize ederek değerlerin sınırları aşmamasını sağla
  float maxPower = max(abs(leftPower), abs(rightPower));
  if (maxPower > 1.0) {
    leftPower /= maxPower;
    rightPower /= maxPower;
  }

  setMotorPowers(leftPower, rightPower);
}

void BasicTankdrive::stop() {
  leftMotor.stop();
  rightMotor.stop();
}
