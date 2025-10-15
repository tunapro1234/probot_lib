#pragma once
#include <cmath>
#include <probot/control/imotor_controller.hpp>
#include <probot/mechanism/nfr/common.hpp>

namespace probot::mechanism::nfr {

  class NfrShooter {
  public:
    explicit NfrShooter(probot::control::IMotorController* primary,
                        probot::control::IMotorController* secondary = nullptr)
    : primary_(primary), secondary_(secondary) {
      configureController(primary_);
      configureController(secondary_);
    }

    void setTicksPerRevolution(float ticks){ ticks_per_rev_ = ticks <= 0.0f ? 1.0f : ticks; }
    float ticksPerRevolution() const { return ticks_per_rev_; }

    void setPrimaryRpm(float rpm){ primary_target_rpm_ = rpm; applySetpoint(primary_, rpm); }
    void setSecondaryRpm(float rpm){ secondary_target_rpm_ = rpm; applySetpoint(secondary_, rpm); }

    void setRpm(float primaryRpm, float secondaryRpm){
      setPrimaryRpm(primaryRpm);
      setSecondaryRpm(secondaryRpm);
    }

    void stop(){
      setPrimaryRpm(0.0f);
      if (secondary_) setSecondaryRpm(0.0f);
    }

    bool primaryAtTarget(float toleranceRpm) const {
      return primary_ ? primary_->isAtTarget(rpmToTicks(toleranceRpm)) : true;
    }

    bool secondaryAtTarget(float toleranceRpm) const {
      return secondary_ ? secondary_->isAtTarget(rpmToTicks(toleranceRpm)) : true;
    }

    float primaryTargetRpm() const { return primary_target_rpm_; }
    float secondaryTargetRpm() const { return secondary_target_rpm_; }

  private:
    void configureController(probot::control::IMotorController* ctrl){
      if (!ctrl) return;
      ctrl->configurePidSlots(detail::kVelocitySlot, detail::kDefaultVelocityPid,
                              detail::kPositionSlot, detail::kDefaultPositionPid);
      ctrl->selectDefaultSlot(probot::control::ControlType::kVelocity, detail::kVelocitySlot);
      ctrl->setMotionProfile(probot::control::MotionProfileType::kNone);
      ctrl->setMotionProfileConfig({0.0f, 0.0f, 0.0f});
    }

    void applySetpoint(probot::control::IMotorController* ctrl, float rpm){
      if (!ctrl) return;
      float ticks_per_second = rpmToTicks(rpm);
      ctrl->setSetpoint(ticks_per_second, probot::control::ControlType::kVelocity, detail::kVelocitySlot);
    }

    float rpmToTicks(float rpm) const { return ticks_per_rev_ * rpm / 60.0f; }

    probot::control::IMotorController* primary_;
    probot::control::IMotorController* secondary_;
    float ticks_per_rev_ = 2048.0f; // default encoder resolution
    float primary_target_rpm_ = 0.0f;
    float secondary_target_rpm_ = 0.0f;
  };
}
