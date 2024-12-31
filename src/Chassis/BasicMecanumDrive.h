#ifndef BasicMecanumDrive_h
#define BasicMecanumDrive_h

#include "BaseMotorController.h"
#include <Arduino.h>

class BasicMecanumDrive {
  public:
    BasicMecanumDrive(
      BaseMotorController& frontLeftMotor,
      BaseMotorController& frontRightMotor,
      BaseMotorController& backLeftMotor,
      BaseMotorController& backRightMotor
    );

    void begin();
    void update();
    void stop();

    // Hız tabanlı hareket
    void drive(float xVelocity, float yVelocity, float rotationalSpeed);

  private:
    BaseMotorController& frontLeftMotor;
    BaseMotorController& frontRightMotor;
    BaseMotorController& backLeftMotor;
    BaseMotorController& backRightMotor;

    void applyMotorPowers(float frontLeft, float frontRight, float backLeft, float backRight);
};

#endif
