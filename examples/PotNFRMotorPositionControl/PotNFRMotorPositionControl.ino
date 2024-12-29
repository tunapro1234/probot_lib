#include <probot.h>

NFRFeedbackMotor motor(1);

void setup() {
    Serial.begin(9600);

    // Initialize Motor and Encoder
    motor.begin();

    // Configure PID tolerances
    motor.setPositionTolerance(5.0); // ±5 ticks for position
    motor.setVelocityTolerance(1.0); // ±1 RPM for velocity

    // Set initial velocity target
    motor.setPositionTarget(0.0); // Target speed: 100 RPM
}

void loop() {
    // Update feedback motor control
    motor.update();

    // Display current motor state
    Serial.print("Current Position: ");
    Serial.println(motor.getPosition());
    
    Serial.print("Current Velocity (RPM): ");
    Serial.println(motor.getVelocity());
    
    // Check if the motor reached the target
    if (motor.isAtTarget()) {
        Serial.println("Target Reached!");
    }

    delay(10); // Loop delay
}
