#include "NFRRawMotor.h"
#include <Arduino.h>

// Constructor
NFRRawMotor::NFRRawMotor(int id) : motorID(id), power(0.0f) {}

// Initialize Motor
void NFRRawMotor::begin() {
    Serial.print(F("NFRRawMotor Initialized with ID: "));
    Serial.println(motorID);
}

// Set Motor Power
void NFRRawMotor::setPower(float power) {
    this->power = power;
    Serial.print(F("Motor "));
    Serial.print(motorID);
    Serial.print(F(" Power set to: "));
    Serial.println(power);
}

// Reset Motor
void NFRRawMotor::reset() {
    power = 0.0f;
    Serial.print(F("Motor "));
    Serial.print(motorID);
    Serial.println(F(" Reset."));
}

// Get Motor ID
int NFRRawMotor::getID() const {
    return motorID;
}
