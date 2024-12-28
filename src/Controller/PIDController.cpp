#include "PIDController.h"

PIDController::PIDController(float kp, float ki, float kd)
  : Kp(kp), Ki(ki), Kd(kd), setpoint(0.0), integral(0.0), previousError(0.0), lastTime(millis()) {}

void PIDController::setTunings(float kp, float ki, float kd) {
  Kp = kp;
  Ki = ki;
  Kd = kd;
}

void PIDController::setSetpoint(float set) {
  setpoint = set;
}

float PIDController::compute(float input) {
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  if (dt <= 0.0) return 0.0;

  float error = setpoint - input;
  integral += error * dt;
  float derivative = (error - previousError) / dt;
  float output = Kp * error + Ki * integral + Kd * derivative;

  previousError = error;
  lastTime = now;

  return constrain(output, -1.0, 1.0);
}

void PIDController::reset() {
  integral = 0.0;
  previousError = 0.0;
  lastTime = millis();
}
