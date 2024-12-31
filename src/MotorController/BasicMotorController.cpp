#include "BasicMotorController.h"

BasicMotorController::BasicMotorController(int pwm, int dir1, int dir2, bool isReversed)
  : pwmPin(pwm), dirPin1(dir1), dirPin2(dir2), isReversed(isReversed) {}

void BasicMotorController::begin() {
  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  stop();
}

void BasicMotorController::setPower(float speed) {
  this->speed = constrain(speed, -1.0, 1.0);
  this->speed = isReversed ? -this->speed : this->speed;
  int pwmValue = abs(this->speed) * 255;

  if (this->speed > 0) {
    digitalWrite(dirPin1, HIGH);
    digitalWrite(dirPin2, LOW);
  } else if (this->speed < 0) {
    digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, HIGH);
  } else {
    stop();
    return;
  }

  analogWrite(pwmPin, pwmValue);
}

float BasicMotorController::getPower() {
  return this->speed;
}

void BasicMotorController::stop() {
  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, LOW);
  analogWrite(pwmPin, 0);
}
