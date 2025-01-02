#include <probot.h>

// Gamepad, driver station üzerinden gelen verileri işleyecek
TunaGamepad gamepad(robot);

// Sol motora takılı motor sürücünün pwm pini
const int LEFT_MOTOR_PWM = 9;
// Sol motora takılı motor sürücünün dir1 pini
const int LEFT_MOTOR_DIR1 = 8;
// Sol motora takılı motor sürücünün dir2 pini
const int LEFT_MOTOR_DIR2 = 7;

// Sağ motora takılı motor sürücünün pwm pini
const int RIGHT_MOTOR_PWM = 6;
// Sağ motora takılı motor sürücünün dir1 pini
const int RIGHT_MOTOR_DIR1 = 5;
// Sağ motora takılı motor sürücünün dir2 pini
const int RIGHT_MOTOR_DIR2 = 4;

// Motor sürücüler
BasicMotorController left_motor(LEFT_MOTOR_PWM, LEFT_MOTOR_DIR1, LEFT_MOTOR_DIR2);
BasicMotorController right_motor(RIGHT_MOTOR_PWM, RIGHT_MOTOR_DIR1, RIGHT_MOTOR_DIR2);

// Şasemiz
BasicTankdrive tankdrive(left_motor, right_motor);


void robotInit() {
    // Robot başlatıldığında sadece bir kez çalışacak olan kısım
    Serial.begin(115200);
    Serial.println("Robot Enabled.");

    tankdrive.begin()
}


void robotEnd() {
    // Robot kapatıldığında yapılacak işlemler, sadece bir kez çalışacak
    Serial.println("Robot Disabled.");
    tankdrive.stop();
}


void autoInit() {
    // Otonom modu başladığında bir sefer çalışacak olan kısım
    Serial.println("Autonomous Mode Started."); 

}


void autoLoop() {
    // Otonom modu başladığında döngü halinde çalışacak kısım

}


void teleopInit() {
    // Teleop modu başladığında bir sefer çalışacak olan kısım
    Serial.println("Teleop Mode Started."); 

}


void teleopLoop() {
    // Teleop modu başladığında döngü halinde çalışacak kısım

    // Joystickten gelen güç değerini kaydet
    float forwardSpeed = -gamepad.getAxis(Axis::LeftJoystickY);
    float rotationalSpeed = -gamepad.getAxis(Axis::RightJoystickX);

    // Güç değerini motora yazdır
    tankdrive.drive(forwardSpeed, rotationalSpeed);
}


