#ifndef PIDController_h
#define PIDController_h

#include <Arduino.h>

class PIDController {
  public:
    PIDController(float kp = 1.0, float ki = 0.0, float kd = 0.0);
    void setTunings(float kp, float ki, float kd);
    void setSetpoint(float setpoint);
    float compute(float input);
    void reset();

  private:
    float Kp, Ki, Kd;
    float setpoint;
    float integral;
    float previousError;
    unsigned long lastTime;
};

#endif
