#pragma once
#include <cmath>
#include <algorithm>
#include <probot/control/pid.hpp>
#include <probot/control/geometry.hpp>

namespace probot::control::trajectory {
  class HolonomicDriveController {
  public:
    HolonomicDriveController(probot::control::PID* xController,
                             probot::control::PID* yController,
                             probot::control::PID* thetaController)
    : xController_(xController),
      yController_(yController),
      thetaController_(thetaController),
      enabled_(true),
      toleranceX_(0.0f),
      toleranceY_(0.0f),
      toleranceTheta_(0.0f) {}

    void setTolerance(float xTol, float yTol, float thetaTol){
      toleranceX_ = std::max(0.0f, xTol);
      toleranceY_ = std::max(0.0f, yTol);
      toleranceTheta_ = std::max(0.0f, thetaTol);
    }

    void setEnabled(bool enabled){ enabled_ = enabled; }
    bool enabled() const { return enabled_; }

    ChassisSpeeds calculate(const Pose2d& current,
                             const Pose2d& desired,
                             const ChassisSpeeds& desiredSpeeds,
                             float dtSeconds) {
      Pose2d errorField(desired.x - current.x,
                        desired.y - current.y,
                        normalizeAngle(desired.heading - current.heading));

      if (!enabled_ || !xController_ || !yController_ || !thetaController_ || dtSeconds <= 0.0f){
        lastError_ = errorField;
        return desiredSpeeds;
      }

      float vx = desiredSpeeds.vx + xController_->step(errorField.x, dtSeconds);
      float vy = desiredSpeeds.vy + yController_->step(errorField.y, dtSeconds);
      float omega = desiredSpeeds.omega + thetaController_->step(errorField.heading, dtSeconds);

      lastError_ = errorField;
      return ChassisSpeeds(vx, vy, omega);
    }

    bool atReference() const {
      return std::fabs(lastError_.x) <= toleranceX_
          && std::fabs(lastError_.y) <= toleranceY_
          && std::fabs(lastError_.heading) <= toleranceTheta_;
    }

    const Pose2d& lastError() const { return lastError_; }

  private:
    probot::control::PID* xController_;
    probot::control::PID* yController_;
    probot::control::PID* thetaController_;
    bool enabled_;
    float toleranceX_;
    float toleranceY_;
    float toleranceTheta_;
    Pose2d lastError_{};
  };
}
