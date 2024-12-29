#include "NFRSlider.h"
#include <Arduino.h>

NFRSlider::NFRSlider(int motorID) {
    motor = new NFRFeedbackMotor(motorID);
}

void NFRSlider::begin() {
    motor.begin();
    Serial.println(F("NFRSlider Initialized"));
}

void NFRSlider::setTargetLength(float length) {
    if (length < 0) length = 0;
    if (length > NFRSLIDER_LENGTH_LIMIT) length = NFRSLIDER_LENGTH_LIMIT;
    motor.setPositionTarget(length * NFRSLIDER_COUNTS_PER_CM);
}

void NFRSlider::update() {
    motor.update();
    safetyCheck();
}

float NFRSlider::getLength() {
    return motor.getPosition() / NFRSLIDER_COUNTS_PER_CM;
}

float NFRSlider::getTargetLength() {
    return motor.getPositionTarget() / NFRSLIDER_COUNTS_PER_CM;
}

float NFRSlider::getMaxLength() {
    return NFRSLIDER_LENGTH_LIMIT;
}

bool NFRSlider::isAtTarget() {
    return motor.isAtTarget();
}

void NFRSlider::safetyCheck() {
    if (motor.getPosition() > NFRSLIDER_LENGTH_LIMIT * NFRSLIDER_COUNTS_PER_CM) {
        motor.setPositionTarget(NFRSLIDER_LENGTH_LIMIT * NFRSLIDER_COUNTS_PER_CM);
    }
}
