#pragma once
#include <cmath>
#include <probot/control/geometry.hpp>
#include <probot/control/kinematics/mecanum_drive_kinematics.hpp>

namespace probot::control::odometry {
  class MecanumDriveOdometry {
  public:
    MecanumDriveOdometry(const probot::control::Pose2d& initial = probot::control::Pose2d())
    : pose_(initial), lastTimestamp_(0.0f), hasPrev_(false) {}

    void reset(const probot::control::Pose2d& pose, float timestamp){
      pose_ = pose;
      lastTimestamp_ = timestamp;
      hasPrev_ = false;
    }

    probot::control::Pose2d update(float timestamp,
                                   const probot::control::kinematics::WheelPositions4& wheelPositions,
                                   const probot::control::kinematics::MecanumDriveKinematics& kin){
      float dt = timestamp - lastTimestamp_;
      if (!hasPrev_){
        prevPositions_ = wheelPositions;
        lastTimestamp_ = timestamp;
        hasPrev_ = true;
        return pose_;
      }
      if (dt <= 0.0f) dt = 1e-3f;
      probot::control::kinematics::WheelSpeeds4 speeds {
        (wheelPositions.frontLeft - prevPositions_.frontLeft) / dt,
        (wheelPositions.frontRight - prevPositions_.frontRight) / dt,
        (wheelPositions.rearLeft - prevPositions_.rearLeft) / dt,
        (wheelPositions.rearRight - prevPositions_.rearRight) / dt
      };
      auto chassis = kin.toChassisSpeeds(speeds);
      float cosH = std::cos(pose_.heading);
      float sinH = std::sin(pose_.heading);
      pose_.x += (chassis.vx * cosH - chassis.vy * sinH) * dt;
      pose_.y += (chassis.vx * sinH + chassis.vy * cosH) * dt;
      pose_.heading = probot::control::normalizeAngle(pose_.heading + chassis.omega * dt);
      lastTimestamp_ = timestamp;
      prevPositions_ = wheelPositions;
      return pose_;
    }

    const probot::control::Pose2d& pose() const { return pose_; }

    void setHeading(float heading){ pose_.heading = probot::control::normalizeAngle(heading); }

  private:
    probot::control::Pose2d pose_;
    float lastTimestamp_;
    bool hasPrev_;
    probot::control::kinematics::WheelPositions4 prevPositions_{};
  };
}
