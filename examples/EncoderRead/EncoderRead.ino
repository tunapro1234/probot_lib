#include <probot.h>

// Encoder Pins
const int ENCODER_PIN_A = 2; // Encoder channel A
const int ENCODER_PIN_B = 3; // Encoder channel B

// Encoder Instance
BasicEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// Timing Variables
unsigned long lastUpdateTime = 0;
unsigned long updateInterval = 100; // Update every 100ms

// Encoder State Variables
int lastPosition = 0;

void setup() {
    Serial.begin(9600);

    // Initialize Encoder
    encoder.begin();

    Serial.println("Basic Encoder Example Initialized");
}

void loop() {
    // Read current position
    int currentPosition = encoder.getCount();
    unsigned long currentTime = millis();

    // Check if it's time to update speed
    if (currentTime - lastUpdateTime >= updateInterval) {
        // Calculate Delta Time (in seconds)
        float deltaTime = (currentTime - lastUpdateTime) / 1000.0;

        // Calculate Speed (RPM)
        int deltaTicks = currentPosition - lastPosition;
        const int TICKS_PER_REVOLUTION = 2048; // Adjust based on your encoder specs
        float speedRPM = (deltaTicks / (float)TICKS_PER_REVOLUTION) * 60.0 / deltaTime;

        // Print Encoder Position and Speed
        Serial.print("Position (Ticks): ");
        Serial.print(currentPosition);
        Serial.print(" | Speed (RPM): ");
        Serial.println(speedRPM);

        // Update tracking variables
        lastPosition = currentPosition;
        lastUpdateTime = currentTime;
    }

    delay(10); // Short delay to avoid excessive CPU usage
}
