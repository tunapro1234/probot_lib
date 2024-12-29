#include "BasicMotorController.h"

BasicMotorController::BasicMotorController(int pwm, int dir1, int dir2)
  : pwmPin(pwm), dirPin1(dir1), dirPin2(dir2) {}

void BasicMotorController::begin() {
  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  stop();
}

void BasicMotorController::setSpeed(float speed) {
  speed = constrain(speed, -1.0, 1.0);
  this->speed = speed;
  int pwmValue = abs(speed) * 255;

  if (speed > 0) {
    digitalWrite(dirPin1, HIGH);
    digitalWrite(dirPin2, LOW);
  } else if (speed < 0) {
    digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, HIGH);
  } else {
    stop();
    return;
  }

  analogWrite(pwmPin, pwmValue);
}

float BasicMotorController::getSpeed() {
  return this->speed;
}

void BasicMotorController::stop() {
  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, LOW);
  analogWrite(pwmPin, 0);
}
