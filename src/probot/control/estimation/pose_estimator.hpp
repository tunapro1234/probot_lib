#pragma once
#include <deque>
#include <algorithm>
#include <probot/control/geometry.hpp>

namespace probot::control::estimation {
  struct TimestampedPose {
    float timestamp;
    probot::control::Pose2d pose;
    float trust;
  };

  class PoseEstimator {
  public:
    PoseEstimator(float visionBlend = 0.5f)
    : pose_(), lastTimestamp_(0.0f), blend_(visionBlend) {}

    void reset(const probot::control::Pose2d& pose, float timestamp){
      pose_ = pose;
      lastTimestamp_ = timestamp;
      measurements_.clear();
    }

    const probot::control::Pose2d& pose() const { return pose_; }

    void predict(float timestamp, const probot::control::ChassisSpeeds& speeds){
      float dt = timestamp - lastTimestamp_;
      if (dt < 0.0f){ dt = 0.0f; }
      float heading = pose_.heading;
      float cosH = std::cos(heading);
      float sinH = std::sin(heading);
      pose_.x += (speeds.vx * cosH - speeds.vy * sinH) * dt;
      pose_.y += (speeds.vx * sinH + speeds.vy * cosH) * dt;
      pose_.heading = probot::control::normalizeAngle(pose_.heading + speeds.omega * dt);
      lastTimestamp_ = timestamp;
      applyVision(timestamp);
    }

    void addVisionMeasurement(const probot::control::Pose2d& visionPose,
                              float timestamp,
                              float trust = 1.0f){
      measurements_.push_back({timestamp, visionPose, trust});
    }

    void setVisionBlend(float alpha){ blend_ = alpha; }
    float visionBlend() const { return blend_; }

  private:
    void applyVision(float currentTimestamp){
      while (!measurements_.empty() && measurements_.front().timestamp <= currentTimestamp){
        auto meas = measurements_.front();
        measurements_.pop_front();
        float alpha = std::clamp(blend_ * meas.trust, 0.0f, 1.0f);
        pose_.x = (1.0f - alpha) * pose_.x + alpha * meas.pose.x;
        pose_.y = (1.0f - alpha) * pose_.y + alpha * meas.pose.y;
        pose_.heading = probot::control::normalizeAngle((1.0f - alpha) * pose_.heading + alpha * meas.pose.heading);
      }
    }

    probot::control::Pose2d pose_;
    float lastTimestamp_;
    float blend_;
    std::deque<TimestampedPose> measurements_;
  };
}
