#pragma once
#include <probot/control/geometry.hpp>

namespace probot::control::kinematics {
  struct WheelSpeeds4 {
    float frontLeft;
    float frontRight;
    float rearLeft;
    float rearRight;
  };

  class MecanumDriveKinematics {
  public:
    MecanumDriveKinematics(float wheelBase, float trackWidth)
    : wheelBase_(wheelBase), trackWidth_(trackWidth) {}

    WheelSpeeds4 toWheelSpeeds(const probot::control::ChassisSpeeds& speeds) const {
      float k = wheelBase_ + trackWidth_;
      WheelSpeeds4 ws;
      ws.frontLeft  = speeds.vx - speeds.vy - speeds.omega * k;
      ws.frontRight = speeds.vx + speeds.vy + speeds.omega * k;
      ws.rearLeft   = speeds.vx + speeds.vy - speeds.omega * k;
      ws.rearRight  = speeds.vx - speeds.vy + speeds.omega * k;
      return ws;
    }

    probot::control::ChassisSpeeds toChassisSpeeds(const WheelSpeeds4& ws) const {
      float k = wheelBase_ + trackWidth_;
      float vx = (ws.frontLeft + ws.frontRight + ws.rearLeft + ws.rearRight) * 0.25f;
      float vy = (-ws.frontLeft + ws.frontRight + ws.rearLeft - ws.rearRight) * 0.25f;
      float omega = (-ws.frontLeft + ws.frontRight - ws.rearLeft + ws.rearRight) * 0.25f / k;
      return probot::control::ChassisSpeeds(vx, vy, omega);
    }

  private:
    float wheelBase_;
    float trackWidth_;
  };
}
