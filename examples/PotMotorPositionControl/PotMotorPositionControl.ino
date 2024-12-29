#include <probot.h>

// Motor and Encoder Pins
const int MOTOR_PWM = 9;  // PWM pin for motor speed
const int MOTOR_DIR1 = 8;  // Dir1 pin for motor
const int MOTOR_DIR2 = 7;  // Dir2 pin for motor
const int ENCODER_PIN_A = 2; // Encoder channel A
const int ENCODER_PIN_B = 3; // Encoder channel B

// PID Coefficients
PIDCoefficients positionCoeffs(1.0, 0.1, 0.01);
PIDCoefficients velocityCoeffs(0.8, 0.05, 0.02);

// Motor Controller and Encoder Instances
BasicMotorController motor(MOTOR_PWM, MOTOR_DIR1, MOTOR_DIR2);
BasicEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// Feedback Motor Instance
CustomFeedbackMotor feedbackMotor(motor, encoder, positionCoeffs, velocityCoeffs, 2048);

void setup() {
    Serial.begin(9600);

    // Initialize Motor and Encoder
    feedbackMotor.begin();

    // Configure PID tolerances
    feedbackMotor.setPositionTolerance(5.0); // ±5 ticks for position
    feedbackMotor.setVelocityTolerance(1.0); // ±1 RPM for velocity

    // Set initial velocity target
    feedbackMotor.setPositiontarget(0.0); // Target speed: 100 RPM
}

void loop() {
    // Read potentiometer value
    int potValue = analogRead(POT_PIN);
    long target = map(analogRead(POT_PIN), 0, 1023, 0, 500);

    feedbackMotor.setPositionTarget(target);

    // Display current motor state
    Serial.print("Current Position: ");
    Serial.println(feedbackMotor.getPosition());
    
    Serial.print("Current Velocity (RPM): ");
    Serial.println(feedbackMotor.getVelocity());
    
    // Check if the motor reached the target
    if (feedbackMotor.isAtTarget()) {
        Serial.println("Target Reached!");
    }

    // Update feedback motor control
    feedbackMotor.update();
    delay(10); // Loop delay
}
