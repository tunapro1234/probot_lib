#pragma once
#include <cmath>
#include <algorithm>
#include <probot/control/geometry.hpp>

namespace probot::control::trajectory {
  class RamseteController {
  public:
    RamseteController(float b = 2.0f, float zeta = 0.7f)
    : b_(b), zeta_(zeta), toleranceX_(0.0f), toleranceY_(0.0f), toleranceTheta_(0.0f) {}

    void setB(float b){ b_ = b; }
    void setZeta(float zeta){ zeta_ = zeta; }
    void setTolerance(float xTol, float yTol, float thetaTol){
      toleranceX_ = std::max(0.0f, xTol);
      toleranceY_ = std::max(0.0f, yTol);
      toleranceTheta_ = std::max(0.0f, thetaTol);
    }

    ChassisSpeeds calculate(const Pose2d& current,
                             const Pose2d& desired,
                             const ChassisSpeeds& desiredSpeeds) {
      Pose2d error = computeRobotRelativeError(desired, current);

      float vRef = desiredSpeeds.vx;
      float omegaRef = desiredSpeeds.omega;

      float k = 2.0f * zeta_ * std::sqrt(omegaRef * omegaRef + b_ * vRef * vRef);
      float headingError = error.heading;
      float sinXOverX = (std::fabs(headingError) < 1e-9f) ? 1.0f : std::sin(headingError) / headingError;

      float vx = vRef * std::cos(headingError) + k * error.x;
      float omega = omegaRef + b_ * vRef * sinXOverX * error.y + k * headingError;

      lastError_ = error;
      lastReference_ = desired;

      return ChassisSpeeds(vx, 0.0f, omega);
    }

    bool atReference() const {
      return std::fabs(lastError_.x) <= toleranceX_
          && std::fabs(lastError_.y) <= toleranceY_
          && std::fabs(lastError_.heading) <= toleranceTheta_;
    }

    const Pose2d& lastError() const { return lastError_; }

  private:
    float b_;
    float zeta_;
    float toleranceX_;
    float toleranceY_;
    float toleranceTheta_;

    Pose2d lastError_{};
    Pose2d lastReference_{};
  };
}
