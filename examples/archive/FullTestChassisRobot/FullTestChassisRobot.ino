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


Robot robot;

// Gamepad, driver station üzerinden gelen verileri işleyecek
TunaGamepad tuna_pad(robot);

// Motor ve encoder pinleri
BasicMotorController left_motor(LEFT_MOTOR_PWM, LEFT_MOTOR_DIR1, LEFT_MOTOR_DIR2);
BasicMotorController right_motor(RIGHT_MOTOR_PWM, RIGHT_MOTOR_DIR1, RIGHT_MOTOR_DIR2);

// Basit Şasemiz
BasicTankdrive tankdrive(left_motor, right_motor);


void robotInit() {
    // Robot başlatıldığında yapılacak işlemler
    tankdrive.begin();
    Serial.print("Robot Init called!!");
}

void autoLoop() {
    // Otonom modunda yapılacak işlemler
    // Otonom modda hareket et
    if (robot.getAutoElapsed() < 3) {
        tankdrive.drive(0.1, 0);
    } else if (robot.getAutoElapsed() < 6) {
        tankdrive.drive(0, 0.1);
    } else {
        tankdrive.stop();
    }
}

void teleopLoop() {
    // Teleop modunda yapılacak işlemler
    // Joystick verilerini al
    float forwardPower = -tuna_pad.getAxis(Axis::LeftJoystickY);
    float rotationalPower = tuna_pad.getAxis(Axis::RightJoystickX);

    // Hareket et
    tankdrive.drive(forwardPower, rotationalPower);
}



void setup() {
    Serial.begin(115200);
    robot.begin();

    Serial.println("Probot initialized!");
}

void loop() {
    robot.update();

    if (robot.getMode() == RobotMode::DISABLED) {
        tankdrive.stop();
    }

    else if (robot.getMode() == RobotMode::INITIALIZED) {
        robotInit();
    }

    else if (robot.getMode() == RobotMode::AUTONOMOUS) {
        // Otonom modda hareket et
        autoLoop();
    }

    else if (robot.getMode() == RobotMode::TELEOP) {
        // Teleop modda hareket et
        teleopLoop();
    }

    float forwardPower = -tuna_pad.getAxis(Axis::LeftJoystickY);
    float rotationalPower = tuna_pad.getAxis(Axis::RightJoystickX);

    tankdrive.drive(forwardPower, rotationalPower);
}
