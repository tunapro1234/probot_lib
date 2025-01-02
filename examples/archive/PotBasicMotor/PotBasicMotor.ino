#include <probot.h>

const int POT_PIN = A0;  // Analog pin for potentiometer

// Motor and Encoder Pins
const int MOTOR_PWM = 9;  // PWM pin for motor speed
const int MOTOR_DIR1 = 8;  // Dir1 pin for motor
const int MOTOR_DIR2 = 7;  // Dir2 pin for motor

// Motor Controller
BasicMotorController motor(MOTOR_PWM, MOTOR_DIR1, MOTOR_DIR2);

void setup() {
    Serial.begin(9600);
    motor.begin();
}

void loop() {
    // Read potentiometer value
    int potValue = analogRead(POT_PIN);
    float target = map(analogRead(POT_PIN), 0, 1023, 0, 100) / 100.;

    motor.setPower(target); 

    // Display current motor state
    Serial.print("Pot Reading: ");
    Serial.print(potValue);
    Serial.print("Current Power: ");
    Serial.println(motor.getPower());

    delay(10); // Loop delay
}
