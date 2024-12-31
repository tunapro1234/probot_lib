#include "BasicMecanumDrive.h"

BasicMecanumDrive::BasicMecanumDrive(
    BaseMotorController& fl,
    BaseMotorController& fr,
    BaseMotorController& bl,
    BaseMotorController& br
) : frontLeftMotor(fl), 
    frontRightMotor(fr), 
    backLeftMotor(bl), 
    backRightMotor(br) {}

void BasicMecanumDrive::begin() {
  frontLeftMotor.begin();
  frontRightMotor.begin();
  backLeftMotor.begin();
  backRightMotor.begin();

  frontLeftMotor.setPower(0);
  frontRightMotor.setPower(0);
  backLeftMotor.setPower(0);
  backRightMotor.setPower(0);
}

void BasicMecanumDrive::update() {
  // Bu metod, gelecekteki uzantılar için yer tutucu olarak kalabilir.
}

void BasicMecanumDrive::stop() {
  frontLeftMotor.stop();
  frontRightMotor.stop();
  backLeftMotor.stop();
  backRightMotor.stop();
}

void BasicMecanumDrive::applyMotorPowers(float frontLeft, float frontRight, float backLeft, float backRight) {
  // Güçleri normalize et (-1.0 ile 1.0 arasında tut)
  float maxPower = max(max(abs(frontLeft), abs(frontRight)), max(abs(backLeft), abs(backRight)));
  if (maxPower > 1.0) {
    frontLeft /= maxPower;
    frontRight /= maxPower;
    backLeft /= maxPower;
    backRight /= maxPower;
  }

  frontLeftMotor.setPower(frontLeft);
  frontRightMotor.setPower(frontRight);
  backLeftMotor.setPower(backLeft);
  backRightMotor.setPower(backRight);
}

void BasicMecanumDrive::drive(float xVelocity, float yVelocity, float rotationalSpeed) {
  // Motor güçlerini hesapla
  float frontLeftPower = yVelocity + xVelocity + rotationalSpeed;
  float backLeftPower = yVelocity - xVelocity + rotationalSpeed;
  float frontRightPower = yVelocity - xVelocity - rotationalSpeed;
  float backRightPower = yVelocity + xVelocity - rotationalSpeed;

  // Motor güçlerini uygula
  applyMotorPowers(frontLeftPower, frontRightPower, backLeftPower, backRightPower);
}
