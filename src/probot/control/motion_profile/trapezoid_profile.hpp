#pragma once
#include <algorithm>
#include <cmath>
#include <probot/control/motion_profile/imotion_profile.hpp>

namespace probot::control::motion_profile {
  class TrapezoidProfile : public IMotionProfile {
  public:
    struct Constraints {
      float maxVelocity;
      float maxAcceleration;
      constexpr Constraints(float maxVel = 0.0f, float maxAcc = 0.0f)
      : maxVelocity(maxVel), maxAcceleration(maxAcc) {}
    };

    // Use base class State
    using State = IMotionProfile::State;

    TrapezoidProfile(const Constraints& constraints,
                     const State& goal,
                     const State& initial = State{})
    : constraints_(constraints), goal_(goal), initial_(initial) {
      computeProfile();
    }

    void reset(const State& initial, const State& goal){
      initial_ = initial;
      goal_ = goal;
      computeProfile();
    }

    State calculate(float time) const override {
      if (totalTime_ <= 0.0f){ return goal_; }
      if (time >= totalTime_) return goal_;
      if (time <= 0.0f) return initial_;

      float t = std::clamp(time, 0.0f, totalTime_);
      float a = effectiveMaxAcceleration_;
      float pos = pos0_;
      float vel = vi_;

      if (t <= t1_){
        vel = vi_ + a * t;
        pos = pos0_ + vi_ * t + 0.5f * a * t * t;
      } else if (t <= t1_ + t2_){
        float tc = t - t1_;
        vel = vPeak_;
        pos = pos1End_ + vPeak_ * tc;
      } else {
        float td = t - t1_ - t2_;
        vel = vPeak_ - a * td;
        pos = pos2End_ + vPeak_ * td - 0.5f * a * td * td;
      }

      State result;
      result.position = pos * dir_;
      result.velocity = vel * dir_;
      return result;
    }

    float totalTime() const override { return totalTime_; }
    bool isFinished(float time) const override { return time >= totalTime_; }
    const Constraints& constraints() const { return constraints_; }
    const State& initial() const { return initial_; }
    const State& goal() const { return goal_; }

  private:
    void computeProfile(){
      dir_ = (goal_.position - initial_.position) >= 0.0f ? 1.0f : -1.0f;
      pos0_ = initial_.position * dir_;
      float posf = goal_.position * dir_;
      float deltaPos = posf - pos0_;

      vi_ = initial_.velocity * dir_;
      vf_ = goal_.velocity * dir_;

      float maxVel = std::max(0.0f, constraints_.maxVelocity);
      float maxAcc = std::max(constraints_.maxAcceleration, 1e-6f);

      // Ensure velocities do not exceed physical max velocity
      vi_ = std::clamp(vi_, -maxVel, maxVel);
      vf_ = std::clamp(vf_, -maxVel, maxVel);

      if (std::fabs(deltaPos) < 1e-6f && std::fabs(vi_ - vf_) < 1e-6f){
        t1_ = t2_ = t3_ = totalTime_ = 0.0f;
        vPeak_ = vi_;
        pos1End_ = pos2End_ = pos0_;
        effectiveMaxAcceleration_ = maxAcc;
        return;
      }

      float accelTime = std::max(0.0f, (maxVel - vi_) / maxAcc);
      float decelTime = std::max(0.0f, (maxVel - vf_) / maxAcc);
      float accelDist = (vi_ + maxVel) * 0.5f * accelTime;
      float decelDist = (vf_ + maxVel) * 0.5f * decelTime;
      float cruiseDist = deltaPos - accelDist - decelDist;

      if (cruiseDist < 0.0f){
        // Triangular profile (no cruise)
        float discriminant = std::max(0.0f, maxAcc * deltaPos + 0.5f * (vi_ * vi_ + vf_ * vf_));
        maxVel = std::sqrt(discriminant);
        maxVel = std::min(maxVel, constraints_.maxVelocity);
        accelTime = std::max(0.0f, (maxVel - vi_) / maxAcc);
        decelTime = std::max(0.0f, (maxVel - vf_) / maxAcc);
        cruiseDist = 0.0f;
      }

      float cruiseTime = (maxVel > 1e-6f) ? cruiseDist / maxVel : 0.0f;
      cruiseTime = std::max(0.0f, cruiseTime);

      t1_ = accelTime;
      t2_ = cruiseTime;
      t3_ = decelTime;
      totalTime_ = t1_ + t2_ + t3_;
      vPeak_ = maxVel;
      effectiveMaxAcceleration_ = maxAcc;

      pos1End_ = pos0_ + vi_ * t1_ + 0.5f * maxAcc * t1_ * t1_;
      pos2End_ = pos1End_ + vPeak_ * t2_;
    }

    Constraints constraints_{};
    State goal_{};
    State initial_{};

    float dir_ = 1.0f;
    float pos0_ = 0.0f;
    float vi_ = 0.0f;
    float vf_ = 0.0f;
    float vPeak_ = 0.0f;
    float t1_ = 0.0f, t2_ = 0.0f, t3_ = 0.0f;
    float totalTime_ = 0.0f;
    float pos1End_ = 0.0f;
    float pos2End_ = 0.0f;
    float effectiveMaxAcceleration_ = 0.0f;
  };
}
