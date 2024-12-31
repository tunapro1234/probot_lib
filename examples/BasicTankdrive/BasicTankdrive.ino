#include <probot.h>

const int LEFT_POT_PIN = A0;
const int RIGHT_POT_PIN = A1;

// Motor and Encoder Pins
const int LEFT_MOTOR_PWM = 9;
const int LEFT_MOTOR_DIR1 = 8;
const int LEFT_MOTOR_DIR2 = 7;

const int RIGHT_MOTOR_PWM = 6;
const int RIGHT_MOTOR_DIR1 = 5;
const int RIGHT_MOTOR_DIR2 = 4;

BasicMotorController left_motor(LEFT_MOTOR_PWM, LEFT_MOTOR_DIR1, LEFT_MOTOR_DIR2);
BasicMotorController right_motor(RIGHT_MOTOR_PWM, RIGHT_MOTOR_DIR1, RIGHT_MOTOR_DIR2);

BasicTankdrive tankdrive(left_motor, right_motor);

void setup() {
    Serial.begin(9600);
    tankdrive.begin();
}

void loop() {
    // Read potentiometer value
    float left_power = map(analogRead(LEFT_POT_PIN), 0, 1023, 0, 100) / 100.;
    float right_power = map(analogRead(RIGHT_POT_PIN), 0, 1023, 0, 100) / 100.;

    tankdrive.setMotorPowers(left_power, right_power);

    // Display current motor states
    Serial.print("Left Pot Reading: ");
    Serial.print(left_power);
    Serial.print("Left Motor Current Power: ");
    Serial.print(left_motor.getPower());

    Serial.print("   |   ");

    Serial.print("Right Pot Reading: ");
    Serial.print(right_power);
    Serial.print("Right Motor Current Power: ");
    Serial.println(right_motor.getPower());

    delay(10); // Loop delay
}
