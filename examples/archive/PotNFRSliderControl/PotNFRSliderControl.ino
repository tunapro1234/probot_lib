#include <probot.h>

const int POT_PIN = A0;

// Slider with NFRFeedbackMotor, id is the motor controller can id
NFRSlider slider(3);

void setup() {
    Serial.begin(9600);
    slider.begin();
    Serial.println("Slider Example Initialized");
}

void loop() {
    int potValue = analogRead(POT_PIN);
    float targetLength = map(potValue, 0, 1023, 0, slider.getMaxLength()-10);

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
