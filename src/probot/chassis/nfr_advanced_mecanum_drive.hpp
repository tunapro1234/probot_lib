#pragma once
#include <algorithm>
#include <cmath>
#include <probot/control/kinematics/mecanum_drive_kinematics.hpp>
#include <probot/control/odometry/mecanum_drive_odometry.hpp>
#include <probot/control/trajectory/holonomic_drive_controller.hpp>
#include <probot/control/geometry.hpp>
#include <probot/control/feedforward/simple_motor_ff.hpp>
#include <probot/control/pid.hpp>
#include <probot/control/limiters/slew_rate_limiter.hpp>
#include <probot/sensors/imu/imu.hpp>
#include <probot/control/imotor_controller.hpp>

namespace probot::chassis {
  class NfrAdvancedMecanumDrive {
  public:
    struct Config {
      float wheelBase{0.5f};
      float trackWidth{0.5f};
      float maxOutput{1.0f};
      float slewRateVx{3.0f};
      float slewRateVy{3.0f};
      probot::control::PidConfig pidX{1.0f,0.0f,0.0f,0.0f,-1.0f,1.0f};
      probot::control::PidConfig pidY{1.0f,0.0f,0.0f,0.0f,-1.0f,1.0f};
      probot::control::PidConfig pidTheta{1.0f,0.0f,0.0f,0.0f,-1.0f,1.0f};
      probot::control::PidConfig wheelPid{0.4f,0.0f,0.0f,0.0f,-1.0f,1.0f};
      probot::control::feedforward::SimpleMotorFF feedforward{0.0f,0.0f,0.0f};
      probot::control::MotionProfileType wheelProfileType{probot::control::MotionProfileType::kNone};
      probot::control::MotionProfileConfig wheelProfileConfig{};
      bool useMotorControllerVelocity{false};
    };

    NfrAdvancedMecanumDrive(probot::control::IMotorController* frontLeft,
                            probot::control::IMotorController* frontRight,
                            probot::control::IMotorController* rearLeft,
                            probot::control::IMotorController* rearRight)
    : NfrAdvancedMecanumDrive(frontLeft, frontRight, rearLeft, rearRight, Config{}) {}

    NfrAdvancedMecanumDrive(probot::control::IMotorController* frontLeft,
                            probot::control::IMotorController* frontRight,
                            probot::control::IMotorController* rearLeft,
                            probot::control::IMotorController* rearRight,
                            const Config& config)
    : fl_(frontLeft), fr_(frontRight), rl_(rearLeft), rr_(rearRight),
      config_(config),
      kinematics_(config.wheelBase, config.trackWidth),
      odometry_(probot::control::Pose2d()),
      pidX_(config.pidX), pidY_(config.pidY), pidTheta_(config.pidTheta),
      controller_(&pidX_, &pidY_, &pidTheta_),
      pidFL_(config.wheelPid), pidFR_(config.wheelPid),
      pidRL_(config.wheelPid), pidRR_(config.wheelPid),
      feedforward_(config.feedforward),
      limiterVx_(config.slewRateVx, 0.0f),
      limiterVy_(config.slewRateVy, 0.0f)
    {
      if (fl_) { fl_->claim(&tokenFL_); applyWheelMotionProfile(fl_); }
      if (fr_) { fr_->claim(&tokenFR_); applyWheelMotionProfile(fr_); }
      if (rl_) { rl_->claim(&tokenRL_); applyWheelMotionProfile(rl_); }
      if (rr_) { rr_->claim(&tokenRR_); applyWheelMotionProfile(rr_); }
      controller_.setEnabled(true);
    }

    ~NfrAdvancedMecanumDrive(){
      if (fl_) fl_->release(&tokenFL_);
      if (fr_) fr_->release(&tokenFR_);
      if (rl_) rl_->release(&tokenRL_);
      if (rr_) rr_->release(&tokenRR_);
    }

    void setImu(probot::sensors::imu::IImu* imu){ imu_ = imu; }
    probot::sensors::imu::IImu* imu() const { return imu_; }

    void setGeometry(float wheelBase, float trackWidth){
      config_.wheelBase = wheelBase;
      config_.trackWidth = trackWidth;
      kinematics_ = probot::control::kinematics::MecanumDriveKinematics(wheelBase, trackWidth);
    }
    float wheelBase() const { return config_.wheelBase; }
    float trackWidth() const { return config_.trackWidth; }

    void setMaxOutput(float maxOut){ config_.maxOutput = std::max(0.1f, maxOut); }
    float maxOutput() const { return config_.maxOutput; }

    void setSlewRates(float vxRate, float vyRate){
      config_.slewRateVx = vxRate;
      config_.slewRateVy = vyRate;
      limiterVx_.setMaxRate(vxRate);
      limiterVy_.setMaxRate(vyRate);
    }

    void setPositionPid(const probot::control::PidConfig& pidX,
                        const probot::control::PidConfig& pidY,
                        const probot::control::PidConfig& pidTheta){
      config_.pidX = pidX;
      config_.pidY = pidY;
      config_.pidTheta = pidTheta;
      pidX_.setConfig(pidX);
      pidY_.setConfig(pidY);
      pidTheta_.setConfig(pidTheta);
    }

    void setWheelPidConfig(const probot::control::PidConfig& cfg){
      config_.wheelPid = cfg;
      pidFL_.setConfig(cfg);
      pidFR_.setConfig(cfg);
      pidRL_.setConfig(cfg);
      pidRR_.setConfig(cfg);
    }

    void setFeedforwardGains(float kS, float kV, float kA){
      feedforward_.setKs(kS);
      feedforward_.setKv(kV);
      feedforward_.setKa(kA);
      config_.feedforward = probot::control::feedforward::SimpleMotorFF(kS,kV,kA);
    }

    void enableController(bool enable){ controller_.setEnabled(enable); }
    bool controllerEnabled() const { return controller_.enabled(); }

    void setWheelMotionProfile(probot::control::MotionProfileType type,
                               const probot::control::MotionProfileConfig& cfg){
      config_.wheelProfileType = type;
      config_.wheelProfileConfig = cfg;
      applyWheelMotionProfile(fl_);
      applyWheelMotionProfile(fr_);
      applyWheelMotionProfile(rl_);
      applyWheelMotionProfile(rr_);
    }
    probot::control::MotionProfileType wheelMotionProfileType() const { return config_.wheelProfileType; }
    probot::control::MotionProfileConfig wheelMotionProfileConfig() const { return config_.wheelProfileConfig; }

    void useMotorControllerVelocityLoop(bool enable){ config_.useMotorControllerVelocity = enable; }
    bool motorControllerVelocityLoopEnabled() const { return config_.useMotorControllerVelocity; }

    const probot::control::Pose2d& pose() const { return odometry_.pose(); }

    void resetPose(const probot::control::Pose2d& pose,
                   float timestamp,
                   const probot::control::kinematics::WheelPositions4& wheelPositions){
      odometry_.reset(pose, timestamp);
      prevPositions_ = wheelPositions;
      prevWheelTargets_ = {0.0f, 0.0f, 0.0f, 0.0f};
      lastTimestamp_ = timestamp;
      initialized_ = true;
    }

    void setReference(const probot::control::Pose2d& pose,
                      const probot::control::ChassisSpeeds& speeds){
      referencePose_ = pose;
      referenceSpeeds_ = speeds;
      useController_ = true;
    }

    void setTargetSpeeds(const probot::control::ChassisSpeeds& speeds){
      referenceSpeeds_ = speeds;
      useController_ = false;
    }

    void update(float timestamp,
                const probot::control::kinematics::WheelPositions4& wheelPositions){
      if (!initialized_){
        resetPose(odometry_.pose(), timestamp, wheelPositions);
        return;
      }
      float dt = timestamp - lastTimestamp_;
      if (dt <= 1e-4f){
        lastTimestamp_ = timestamp;
        prevPositions_ = wheelPositions;
        return;
      }

      auto pose = odometry_.update(timestamp, wheelPositions, kinematics_);
      if (imu_){
        odometry_.setHeading(imu_->yaw());
        pose = odometry_.pose();
      }

      probot::control::ChassisSpeeds desired = referenceSpeeds_;
      if (useController_){
        desired = controller_.calculate(pose, referencePose_, referenceSpeeds_, dt);
      }
      desired.vx = limiterVx_.calculate(desired.vx, dt);
      desired.vy = limiterVy_.calculate(desired.vy, dt);

      auto wheelTargets = kinematics_.toWheelSpeeds(desired);
      probot::control::kinematics::WheelSpeeds4 measuredSpeeds {
        (wheelPositions.frontLeft - prevPositions_.frontLeft) / dt,
        (wheelPositions.frontRight - prevPositions_.frontRight) / dt,
        (wheelPositions.rearLeft - prevPositions_.rearLeft) / dt,
        (wheelPositions.rearRight - prevPositions_.rearRight) / dt
      };

      if (config_.useMotorControllerVelocity && fl_ && fr_ && rl_ && rr_){
        fl_->setSetpoint(wheelTargets.frontLeft, probot::control::ControlType::kVelocity);
        fr_->setSetpoint(wheelTargets.frontRight, probot::control::ControlType::kVelocity);
        rl_->setSetpoint(wheelTargets.rearLeft, probot::control::ControlType::kVelocity);
        rr_->setSetpoint(wheelTargets.rearRight, probot::control::ControlType::kVelocity);
      } else {
        float outputs[4];
        outputs[0] = computeWheelOutput(wheelTargets.frontLeft, measuredSpeeds.frontLeft, prevWheelTargets_.frontLeft, pidFL_, dt);
        outputs[1] = computeWheelOutput(wheelTargets.frontRight, measuredSpeeds.frontRight, prevWheelTargets_.frontRight, pidFR_, dt);
        outputs[2] = computeWheelOutput(wheelTargets.rearLeft, measuredSpeeds.rearLeft, prevWheelTargets_.rearLeft, pidRL_, dt);
        outputs[3] = computeWheelOutput(wheelTargets.rearRight, measuredSpeeds.rearRight, prevWheelTargets_.rearRight, pidRR_, dt);

        float maxMag = config_.maxOutput;
        for (float o : outputs) maxMag = std::max(maxMag, std::fabs(o));
        float scale = (maxMag > config_.maxOutput) ? config_.maxOutput / maxMag : 1.0f;

        if (fl_) fl_->setPower(std::clamp(outputs[0] * scale / config_.maxOutput, -1.0f, 1.0f), &tokenFL_);
        if (fr_) fr_->setPower(std::clamp(outputs[1] * scale / config_.maxOutput, -1.0f, 1.0f), &tokenFR_);
        if (rl_) rl_->setPower(std::clamp(outputs[2] * scale / config_.maxOutput, -1.0f, 1.0f), &tokenRL_);
        if (rr_) rr_->setPower(std::clamp(outputs[3] * scale / config_.maxOutput, -1.0f, 1.0f), &tokenRR_);
      }

      prevWheelTargets_ = wheelTargets;
      prevPositions_ = wheelPositions;
      lastTimestamp_ = timestamp;
    }

  private:
    float computeWheelOutput(float targetVel,
                             float measuredVel,
                             float prevTarget,
                             probot::control::PID& pid,
                             float dt){
      float accel = (targetVel - prevTarget) / dt;
      float error = targetVel - measuredVel;
      return feedforward_.calculate(targetVel, accel) + pid.step(error, dt);
    }

    probot::control::IMotorController* fl_;
    probot::control::IMotorController* fr_;
    probot::control::IMotorController* rl_;
    probot::control::IMotorController* rr_;
    void* tokenFL_ = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0);
    void* tokenFR_ = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 1);
    void* tokenRL_ = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 2);
    void* tokenRR_ = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 3);

    Config config_;
    probot::control::kinematics::MecanumDriveKinematics kinematics_;
    probot::control::odometry::MecanumDriveOdometry odometry_;
    probot::control::PID pidX_;
    probot::control::PID pidY_;
    probot::control::PID pidTheta_;
    probot::control::trajectory::HolonomicDriveController controller_;
    probot::control::PID pidFL_;
    probot::control::PID pidFR_;
    probot::control::PID pidRL_;
    probot::control::PID pidRR_;
    probot::control::feedforward::SimpleMotorFF feedforward_;
    probot::control::limiters::SlewRateLimiter limiterVx_;
    probot::control::limiters::SlewRateLimiter limiterVy_;
    probot::sensors::imu::IImu* imu_ = nullptr;

    probot::control::Pose2d referencePose_{};
    probot::control::ChassisSpeeds referenceSpeeds_{};
    bool useController_ = false;

    float lastTimestamp_ = 0.0f;
    bool initialized_ = false;
    probot::control::kinematics::WheelPositions4 prevPositions_{};
    probot::control::kinematics::WheelSpeeds4 prevWheelTargets_{};

    void applyWheelMotionProfile(probot::control::IMotorController* ctrl){
      if (!ctrl) return;
      ctrl->setMotionProfile(config_.wheelProfileType);
      ctrl->setMotionProfileConfig(config_.wheelProfileConfig);
    }
  };
}
