#include <probot/devices/motors/Boardoza_BA6208.hpp>

#if defined(ARDUINO_ARCH_ESP32)
#include <esp32-hal-ledc.h>
#endif

const int pinA = 22;
const int pinB = 21;
uint16_t timer = 0;

/**
 * @brief Constructor for the Boardoza_BA6208 class.
 * 
 * This constructor is used to create objects of the Boardoza_BA6208 class.
 * 
 * @param pwmPin The PWM pin number
 * @param pwmChannel The PWM channel number
 * @param pwmFrequency The PWM frequency
 * @param pwmResolution The PWM resolution
 */
Boardoza_BA6208::Boardoza_BA6208(uint8_t pwmPin, uint8_t pwmChannel, uint16_t pwmFrequency, uint8_t pwmResolution)
    : pwmPin(pwmPin), pwmChannel(pwmChannel), pwmFrequency(pwmFrequency), pwmResolution(pwmResolution) {}

/**
 * @brief Initialize the Boardoza_BA6208 object.
 * 
 * This function initializes the Boardoza_BA6208 object by setting up the PWM channel with the specified frequency and resolution
 * and attaches the PWM pin to the PWM channel.
 */
void Boardoza_BA6208::begin() {
#if defined(ARDUINO_ARCH_ESP32)
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcAttachChannel(pwmPin, pwmFrequency, pwmResolution, pwmChannel);
#else
    ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
    ledcAttachPin(pwmPin, pwmChannel);
#endif
#else
    (void)pwmPin;
    (void)pwmChannel;
    (void)pwmFrequency;
    (void)pwmResolution;
#endif
}

/**
 * @brief Rotate the Boardoza_BA6208 object forward.
 * 
 * This function rotates the Boardoza_BA6208 object forward by setting the specified pins to output mode
 * and applying a high signal to pinA and a low signal to pinB.
 */
void Boardoza_BA6208::turnForward() {
    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
}

/**
 * @brief Rotate the Boardoza_BA6208 object in reverse direction.
 * 
 * This function rotates the Boardoza_BA6208 object in reverse direction by setting the specified pins to output mode
 * and applying a low signal to pinA and a high signal to pinB.
 */
void Boardoza_BA6208::turnReverse() {
    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
}

/**
 * @brief Apply brake to stop the rotation of the Boardoza_BA6208 object.
 * 
 * This function applies brake to stop the rotation of the Boardoza_BA6208 object by setting the specified pins to output mode
 * and applying low signals to both pinA and pinB.
 */
void Boardoza_BA6208::brake() {
    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
}

/**
 * @brief Set software PWM for controlling the Boardoza_BA6208 object.
 * 
 * This function sets a software PWM signal for controlling the Boardoza_BA6208 object. The duty cycle, period, and action parameters are used to determine the rotation direction or braking of the motor.
 * 
 * @param dutyCycle The duty cycle of the PWM signal.
 * @param period The period of the PWM signal.
 * @param action The action to be performed (FORWARD, REVERSE, or BRAKE).
 */
void Boardoza_BA6208::setSoftwarePWM(uint16_t dutyCycle, uint16_t period, uint16_t action) {
    if(timer < period) {
        if(timer < dutyCycle) {
            switch (action)
            {
            case FORWARD: turnForward();  break;
            case REVERSE: turnReverse();  break;
            case BRAKE  : brake();  break;
            default: brake();
                break;
            }
        } else {
            brake();
        }
        timer++;
    } else {
        timer=0;
    }
}
/**
 * @brief Set hardware PWM duty cycle for controlling the Boardoza_BA6208 object.
 * 
 * This function sets the hardware PWM duty cycle for controlling the Boardoza_BA6208 object. The duty cycle parameter determines the percentage of power supplied to the motor.
 * 
 * @param dutyCycle The duty cycle value to set the PWM signal. Valid values range from 0 to 255, corresponding to 0% to 100% duty cycle.
 */
void Boardoza_BA6208::setHardwarePWM(uint16_t dutyCycle) {
    /* dutyCycle = 64 = %25
    dutyCycle = 127 = %50
    dutyCycle = 191 = %75
    dutyCycle = 255 = %100
    */ 
#if defined(ARDUINO_ARCH_ESP32)
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWriteChannel(pwmChannel, dutyCycle);
#else
    ledcWrite(pwmChannel, dutyCycle);
#endif
#else
    (void)dutyCycle;
#endif
}
