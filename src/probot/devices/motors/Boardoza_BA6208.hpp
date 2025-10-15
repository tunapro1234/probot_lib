#ifndef Boardoza_BA6208_h
#define Boardoza_BA6208_h

#include <Arduino.h>

#define FORWARD 0x01
#define REVERSE 0x02
#define BRAKE 0x00


class Boardoza_BA6208 {
public:
    Boardoza_BA6208(uint8_t pwmPin, uint8_t pwmChannel, uint16_t pwmFrequency, uint8_t pwmResolution);
    void begin();
    void turnForward();
    void turnReverse();
    void brake();
    void setSoftwarePWM(uint16_t dutyCycle, uint16_t period, uint16_t action);
    void setHardwarePWM(uint16_t dutyCycle);

private:
uint8_t pwmPin; // Pin number
uint8_t pwmChannel; // (Between 0-15 )
uint16_t pwmFrequency; // (Hz)
uint8_t pwmResolution; // (bit) (Between 1-16 )
};

#endif