#pragma once
#include <algorithm>
#include <probot/devices/motors/imotor_driver.hpp>

namespace probot::chassis {
  /**
   * @brief Simple mecanum drive with direct motor power control
   * Uses IMotorDriver for open-loop control (no PID)
   */
  class SimpleMecanumDrive {
  public:
    SimpleMecanumDrive(probot::motor::IMotorDriver* frontLeft,
                      probot::motor::IMotorDriver* frontRight,
                      probot::motor::IMotorDriver* rearLeft,
                      probot::motor::IMotorDriver* rearRight)
    : fl_(frontLeft), fr_(frontRight), rl_(rearLeft), rr_(rearRight) {
      if (fl_) fl_->claim(&tokenFL_);
      if (fr_) fr_->claim(&tokenFR_);
      if (rl_) rl_->claim(&tokenRL_);
      if (rr_) rr_->claim(&tokenRR_);
    }

    ~SimpleMecanumDrive(){
      if (fl_) fl_->release(&tokenFL_);
      if (fr_) fr_->release(&tokenFR_);
      if (rl_) rl_->release(&tokenRL_);
      if (rr_) rr_->release(&tokenRR_);
    }

    void setInverted(bool frontLeft, bool frontRight, bool rearLeft, bool rearRight){
      if (fl_) fl_->setInverted(frontLeft);
      if (fr_) fr_->setInverted(frontRight);
      if (rl_) rl_->setInverted(rearLeft);
      if (rr_) rr_->setInverted(rearRight);
    }

    void driveCartesian(float vx, float vy, float omega){
      float fl = vx - vy - omega;
      float fr = vx + vy + omega;
      float rl = vx + vy - omega;
      float rr = vx - vy + omega;
      float maxMag = std::max({std::fabs(fl), std::fabs(fr), std::fabs(rl), std::fabs(rr), 1.0f});
      fl /= maxMag; fr /= maxMag; rl /= maxMag; rr /= maxMag;
      if (fl_) fl_->setPower(fl, &tokenFL_);
      if (fr_) fr_->setPower(fr, &tokenFR_);
      if (rl_) rl_->setPower(rl, &tokenRL_);
      if (rr_) rr_->setPower(rr, &tokenRR_);
    }

    void stop(){ driveCartesian(0.0f, 0.0f, 0.0f); }

  private:
    probot::motor::IMotorDriver* fl_;
    probot::motor::IMotorDriver* fr_;
    probot::motor::IMotorDriver* rl_;
    probot::motor::IMotorDriver* rr_;
    void* tokenFL_ = this;
    void* tokenFR_ = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 1);
    void* tokenRL_ = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 2);
    void* tokenRR_ = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 3);
  };
}
