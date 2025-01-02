#include <probot.h>


// Gamepad, driver station üzerinden gelen verileri işleyecek
TunaGamepad gamepad(robot);

const int MOTOR_PWM = 9;  // Motor sürücünün pwm pini
const int MOTOR_DIR1 = 8;  // Motor sürücünün dir1 pini
const int MOTOR_DIR2 = 7;  // Motor sürücünün dir2 pini

// Motor Controller
BasicMotorController motor(MOTOR_PWM, MOTOR_DIR1, MOTOR_DIR2);

void robotInit() {
    // Robot başlatıldığında sadece bir kez çalışacak olan kısım
    Serial.begin(115200);
    Serial.println("Robot Enabled.");

    motor.begin();
}


void robotEnd() {
    // Robot kapatıldığında yapılacak işlemler, sadece bir kez çalışacak
    Serial.println("Robot Disabled.");

    motor.stop();
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
    float power = -gamepad.getAxis(Axis::LeftJoystickY);
    // Güç değerini motora yazdır
    motor.setPower(power);
}


