#include "GenericSlider.h"
#include <Arduino.h>

// Constructor
GenericSlider::GenericSlider(BaseFeedbackMotor& motor, float lengthLimit, float countsPerCm)
    : motor(motor), lengthLimit(lengthLimit), countsPerCm(countsPerCm) {}

// Initialize Slider
void GenericSlider::begin() {
    motor.begin();
    Serial.println(F("GenericSlider Initialized with countsPerCm"));
}

// Set Target Length (in cm)
void GenericSlider::setTargetLength(float length) {
    if (length < 0) {
        Serial.println(F("Error: Target length below 0, clamping to 0"));
        motor.setPositionTarget(0);
    } else if (length > lengthLimit) {
        Serial.println(F("Error: Target length exceeds maximum limit, clamping to max length"));
        motor.setPositionTarget(mapLengthToTicks(lengthLimit));
    } else {
        motor.setPositionTarget(mapLengthToTicks(length));
    }
}

// Update Slider State
void GenericSlider::update() {
    motor.update();
    safetyCheck();
}

// Get Current Length (in cm)
float GenericSlider::getLength() {
    return mapTicksToLength(motor.getPosition());
}

// Get Target Length (in cm)
float GenericSlider::getTargetLength() {
    return mapTicksToLength(motor.getPositionTarget());
}

// Get Maximum Length (in cm)
float GenericSlider::getMaxLength() {
    return lengthLimit;
}

// Check if Slider is at Target
bool GenericSlider::isAtTarget() {
    return motor.isAtTarget();
}

// Map Ticks to Length (in cm)
float GenericSlider::mapTicksToLength(float ticks) {
    return ticks / countsPerCm;
}

// Map Length (in cm) to Ticks
float GenericSlider::mapLengthToTicks(float length) {
    return length * countsPerCm;
}

// Internal Safety Check
void GenericSlider::safetyCheck() {
    float currentPosition = motor.getPosition();

    if (currentPosition < 0) {
        Serial.println(F("Warning: Slider exceeded minimum limit, resetting to 0"));
        motor.setPositionTarget(0);
    } else if (currentPosition > mapLengthToTicks(lengthLimit)) {
        Serial.println(F("Warning: Slider exceeded maximum limit, resetting to max length"));
        motor.setPositionTarget(mapLengthToTicks(lengthLimit));
    }
}