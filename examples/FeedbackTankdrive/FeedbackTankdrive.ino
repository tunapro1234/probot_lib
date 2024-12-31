#include <probot.h>

const int LEFT_POT_PIN = A0;
const int RIGHT_POT_PIN = A1;

// Motor and Encoder Pins
const int LEFT_MOTOR_PWM = 9;
const int LEFT_MOTOR_DIR1 = 8;
const int LEFT_MOTOR_DIR2 = 7;

const int LEFT_ENCODER_A = A2;
const int LEFT_ENCODER_B = A3;

const int RIGHT_MOTOR_PWM = 6;
const int RIGHT_MOTOR_DIR1 = 5;
const int RIGHT_MOTOR_DIR2 = 4;

const int RIGHT_ENCODER_A = A4;
const int RIGHT_ENCODER_B = A5;

const int ticksPerRotation = 12;

BasicMotorController left_raw_motor(LEFT_MOTOR_PWM, LEFT_MOTOR_DIR1, LEFT_MOTOR_DIR2);
BasicEncoder left_encoder(LEFT_ENCODER_A, LEFT_ENCODER_B);
CustomFeedbackMotor left_motor(left_raw_motor, left_encoder, {0.1, 0.1, 0.1}, {0.1, 0.1, 0.1}, ticksPerRotation);

BasicMotorController right_raw_motor(RIGHT_MOTOR_PWM, RIGHT_MOTOR_DIR1, RIGHT_MOTOR_DIR2);
BasicEncoder right_encoder(RIGHT_ENCODER_A, RIGHT_ENCODER_B);
CustomFeedbackMotor left_motor(right_raw_motor, right_encoder, {0.1, 0.1, 0.1}, {0.1, 0.1, 0.1}, ticksPerRotation);

FeedbackTankdrive tankdrive(9, 10, left_motor, right_motor);

void setup() {
    Serial.begin(9600);
    tankdrive.begin();
}

void loop() {
    // Read potentiometer value
    float left_vel = map(analogRead(LEFT_POT_PIN), 0, 1023, 0, 100) / 100.;
    float right_vel = map(analogRead(RIGHT_POT_PIN), 0, 1023, 0, 100) / 100.;

    tankdrive.setMotorVelocities(left_vel, right_vel);

    // Display current motor states
    Serial.print("Left Motor Current RPM: ");
    Serial.print(left_motor.getVelocity());
    Serial.print("Left Motor Target RPM: ");
    Serial.print(left_motor.getVelocityTarget());

    Serial.print("   |   ");

    Serial.print("Right Motor Current RPM: ");
    Serial.print(right_motor.getVelocity());
    Serial.print("Right Motor Target RPM: ");
    Serial.println(right_motor.getVelocityTarget());

    delay(10); // Loop delay
}
