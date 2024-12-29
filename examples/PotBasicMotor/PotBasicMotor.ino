#include <probot.h>

// Motor and Encoder Pins
const int MOTOR_PWM = 9;  // PWM pin for motor speed
const int MOTOR_DIR = 8;  // Direction pin for motor
const int ENCODER_PIN_A = 2; // Encoder channel A
const int ENCODER_PIN_B = 3; // Encoder channel B

// Potentiometer Pin
const int POT_PIN = A0; // Analog pin for potentiometer

// PID Coefficients for Position Control
PIDCoefficients positionCoeffs(1.5, 0.1, 0.05);
PIDCoefficients velocityCoeffs(0.8, 0.05, 0.02);

// Motor and Encoder Instances
BasicMotorController motor(MOTOR_PWM, MOTOR_DIR);
BasicEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// Feedback Motor Instance
CustomFeedbackMotor feedbackMotor(motor, encoder, positionCoeffs, velocityCoeffs, 2048);

void setup() {
    Serial.begin(9600);

    // Initialize Motor and Encoder
    feedbackMotor.begin();

    // Set Position PID coefficients and tolerance
    feedbackMotor.setPositionPID(positionCoeffs);
    feedbackMotor.setPositionTolerance(5.0); // ±5 ticks for position

    Serial.println("Potentiometer Position Control Initialized");
}

void loop() {
    // Read Potentiometer Value (0-1023)
    int potValue = analogRead(POT_PIN);

    // Map Potentiometer Value to a Position Target (0-1000 ticks)
    float positionTarget = map(potValue, 0, 1023, 0, 1000);

    // Set Motor to Target Position
    feedbackMotor.setPositionTarget(positionTarget);
    feedbackMotor.update();

    // Print Current Position and Target
    Serial.print("Potentiometer Value: ");
    Serial.print(potValue);
    Serial.print(" | Position Target: ");
    Serial.print(positionTarget);
    Serial.print(" | Current Position: ");
    Serial.println(feedbackMotor.getPosition());

    // Check if the motor reached the target
    if (feedbackMotor.isAtTarget()) {
        Serial.println("Target Position Reached!");
    }

    delay(10); // Adjust refresh rate
}
