#pragma once
#include <cmath>

namespace probot::control {
  struct Pose2d {
    float x;
    float y;
    float heading; // radians
    constexpr Pose2d(float ix = 0.0f, float iy = 0.0f, float iHeading = 0.0f)
    : x(ix), y(iy), heading(iHeading) {}
  };

  struct ChassisSpeeds {
    float vx;    // forward velocity (units/s)
    float vy;    // lateral velocity (units/s)
    float omega; // angular velocity (rad/s)
    constexpr ChassisSpeeds(float ivx = 0.0f, float ivy = 0.0f, float iomega = 0.0f)
    : vx(ivx), vy(ivy), omega(iomega) {}
  };

  inline float normalizeAngle(float angle){
    return std::atan2(std::sin(angle), std::cos(angle));
  }

  inline Pose2d computeRobotRelativeError(const Pose2d& reference, const Pose2d& current){
    float dx = reference.x - current.x;
    float dy = reference.y - current.y;
    float headingDiff = normalizeAngle(reference.heading - current.heading);
    float cosTheta = std::cos(current.heading);
    float sinTheta = std::sin(current.heading);
    float xError = cosTheta * dx + sinTheta * dy;
    float yError = -sinTheta * dx + cosTheta * dy;
    return Pose2d(xError, yError, headingDiff);
  }
}
