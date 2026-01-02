#pragma once
#include <utility>
#include <probot/control/geometry.hpp>

namespace probot::control::kinematics {
  class DifferentialDriveKinematics {
  public:
    explicit DifferentialDriveKinematics(float trackWidth)
    : trackWidth_(trackWidth > 0.0f ? trackWidth : 1.0f) {}

    float trackWidth() const { return trackWidth_; }
    void setTrackWidth(float w){ trackWidth_ = w > 0.0f ? w : 1.0f; }

    inline std::pair<float,float> toWheelSpeeds(const probot::control::ChassisSpeeds& speeds) const {
      float left = speeds.vx - speeds.omega * (trackWidth_ * 0.5f);
      float right = speeds.vx + speeds.omega * (trackWidth_ * 0.5f);
      return {left, right};
    }

    inline probot::control::ChassisSpeeds toChassisSpeeds(float leftSpeed, float rightSpeed) const {
      float vx = (leftSpeed + rightSpeed) * 0.5f;
      float omega = (rightSpeed - leftSpeed) / trackWidth_;
      return probot::control::ChassisSpeeds(vx, 0.0f, omega);
    }

  private:
    float trackWidth_;
  };
}
