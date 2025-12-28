#pragma once
#include <algorithm>
#include <cmath>
#include <probot/command/subsystem.hpp>
#include <probot/control/geometry.hpp>
#include <probot/control/kinematics/mecanum_drive_kinematics.hpp>
#include <probot/control/odometry/mecanum_drive_odometry.hpp>
#include <probot/devices/motors/imotor_controller.hpp>
#include <probot/devices/sensors/encoder.hpp>

namespace probot::command::examples {

class MecanumDrive : public probot::command::SubsystemBase {
public:
  enum class DriveMode {
    kPower,
    kVelocity,
    kPosition
  };

  enum class CommandStatus {
    kOk,
    kVelocityUnsupported,
    kPositionUnsupported,
    kEncodersMissing
  };

  MecanumDrive(probot::motor::IMotorController* frontLeft,
               probot::motor::IMotorController* frontRight,
               probot::motor::IMotorController* rearLeft,
               probot::motor::IMotorController* rearRight,
               probot::sensors::IEncoder* frontLeftEncoder = nullptr,
               probot::sensors::IEncoder* frontRightEncoder = nullptr,
               probot::sensors::IEncoder* rearLeftEncoder = nullptr,
               probot::sensors::IEncoder* rearRightEncoder = nullptr)
  : probot::command::SubsystemBase("MecanumDrive"),
    fl_(frontLeft),
    fr_(frontRight),
    rl_(rearLeft),
    rr_(rearRight),
    fl_encoder_(frontLeftEncoder),
    fr_encoder_(frontRightEncoder),
    rl_encoder_(rearLeftEncoder),
    rr_encoder_(rearRightEncoder),
    kinematics_(wheel_base_, track_width_),
    odometry_(probot::control::Pose2d()) {}

  void setInverted(bool frontLeft, bool frontRight, bool rearLeft, bool rearRight){
    if (fl_) fl_->setInverted(frontLeft);
    if (fr_) fr_->setInverted(frontRight);
    if (rl_) rl_->setInverted(rearLeft);
    if (rr_) rr_->setInverted(rearRight);
  }

  void setEncoders(probot::sensors::IEncoder* frontLeft,
                   probot::sensors::IEncoder* frontRight,
                   probot::sensors::IEncoder* rearLeft,
                   probot::sensors::IEncoder* rearRight){
    fl_encoder_ = frontLeft;
    fr_encoder_ = frontRight;
    rl_encoder_ = rearLeft;
    rr_encoder_ = rearRight;
    resetPose(odometry_.pose());
  }

  void setWheelRadius(float radius_units){ wheel_radius_ = radius_units > 0.0f ? radius_units : 1.0f; }
  void setTrackWidth(float width_units){
    track_width_ = width_units > 0.0f ? width_units : 1.0f;
    kinematics_ = probot::control::kinematics::MecanumDriveKinematics(wheel_base_, track_width_);
  }
  void setWheelBase(float base_units){
    wheel_base_ = base_units > 0.0f ? base_units : 1.0f;
    kinematics_ = probot::control::kinematics::MecanumDriveKinematics(wheel_base_, track_width_);
  }
  void setGearRatio(float ratio){ gear_ratio_ = ratio > 0.0f ? ratio : 1.0f; }
  void setEncoderTicksPerRevolution(float ticks){ ticks_per_rev_ = ticks > 0.0f ? ticks : 1.0f; }
  void setMaxVelocity(float units_per_s){ max_velocity_ = units_per_s > 0.0f ? units_per_s : 0.0f; }
  void setMaxAngularVelocity(float rad_per_s){ max_angular_velocity_ = rad_per_s > 0.0f ? rad_per_s : 0.0f; }

  void drivePower(float vx, float vy, float omega){
    mode_ = DriveMode::kPower;
    last_status_ = CommandStatus::kOk;
    probot::control::ChassisSpeeds speeds(vx, vy, omega);
    auto wheel = kinematics_.toWheelSpeeds(speeds);
    float maxMag = std::max({std::fabs(wheel.frontLeft), std::fabs(wheel.frontRight),
                             std::fabs(wheel.rearLeft), std::fabs(wheel.rearRight), 1.0f});
    wheel.frontLeft /= maxMag;
    wheel.frontRight /= maxMag;
    wheel.rearLeft /= maxMag;
    wheel.rearRight /= maxMag;
    if (fl_) fl_->setPower(wheel.frontLeft);
    if (fr_) fr_->setPower(wheel.frontRight);
    if (rl_) rl_->setPower(wheel.rearLeft);
    if (rr_) rr_->setPower(wheel.rearRight);
  }

  void driveVelocity(float vx, float vy, float omega){
    if (!velocitySupported()){
      drivePower(0.0f, 0.0f, 0.0f);
      last_status_ = CommandStatus::kVelocityUnsupported;
      return;
    }
    mode_ = DriveMode::kVelocity;
    last_status_ = CommandStatus::kOk;
    probot::control::ChassisSpeeds speeds(vx, vy, omega);
    auto wheel = kinematics_.toWheelSpeeds(speeds);
    setWheelVelocity(wheel);
  }

  void driveVelocity(const probot::control::ChassisSpeeds& speeds){
    driveVelocity(speeds.vx, speeds.vy, speeds.omega);
  }

  void driveVelocityNormalized(float vx, float vy, float omega){
    float x = clampUnit(vx) * max_velocity_;
    float y = clampUnit(vy) * max_velocity_;
    float w = clampUnit(omega) * max_angular_velocity_;
    driveVelocity(x, y, w);
  }

  void drivePosition(float frontLeft_units,
                     float frontRight_units,
                     float rearLeft_units,
                     float rearRight_units){
    if (!positionSupported()){
      drivePower(0.0f, 0.0f, 0.0f);
      last_status_ = CommandStatus::kPositionUnsupported;
      return;
    }
    mode_ = DriveMode::kPosition;
    last_status_ = CommandStatus::kOk;
    if (fl_) fl_->setPosition(toMotorPosition(frontLeft_units));
    if (fr_) fr_->setPosition(toMotorPosition(frontRight_units));
    if (rl_) rl_->setPosition(toMotorPosition(rearLeft_units));
    if (rr_) rr_->setPosition(toMotorPosition(rearRight_units));
  }

  void stop(){ drivePower(0.0f, 0.0f, 0.0f); }

  const probot::control::Pose2d& pose() const { return odometry_.pose(); }

  void resetPose(const probot::control::Pose2d& pose, uint32_t now_ms = 0){
    float now_s = static_cast<float>(now_ms) * 0.001f;
    odometry_.reset(pose, now_s);
    if (fl_encoder_ && fr_encoder_ && rl_encoder_ && rr_encoder_){
      probot::control::kinematics::WheelPositions4 positions{
        ticksToDistance(fl_encoder_->readTicks()),
        ticksToDistance(fr_encoder_->readTicks()),
        ticksToDistance(rl_encoder_->readTicks()),
        ticksToDistance(rr_encoder_->readTicks())
      };
      odometry_.update(now_s, positions, kinematics_);
    }
  }

  DriveMode mode() const { return mode_; }
  CommandStatus lastStatus() const { return last_status_; }

  void periodic(uint32_t now_ms, uint32_t dt_ms) override {
    if (fl_) fl_->update(now_ms, dt_ms);
    if (fr_) fr_->update(now_ms, dt_ms);
    if (rl_) rl_->update(now_ms, dt_ms);
    if (rr_) rr_->update(now_ms, dt_ms);
    updateOdometry(now_ms);
  }

private:
  static float clampUnit(float value){
    if (value > 1.0f) return 1.0f;
    if (value < -1.0f) return -1.0f;
    return value;
  }

  bool velocitySupported() const {
    return fl_ && fr_ && rl_ && rr_
      && fl_->supportsVelocity() && fr_->supportsVelocity()
      && rl_->supportsVelocity() && rr_->supportsVelocity();
  }

  bool positionSupported() const {
    return fl_ && fr_ && rl_ && rr_
      && fl_->supportsPosition() && fr_->supportsPosition()
      && rl_->supportsPosition() && rr_->supportsPosition();
  }

  float distancePerMotorRev() const {
    float denom = gear_ratio_ <= 0.0f ? 1.0f : gear_ratio_;
    return (2.0f * 3.1415926535f * wheel_radius_) / denom;
  }

  float toMotorVelocity(float wheel_units_per_s) const {
    float per_rev = distancePerMotorRev();
    return per_rev > 0.0f ? (wheel_units_per_s / per_rev) : 0.0f;
  }

  float toMotorPosition(float wheel_units) const {
    float per_rev = distancePerMotorRev();
    return per_rev > 0.0f ? (wheel_units / per_rev) : 0.0f;
  }

  float ticksToDistance(int32_t ticks) const {
    float revs = ticks_per_rev_ > 0.0f ? static_cast<float>(ticks) / ticks_per_rev_ : 0.0f;
    return revs * distancePerMotorRev();
  }

  void setWheelVelocity(const probot::control::kinematics::WheelSpeeds4& wheel){
    if (fl_) fl_->setVelocity(toMotorVelocity(wheel.frontLeft));
    if (fr_) fr_->setVelocity(toMotorVelocity(wheel.frontRight));
    if (rl_) rl_->setVelocity(toMotorVelocity(wheel.rearLeft));
    if (rr_) rr_->setVelocity(toMotorVelocity(wheel.rearRight));
  }

  void updateOdometry(uint32_t now_ms){
    if (!fl_encoder_ || !fr_encoder_ || !rl_encoder_ || !rr_encoder_){
      return;
    }

    probot::control::kinematics::WheelPositions4 positions{
      ticksToDistance(fl_encoder_->readTicks()),
      ticksToDistance(fr_encoder_->readTicks()),
      ticksToDistance(rl_encoder_->readTicks()),
      ticksToDistance(rr_encoder_->readTicks())
    };

    float now_s = static_cast<float>(now_ms) * 0.001f;
    odometry_.update(now_s, positions, kinematics_);
  }

  probot::motor::IMotorController* fl_;
  probot::motor::IMotorController* fr_;
  probot::motor::IMotorController* rl_;
  probot::motor::IMotorController* rr_;

  probot::sensors::IEncoder* fl_encoder_;
  probot::sensors::IEncoder* fr_encoder_;
  probot::sensors::IEncoder* rl_encoder_;
  probot::sensors::IEncoder* rr_encoder_;

  float wheel_radius_ = 1.0f;
  float track_width_ = 1.0f;
  float wheel_base_ = 1.0f;
  float gear_ratio_ = 1.0f;
  float ticks_per_rev_ = 1.0f;
  float max_velocity_ = 1.0f;
  float max_angular_velocity_ = 1.0f;

  DriveMode mode_ = DriveMode::kPower;
  CommandStatus last_status_ = CommandStatus::kOk;

  probot::control::kinematics::MecanumDriveKinematics kinematics_;
  probot::control::odometry::MecanumDriveOdometry odometry_;
};

} // namespace probot::command::examples
