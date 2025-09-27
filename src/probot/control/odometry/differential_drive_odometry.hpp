#pragma once
#include <probot/control/geometry.hpp>
#include <probot/control/kinematics/differential_drive_kinematics.hpp>

namespace probot::control::odometry {
  class DifferentialDriveOdometry {
  public:
    DifferentialDriveOdometry(const probot::control::Pose2d& initial = probot::control::Pose2d())
    : pose_(initial), prevLeft_(0.0f), prevRight_(0.0f) {}

    void reset(const probot::control::Pose2d& pose, float leftPosition, float rightPosition){
      pose_ = pose;
      prevLeft_ = leftPosition;
      prevRight_ = rightPosition;
    }

    probot::control::Pose2d update(float leftPosition, float rightPosition, float heading){
      float dl = leftPosition - prevLeft_;
      float dr = rightPosition - prevRight_;
      prevLeft_ = leftPosition;
      prevRight_ = rightPosition;
      float ds = (dl + dr) * 0.5f;
      pose_.heading = heading;
      pose_.x += ds * std::cos(heading);
      pose_.y += ds * std::sin(heading);
      return pose_;
    }

    const probot::control::Pose2d& pose() const { return pose_; }

  private:
    probot::control::Pose2d pose_;
    float prevLeft_;
    float prevRight_;
  };
}
