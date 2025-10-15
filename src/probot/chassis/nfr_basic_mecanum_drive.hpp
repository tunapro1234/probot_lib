#pragma once
#include <probot/chassis/simple_mecanum.hpp>

namespace probot::chassis {
  class NfrBasicMecanumDrive : public SimpleMecanumDrive {
  public:
    NfrBasicMecanumDrive(probot::motor::IMotorDriver* frontLeft,
                         probot::motor::IMotorDriver* frontRight,
                         probot::motor::IMotorDriver* rearLeft,
                         probot::motor::IMotorDriver* rearRight)
    : SimpleMecanumDrive(frontLeft, frontRight, rearLeft, rearRight) {
      // Default: right side inverted for NFR mecanum chassis
      setInverted(false, true, false, true);
    }
  };
}
