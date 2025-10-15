#pragma once

namespace probot::control::motion_profile {
  /**
   * @brief Common interface for motion profiles
   * 
   * Allows custom motion profile implementations and makes ClosedLoopMotor
   * profile-agnostic. Users can implement their own trajectories.
   */
  class IMotionProfile {
  public:
    /**
     * @brief Motion profile state at a given time
     */
    struct State {
      float position{0.0f};
      float velocity{0.0f};
      float acceleration{0.0f};  // Used by S-Curve and custom profiles
      
      constexpr State(float pos = 0.0f, float vel = 0.0f, float acc = 0.0f)
        : position(pos), velocity(vel), acceleration(acc) {}
    };
    
    /**
     * @brief Calculate profile state at given time
     * @param time Time since profile start (seconds)
     * @return State at the given time
     */
    virtual State calculate(float time) const = 0;
    
    /**
     * @brief Get total duration of the profile
     * @return Total time in seconds
     */
    virtual float totalTime() const = 0;
    
    /**
     * @brief Check if profile is finished at given time
     * @param time Time since profile start (seconds)
     * @return true if finished, false otherwise
     */
    virtual bool isFinished(float time) const = 0;
    
    virtual ~IMotionProfile() = default;
  };
} 