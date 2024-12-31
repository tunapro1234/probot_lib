#include "NFRSlider.h"
#include <Arduino.h>

NFRSlider::NFRSlider(const unsigned int &motorID) {
    motor = new NFRFeedbackMotor(motorID);
}

void NFRSlider::begin() {
    motor->begin();
    Serial.println(F("NFRSlider Initialized"));
}

void NFRSlider::setTargetLength(float length) {
    if (length < 0) {
        Serial.println(F("Error: Target length below 0, clamping to 0"));
        motor->setPositionTarget(0);
    } else if (length > NFRSLIDER_LENGTH_LIMIT) {
        Serial.println(F("Error: Target length exceeds maximum limit, clamping to max length"));
        motor->setPositionTarget(mapLengthToTicks(NFRSLIDER_LENGTH_LIMIT));
    } else {
        motor->setPositionTarget(mapLengthToTicks(length));
    }
}

void NFRSlider::update() {
    motor->update();
    safetyCheck();
}

float NFRSlider::getLength() {
    return mapTicksToLength(motor->getPosition());
}

float NFRSlider::getTargetLength() {
    return mapTicksToLength(motor->getPositionTarget());
}

float NFRSlider::getMaxLength() {
    return NFRSLIDER_LENGTH_LIMIT;
}

bool NFRSlider::isAtTarget() {
    return motor->isAtTarget();
}

float NFRSlider::mapTicksToLength(float ticks) {
    return ticks / NFRSLIDER_COUNTS_PER_CM;
}

float NFRSlider::mapLengthToTicks(float length) {
    return length * NFRSLIDER_COUNTS_PER_CM;
}

// Internal Safety Check
void NFRSlider::safetyCheck() {
    float currentPosition = motor->getPosition();

    if (currentPosition < 0) {
        Serial.println(F("Warning: Slider exceeded minimum limit, resetting to 0"));
        motor->setPositionTarget(0);
    } else if (currentPosition > mapLengthToTicks(NFRSLIDER_LENGTH_LIMIT)) {
        Serial.println(F("Warning: Slider exceeded maximum limit, resetting to max length"));
        motor->setPositionTarget(mapLengthToTicks(NFRSLIDER_LENGTH_LIMIT));
    }
}