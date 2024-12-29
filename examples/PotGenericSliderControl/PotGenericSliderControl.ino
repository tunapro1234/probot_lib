#include <probot.h>

// Motor and Encoder Pins
const int MOTOR_PWM = 9;
const int MOTOR_DIR1 = 8;
const int MOTOR_DIR2 = 7;
const int ENCODER_PIN_A = 2;
const int ENCODER_PIN_B = 3;
const int POT_PIN = A0;

// Slider Parameters
const float SLIDER_LENGTH_CM = 50.0; // Max length in cm
const float COUNTS_PER_CM = 100.0; // Encoder ticks per cm, this should be retrieved from the manufacturer

// Instances
BasicMotorController motor(MOTOR_PWM, MOTOR_DIR1, MOTOR_DIR2);
BasicEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
PIDCoefficients pidCoeffs(1.0, 0.1, 0.05);
CustomFeedbackMotor feedbackMotor(motor, encoder, pidCoeffs, pidCoeffs, 2048);
GenericSlider slider(feedbackMotor, SLIDER_LENGTH_CM, COUNTS_PER_CM);

void setup() {
    Serial.begin(9600);
    slider.begin();
    Serial.println("Slider Example Initialized");
}

void loop() {
    int potValue = analogRead(POT_PIN);
    float targetLength = map(potValue, 0, 1023, 0, SLIDER_LENGTH_CM);

    slider.setTargetLength(targetLength);
    slider.update();

    Serial.print("Target Length: ");
    Serial.print(slider.getTargetLength());
    Serial.print(" cm | Current Length: ");
    Serial.print(slider.getLength());
    Serial.print(" cm | At Target: ");
    Serial.println(slider.isAtTarget() ? "Yes" : "No");

    delay(10);
}
