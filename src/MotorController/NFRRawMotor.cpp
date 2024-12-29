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

// Get Motor Power
float NFRRawMotor::getPower() {
    return power;
}

// Stop Motor
void NFRRawMotor::stop() {
    power = 0.0f;
    Serial.print(F("Motor "));
    Serial.print(motorID);
    Serial.println(F(" Stopped"));
}

// Get Motor ID
int NFRRawMotor::getID() const {
    return motorID;
}
