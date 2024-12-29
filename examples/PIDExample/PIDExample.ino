
#include <probot.h>

// Motor and Encoder Pins
const int MOTOR_PWM = 9;    // PWM pin for motor speed
const int MOTOR_DIR1 = 8;    // Dir1 pin for motor
const int MOTOR_DIR2 = 7;    // Dir2 pin for motor
const int ENCODER_PIN_A = 2; // Encoder channel A
const int ENCODER_PIN_B = 3; // Encoder channel B

// Potentiometer Pins
const int POT_KP = A0; // Potentiometer for Kp
const int POT_KI = A1; // Potentiometer for Ki
const int POT_KD = A2; // Potentiometer for Kd
const int POT_TARGET = A3; // Potentiometer for Target

// PID Coefficients
PIDCoefficients pidCoeffs(1.0, 0.1, 0.05);

// Motor and Encoder Instances
BasicMotorController motor(MOTOR_PWM, MOTOR_DIR1, MOTOR_DIR2);
BasicEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// Feedback Motor Instance
CustomFeedbackMotor feedbackMotor(motor, encoder, pidCoeffs, pidCoeffs, 2048);

void setup() {
    Serial.begin(9600);

    // Initialize Motor and Encoder
    feedbackMotor.begin();

    // Set initial PID Tolerances
    feedbackMotor.setPositionTolerance(5.0);

    Serial.println("PID Learning Example Initialized");
}

void loop() {
    // Read Potentiometer Values
    float kp = map(analogRead(POT_KP), 0, 1023, 0, 200) / 10.0; // Map to 0.0 - 20.0
    float ki = map(analogRead(POT_KI), 0, 1023, 0, 100) / 100.0; // Map to 0.0 - 1.0
    float kd = map(analogRead(POT_KD), 0, 1023, 0, 100) / 100.0; // Map to 0.0 - 1.0
    float target = map(analogRead(POT_TARGET), 0, 1023, 0, 1000); // Map to 0 - 1000 ticks

    // Update PID Coefficients
    PIDCoefficients newCoeffs(kp, ki, kd);
    feedbackMotor.setPositionPID(newCoeffs);

    // Update Target
    feedbackMotor.setPositionTarget(target);

    // Update Motor
    feedbackMotor.update();

    // Display PID Coefficients and Motor State
    Serial.print("Kp: "); Serial.print(kp);
    Serial.print(" | Ki: "); Serial.print(ki);
    Serial.print(" | Kd: "); Serial.print(kd);
    Serial.print(" | Target: "); Serial.print(target);
    Serial.print(" | Position: "); Serial.print(feedbackMotor.getPosition());
    Serial.print(" | At Target: "); Serial.println(feedbackMotor.isAtTarget() ? "Yes" : "No");

    delay(10); // Slow down the refresh rate
}
