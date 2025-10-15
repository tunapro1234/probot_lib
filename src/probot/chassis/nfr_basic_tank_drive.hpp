#pragma once
#include <probot/chassis/basic_tank_drive.hpp>

namespace probot::chassis {
  class NfrBasicTankDrive : public probot::drive::BasicTankDrive {
  public:
    explicit NfrBasicTankDrive(probot::control::IMotorController* left,
                               probot::control::IMotorController* right)
    : probot::drive::BasicTankDrive(left, right) {
      setWheelCircumference(kWheelCircumference);
      setTrackWidth(kTrackWidth);
    }

  protected:
    static constexpr float kWheelCircumference = 0.314f; // meters
    static constexpr float kTrackWidth        = 0.55f;  // meters
  };
}
