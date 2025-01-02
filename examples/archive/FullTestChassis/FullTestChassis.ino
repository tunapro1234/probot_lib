#include <probot.h>

#define LEFT_POT_PIN A0
#define RIGHT_POT_PIN A1

// Motor and Encoder Pins
#define LEFT_MOTOR_PWM 9
#define LEFT_MOTOR_DIR1 8
#define LEFT_MOTOR_DIR2 7

#define RIGHT_MOTOR_PWM 6
#define RIGHT_MOTOR_DIR1 5
#define RIGHT_MOTOR_DIR2 4


// Driver Station, internet üzerinden arayüz paylaşacak
DriverStation driverStation;
// Gamepad, driver station üzerinden gelen verileri işleyecek
TunaGamepad tuna_pad(&driverStation);

// Motor ve encoder pinleri
BasicMotorController left_motor(LEFT_MOTOR_PWM, LEFT_MOTOR_DIR1, LEFT_MOTOR_DIR2);
BasicMotorController right_motor(RIGHT_MOTOR_PWM, RIGHT_MOTOR_DIR1, RIGHT_MOTOR_DIR2);

// Basit Şasemiz
BasicTankdrive tankdrive(left_motor, right_motor);


void setup() {
    Serial.begin(115200);
    driverStation.begin();
    tankdrive.begin();

    Serial.println("Probot initialized!");
}


void loop() {
    driverStation.handleClient();

    float forwardPower = -tuna_pad.getAxis(Axis::LeftJoystickY);
    float rotationalPower = tuna_pad.getAxis(Axis::RightJoystickX);

    tankdrive.drive(forwardPower, rotationalPower);
}