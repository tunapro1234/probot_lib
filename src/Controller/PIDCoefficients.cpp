#include "PIDCoefficients.h"

// Constructor
PIDCoefficients::PIDCoefficients(float kp, float ki, float kd)
    : kp(kp), ki(ki), kd(kd) {}

// Getters
float PIDCoefficients::getKp() const { return kp; }
float PIDCoefficients::getKi() const { return ki; }
float PIDCoefficients::getKd() const { return kd; }

// Setters
void PIDCoefficients::setKp(float kp) { this->kp = kp; }
void PIDCoefficients::setKi(float ki) { this->ki = ki; }
void PIDCoefficients::setKd(float kd) { this->kd = kd; }

// Operator Overload
void PIDCoefficients::operator=(const PIDCoefficients& other) {
    kp = other.kp;
    ki = other.ki;
    kd = other.kd;
}
