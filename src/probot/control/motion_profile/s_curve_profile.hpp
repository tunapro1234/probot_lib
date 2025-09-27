#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

namespace probot::control::motion_profile {
  class SCurveProfile {
  public:
    struct Constraints {
      float maxVelocity;
      float maxAcceleration;
      float maxJerk;
      constexpr Constraints(float maxVel = 0.0f,
                            float maxAcc = 0.0f,
                            float maxJ = 0.0f)
      : maxVelocity(maxVel), maxAcceleration(maxAcc), maxJerk(maxJ) {}
    };

    struct State {
      float position;
      float velocity;
      float acceleration;
      constexpr State(float pos = 0.0f,
                      float vel = 0.0f,
                      float acc = 0.0f)
      : position(pos), velocity(vel), acceleration(acc) {}
    };

    SCurveProfile(const Constraints& constraints,
                  const State& goal,
                  const State& initial = State{},
                  float timeStep = 0.005f)
    : constraints_(constraints), goal_(goal), initial_(initial), dt_(timeStep) {
      if (dt_ <= 0.0f) dt_ = 0.005f;
      generate();
    }

    void reset(const State& initial, const State& goal){
      initial_ = initial;
      goal_ = goal;
      generate();
    }

    State calculate(float time) const {
      if (samples_.empty()) return goal_;
      if (time <= 0.0f) return samples_.front();
      size_t idx = static_cast<size_t>(time / dt_);
      if (idx >= samples_.size()) return samples_.back();
      return samples_[idx];
    }

    float totalTime() const {
      return samples_.empty() ? 0.0f : dt_ * static_cast<float>(samples_.size() - 1);
    }

    bool isFinished(float time) const { return time >= totalTime(); }
    float timeStep() const { return dt_; }

    const Constraints& constraints() const { return constraints_; }
    const State& initial() const { return initial_; }
    const State& goal() const { return goal_; }

  private:
    void generate(){
      samples_.clear();
      State state = initial_;
      samples_.push_back(state);

      float maxVel = std::max(0.0f, constraints_.maxVelocity);
      float maxAcc = std::max(0.0f, constraints_.maxAcceleration);
      float maxJerk = std::max(0.0f, constraints_.maxJerk);

      const size_t kMaxSteps = 60000; // ~300s at 5ms
      if (maxJerk <= 0.0f || maxAcc <= 0.0f){
        // fall back to a simple acceleration-limited profile
        for (size_t i=0; i<kMaxSteps; ++i){
          float posError = goal_.position - state.position;
          float velError = goal_.velocity - state.velocity;

          float desiredAcc = std::clamp(velError / std::max(dt_, 1e-3f), -maxAcc, maxAcc);
          if (posError * state.velocity < 0.0f){
            desiredAcc = -maxAcc * std::copysign(1.0f, state.velocity);
          }

          state.acceleration = std::clamp(desiredAcc, -maxAcc, maxAcc);
          state.velocity += state.acceleration * dt_;
          state.velocity = std::clamp(state.velocity, -maxVel, maxVel);
          state.position += state.velocity * dt_;

          samples_.push_back(state);
          if (std::fabs(posError) <= 1e-4f &&
              std::fabs(state.velocity - goal_.velocity) <= 1e-4f){
            break;
          }
        }
        if (!samples_.empty()) samples_.back() = goal_;
        else samples_.push_back(goal_);
        return;
      }

      // jerk-limited integration
      for (size_t i=0; i<kMaxSteps; ++i){
        float posError = goal_.position - state.position;
        float velError = goal_.velocity - state.velocity;
        float dir = (posError >= 0.0f) ? 1.0f : -1.0f;

        float stoppingDist = (state.velocity * state.velocity) / (2.0f * std::max(maxAcc, 1e-6f));
        float desiredAcc = dir * maxAcc;

        bool needDecel = (dir > 0.0f && (posError < stoppingDist || velError < 0.0f)) ||
                         (dir < 0.0f && (-posError < stoppingDist || velError > 0.0f));
        if (needDecel){ desiredAcc = -dir * maxAcc; }

        float accelError = desiredAcc - state.acceleration;
        float jerk = accelError / dt_;
        jerk = std::clamp(jerk, -maxJerk, maxJerk);

        state.acceleration += jerk * dt_;
        state.acceleration = std::clamp(state.acceleration, -maxAcc, maxAcc);

        state.velocity += state.acceleration * dt_;
        state.velocity = std::clamp(state.velocity, -maxVel, maxVel);

        state.position += state.velocity * dt_;

        samples_.push_back(state);

        if (std::fabs(posError) <= 1e-4f &&
            std::fabs(state.velocity - goal_.velocity) <= 1e-4f &&
            std::fabs(state.acceleration - goal_.acceleration) <= 1e-3f){
          break;
        }
      }

      if (!samples_.empty()) samples_.back() = goal_;
      else samples_.push_back(goal_);
    }

    Constraints constraints_{};
    State goal_{};
    State initial_{};
    float dt_ = 0.005f;
    std::vector<State> samples_;
  };
}
