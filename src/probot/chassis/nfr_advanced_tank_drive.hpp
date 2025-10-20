#pragma once
#include <algorithm>
#include <cmath>
#include <probot/control/kinematics/differential_drive_kinematics.hpp>
#include <probot/control/odometry/differential_drive_odometry.hpp>
#include <probot/control/trajectory/ramsete_controller.hpp>
#include <probot/control/limiters/slew_rate_limiter.hpp>
#include <probot/control/feedforward/simple_motor_ff.hpp>
#include <probot/control/pid.hpp>
#include <probot/control/geometry.hpp>
#include <probot/sensors/imu/imu.hpp>
#include <probot/control/imotor_controller.hpp>

namespace probot::chassis {
  class NfrAdvancedTankDrive {
  public:
    struct Config {
      float trackWidth{0.6f};
      float linearSlewRate{3.0f};
      float maxOutput{1.0f};
      probot::control::PidConfig velocityPid{0.3f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f};
      probot::control::feedforward::SimpleMotorFF feedforward{0.0f, 0.0f, 0.0f};
      float ramseteB{2.0f};
      float ramseteZeta{0.7f};
      probot::control::MotionProfileType wheelProfileType{probot::control::MotionProfileType::kNone};
      probot::control::MotionProfileConfig wheelProfileConfig{};
      bool useMotorControllerVelocity{false};
    };

    NfrAdvancedTankDrive(probot::control::IMotorController* left,
                         probot::control::IMotorController* right)
    : NfrAdvancedTankDrive(left, right, Config{}) {}

    NfrAdvancedTankDrive(probot::control::IMotorController* left,
                         probot::control::IMotorController* right,
                         const Config& config)
    : left_(left), right_(right), config_(config),
      kinematics_(config.trackWidth),
      odometry_(probot::control::Pose2d()),
      ramsete_(config.ramseteB, config.ramseteZeta),
      linearLimiter_(config.linearSlewRate, 0.0f),
      pidLeft_(config.velocityPid), pidRight_(config.velocityPid),
      feedforward_(config.feedforward)
    {
      applyWheelMotionProfile(left_);
      applyWheelMotionProfile(right_);
    }

    ~NfrAdvancedTankDrive() = default;

    void setImu(probot::sensors::imu::IImu* imu){ imu_ = imu; }
    probot::sensors::imu::IImu* imu() const { return imu_; }

    void setTrackWidth(float trackWidth){
      config_.trackWidth = trackWidth;
      kinematics_.setTrackWidth(trackWidth);
    }
    float trackWidth() const { return config_.trackWidth; }

    void setLinearSlewRate(float rate){ config_.linearSlewRate = rate; linearLimiter_.setMaxRate(rate); }
    float linearSlewRate() const { return config_.linearSlewRate; }

    void setMaxOutput(float maxOut){ config_.maxOutput = std::max(0.1f, maxOut); }
    float maxOutput() const { return config_.maxOutput; }

    void setVelocityPidConfig(const probot::control::PidConfig& cfg){
      config_.velocityPid = cfg;
      pidLeft_.setConfig(cfg);
      pidRight_.setConfig(cfg);
    }
    const probot::control::PidConfig& velocityPidConfig() const { return config_.velocityPid; }

    void setFeedforwardGains(float kS, float kV, float kA){
      feedforward_.setKs(kS);
      feedforward_.setKv(kV);
      feedforward_.setKa(kA);
      config_.feedforward = probot::control::feedforward::SimpleMotorFF(kS, kV, kA);
    }
    probot::control::feedforward::SimpleMotorFF feedforward() const { return feedforward_; }

    void setWheelMotionProfile(probot::control::MotionProfileType type,
                               const probot::control::MotionProfileConfig& cfg){
      config_.wheelProfileType = type;
      config_.wheelProfileConfig = cfg;
      applyWheelMotionProfile(left_);
      applyWheelMotionProfile(right_);
    }
    probot::control::MotionProfileType wheelMotionProfileType() const { return config_.wheelProfileType; }
    probot::control::MotionProfileConfig wheelMotionProfileConfig() const { return config_.wheelProfileConfig; }

    void useMotorControllerVelocityLoop(bool enable){ config_.useMotorControllerVelocity = enable; }
    bool motorControllerVelocityLoopEnabled() const { return config_.useMotorControllerVelocity; }

    void setRamseteParams(float b, float zeta){
      config_.ramseteB = b;
      config_.ramseteZeta = zeta;
      ramsete_.setB(b);
      ramsete_.setZeta(zeta);
    }

    void setReference(const probot::control::Pose2d& referencePose,
                      const probot::control::ChassisSpeeds& referenceSpeeds){
      referencePose_ = referencePose;
      referenceSpeeds_ = referenceSpeeds;
      useRamsete_ = true;
    }

    void setTargetSpeeds(const probot::control::ChassisSpeeds& speeds){
      referenceSpeeds_ = speeds;
      useRamsete_ = false;
    }

    const probot::control::Pose2d& pose() const { return odometry_.pose(); }
    const probot::control::Pose2d& referencePose() const { return referencePose_; }
    probot::control::ChassisSpeeds targetSpeeds() const { return referenceSpeeds_; }

    void resetPose(const probot::control::Pose2d& pose,
                   float leftPosition,
                   float rightPosition){
      odometry_.reset(pose, leftPosition, rightPosition);
      prevLeftPos_ = leftPosition;
      prevRightPos_ = rightPosition;
      prevTimestamp_ = 0.0f;
      prevLeftTarget_ = 0.0f;
      prevRightTarget_ = 0.0f;
      initialized_ = false;
    }

    void update(float timestamp,
                float leftPosition,
                float rightPosition){
      if (!initialized_){
        prevTimestamp_ = timestamp;
        prevLeftPos_ = leftPosition;
        prevRightPos_ = rightPosition;
        if (imu_) {
          odometry_.reset(odometry_.pose(), leftPosition, rightPosition);
        }
        initialized_ = true;
        return;
      }
      float dt = timestamp - prevTimestamp_;
      if (dt <= 1e-4f){
        prevTimestamp_ = timestamp;
        return;
      }

      float deltaLeft = leftPosition - prevLeftPos_;
      float deltaRight = rightPosition - prevRightPos_;
      float heading = imu_ ? imu_->yaw()
                           : probot::control::normalizeAngle(odometry_.pose().heading + (deltaRight - deltaLeft) / config_.trackWidth);
      odometry_.update(leftPosition, rightPosition, heading);

      float leftVel = deltaLeft / dt;
      float rightVel = deltaRight / dt;

      probot::control::ChassisSpeeds desiredSpeeds = referenceSpeeds_;
      if (useRamsete_){
        desiredSpeeds = ramsete_.calculate(odometry_.pose(), referencePose_, referenceSpeeds_);
      }
      desiredSpeeds.vx = linearLimiter_.calculate(desiredSpeeds.vx, dt);

      auto wheelTargets = kinematics_.toWheelSpeeds(desiredSpeeds);
      float leftTarget = wheelTargets.first;
      float rightTarget = wheelTargets.second;

      float leftAccel = (leftTarget - prevLeftTarget_) / dt;
      float rightAccel = (rightTarget - prevRightTarget_) / dt;

      float leftError = leftTarget - leftVel;
      float rightError = rightTarget - rightVel;

      if (config_.useMotorControllerVelocity && left_ && right_){
        left_->setSetpoint(leftTarget, probot::control::ControlType::kVelocity);
        right_->setSetpoint(rightTarget, probot::control::ControlType::kVelocity);
      } else {
        float leftOutput = feedforward_.calculate(leftTarget, leftAccel) + pidLeft_.step(leftError, dt);
        float rightOutput = feedforward_.calculate(rightTarget, rightAccel) + pidRight_.step(rightError, dt);

        float maxMagnitude = std::max({std::fabs(leftOutput), std::fabs(rightOutput), config_.maxOutput});
        float scale = (maxMagnitude > config_.maxOutput) ? config_.maxOutput / maxMagnitude : 1.0f;

        float leftPower = std::clamp(leftOutput * scale / config_.maxOutput, -1.0f, 1.0f);
        float rightPower = std::clamp(rightOutput * scale / config_.maxOutput, -1.0f, 1.0f);

        if (left_) left_->setPower(leftPower);
        if (right_) right_->setPower(rightPower);
      }

      prevLeftTarget_ = leftTarget;
      prevRightTarget_ = rightTarget;
      prevLeftPos_ = leftPosition;
      prevRightPos_ = rightPosition;
      prevTimestamp_ = timestamp;
    }

  private:
    probot::control::IMotorController* left_;
    probot::control::IMotorController* right_;

    Config config_;
    probot::control::kinematics::DifferentialDriveKinematics kinematics_;
    probot::control::odometry::DifferentialDriveOdometry odometry_;
    probot::control::trajectory::RamseteController ramsete_;
    probot::control::limiters::SlewRateLimiter linearLimiter_;
    probot::control::PID pidLeft_;
    probot::control::PID pidRight_;
    probot::control::feedforward::SimpleMotorFF feedforward_;
    probot::sensors::imu::IImu* imu_ = nullptr;

    probot::control::Pose2d referencePose_{};
    probot::control::ChassisSpeeds referenceSpeeds_{};
    bool useRamsete_ = false;

    float prevLeftPos_ = 0.0f;
    float prevRightPos_ = 0.0f;
    float prevTimestamp_ = 0.0f;
    float prevLeftTarget_ = 0.0f;
    float prevRightTarget_ = 0.0f;
    bool initialized_ = false;

    void applyWheelMotionProfile(probot::control::IMotorController* ctrl){
      if (!ctrl) return;
      ctrl->setMotionProfile(config_.wheelProfileType);
      ctrl->setMotionProfileConfig(config_.wheelProfileConfig);
    }
  };
}
