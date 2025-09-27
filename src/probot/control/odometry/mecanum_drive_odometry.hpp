#pragma once
#include <probot/control/geometry.hpp>
#include <probot/control/kinematics/mecanum_drive_kinematics.hpp>

namespace probot::control::odometry {
  class MecanumDriveOdometry {
  public:
    MecanumDriveOdometry(const probot::control::Pose2d& initial = probot::control::Pose2d())
    : pose_(initial), lastTimestamp_(0.0f) {}

    void reset(const probot::control::Pose2d& pose, float timestamp){
      pose_ = pose;
      lastTimestamp_ = timestamp;
    }

    probot::control::Pose2d update(float timestamp,
                                   const probot::control::kinematics::WheelSpeeds4& wheelSpeeds,
                                   const probot::control::kinematics::MecanumDriveKinematics& kin){
      float dt = timestamp - lastTimestamp_;
      if (dt < 0.0f) dt = 0.0f;
      auto chassis = kin.toChassisSpeeds(wheelSpeeds);
      float cosH = std::cos(pose_.heading);
      float sinH = std::sin(pose_.heading);
      pose_.x += (chassis.vx * cosH - chassis.vy * sinH) * dt;
      pose_.y += (chassis.vx * sinH + chassis.vy * cosH) * dt;
      pose_.heading = probot::control::normalizeAngle(pose_.heading + chassis.omega * dt);
      lastTimestamp_ = timestamp;
      return pose_;
    }

    const probot::control::Pose2d& pose() const { return pose_; }

  private:
    probot::control::Pose2d pose_;
    float lastTimestamp_;
  };
}
