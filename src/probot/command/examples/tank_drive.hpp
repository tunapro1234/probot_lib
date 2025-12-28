#pragma once
#include <algorithm>
#include <cmath>
#include <probot/command/subsystem.hpp>
#include <probot/control/geometry.hpp>
#include <probot/control/kinematics/differential_drive_kinematics.hpp>
#include <probot/control/odometry/differential_drive_odometry.hpp>
#include <probot/devices/motors/imotor_controller.hpp>
#include <probot/devices/sensors/encoder.hpp>

namespace probot::command::examples {

class TankDrive : public probot::command::SubsystemBase {
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

  TankDrive(probot::motor::IMotorController* left,
            probot::motor::IMotorController* right,
            probot::sensors::IEncoder* leftEncoder = nullptr,
            probot::sensors::IEncoder* rightEncoder = nullptr)
  : probot::command::SubsystemBase("TankDrive"),
    left_(left),
    right_(right),
    left_encoder_(leftEncoder),
    right_encoder_(rightEncoder),
    kinematics_(track_width_),
    odometry_(probot::control::Pose2d()) {}

  void setInverted(bool left, bool right){
    if (left_) left_->setInverted(left);
    if (right_) right_->setInverted(right);
  }

  void setEncoders(probot::sensors::IEncoder* leftEncoder,
                   probot::sensors::IEncoder* rightEncoder){
    left_encoder_ = leftEncoder;
    right_encoder_ = rightEncoder;
    odom_initialized_ = false;
  }

  void setWheelRadius(float radius_units){ wheel_radius_ = radius_units > 0.0f ? radius_units : 1.0f; }
  void setTrackWidth(float width_units){
    track_width_ = width_units > 0.0f ? width_units : 1.0f;
    kinematics_.setTrackWidth(track_width_);
  }
  void setGearRatio(float ratio){ gear_ratio_ = ratio > 0.0f ? ratio : 1.0f; }
  void setEncoderTicksPerRevolution(float ticks){ ticks_per_rev_ = ticks > 0.0f ? ticks : 1.0f; }
  void setMaxVelocity(float units_per_s){ max_velocity_ = units_per_s > 0.0f ? units_per_s : 0.0f; }

  void drivePower(float left, float right){
    mode_ = DriveMode::kPower;
    last_status_ = CommandStatus::kOk;
    float l = clampUnit(left);
    float r = clampUnit(right);
    if (left_) left_->setPower(l);
    if (right_) right_->setPower(r);
  }

  void driveVelocity(float left_units_per_s, float right_units_per_s){
    if (!velocitySupported()){
      drivePower(0.0f, 0.0f);
      last_status_ = CommandStatus::kVelocityUnsupported;
      return;
    }
    mode_ = DriveMode::kVelocity;
    last_status_ = CommandStatus::kOk;
    float l = toMotorVelocity(left_units_per_s);
    float r = toMotorVelocity(right_units_per_s);
    left_->setVelocity(l);
    right_->setVelocity(r);
  }

  void driveVelocity(const probot::control::ChassisSpeeds& speeds){
    auto wheel = kinematics_.toWheelSpeeds(speeds);
    driveVelocity(wheel.first, wheel.second);
  }

  void driveVelocityNormalized(float left, float right){
    float l = clampUnit(left);
    float r = clampUnit(right);
    driveVelocity(l * max_velocity_, r * max_velocity_);
  }

  void drivePosition(float left_units, float right_units){
    if (!positionSupported()){
      drivePower(0.0f, 0.0f);
      last_status_ = CommandStatus::kPositionUnsupported;
      return;
    }
    mode_ = DriveMode::kPosition;
    last_status_ = CommandStatus::kOk;
    float l = toMotorPosition(left_units);
    float r = toMotorPosition(right_units);
    left_->setPosition(l);
    right_->setPosition(r);
  }

  void stop(){ drivePower(0.0f, 0.0f); }

  const probot::control::Pose2d& pose() const { return odometry_.pose(); }

  void resetPose(const probot::control::Pose2d& pose){
    heading_ = pose.heading;
    if (left_encoder_ && right_encoder_){
      float left_pos = ticksToDistance(left_encoder_->readTicks());
      float right_pos = ticksToDistance(right_encoder_->readTicks());
      odometry_.reset(pose, left_pos, right_pos);
      prev_left_pos_ = left_pos;
      prev_right_pos_ = right_pos;
      odom_initialized_ = true;
    } else {
      odometry_.reset(pose, 0.0f, 0.0f);
      prev_left_pos_ = 0.0f;
      prev_right_pos_ = 0.0f;
      odom_initialized_ = false;
    }
  }

  DriveMode mode() const { return mode_; }
  CommandStatus lastStatus() const { return last_status_; }

  void periodic(uint32_t now_ms, uint32_t dt_ms) override {
    if (left_) left_->update(now_ms, dt_ms);
    if (right_) right_->update(now_ms, dt_ms);
    updateOdometry();
  }

private:
  static float clampUnit(float value){
    if (value > 1.0f) return 1.0f;
    if (value < -1.0f) return -1.0f;
    return value;
  }

  bool velocitySupported() const {
    return left_ && right_ && left_->supportsVelocity() && right_->supportsVelocity();
  }

  bool positionSupported() const {
    return left_ && right_ && left_->supportsPosition() && right_->supportsPosition();
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

  void updateOdometry(){
    if (!left_encoder_ || !right_encoder_){
      return;
    }

    float left_pos = ticksToDistance(left_encoder_->readTicks());
    float right_pos = ticksToDistance(right_encoder_->readTicks());

    if (!odom_initialized_){
      odometry_.reset(odometry_.pose(), left_pos, right_pos);
      prev_left_pos_ = left_pos;
      prev_right_pos_ = right_pos;
      odom_initialized_ = true;
      return;
    }

    float dl = left_pos - prev_left_pos_;
    float dr = right_pos - prev_right_pos_;
    prev_left_pos_ = left_pos;
    prev_right_pos_ = right_pos;

    if (track_width_ > 0.0f){
      heading_ = probot::control::normalizeAngle(heading_ + (dr - dl) / track_width_);
    }

    odometry_.update(left_pos, right_pos, heading_);
  }

  probot::motor::IMotorController* left_;
  probot::motor::IMotorController* right_;
  probot::sensors::IEncoder* left_encoder_;
  probot::sensors::IEncoder* right_encoder_;

  float wheel_radius_ = 1.0f;
  float track_width_ = 1.0f;
  float gear_ratio_ = 1.0f;
  float ticks_per_rev_ = 1.0f;
  float max_velocity_ = 1.0f;

  DriveMode mode_ = DriveMode::kPower;
  CommandStatus last_status_ = CommandStatus::kOk;

  probot::control::kinematics::DifferentialDriveKinematics kinematics_;
  probot::control::odometry::DifferentialDriveOdometry odometry_;
  float heading_ = 0.0f;
  bool odom_initialized_ = false;
  float prev_left_pos_ = 0.0f;
  float prev_right_pos_ = 0.0f;
};

} // namespace probot::command::examples
