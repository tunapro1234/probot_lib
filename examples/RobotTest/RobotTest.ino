#include <probot.h>


// Gamepad, driver station üzerinden gelen verileri işleyecek
TunaGamepad tuna_pad(robot);

// blink için gerekli yapılar
unsigned long last_blink = 0;
unsigned long blink_len = 500;

void basicBlink() {
  unsigned long elapsed = (robot.getAutoElapsed() - last_blink);
  if (elapsed > blink_len) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    last_blink = robot.getAutoElapsed();
  }
}


void robotInit() {
    // Robot başlatıldığında sadece bir kez çalışacak olan kısım

}


void robotInitLoop() {
    // Robot başlatıldığı zamandan otonom ya da 
    // teleop başlayana kadarki sürede olacak işlemler

}


void robotEnd() {
    // Robot kapatıldığında yapılacak işlemler, sadece bir kez çalışacak

    Serial.println("Robot Disabled.");
}


void autoInit() {
    // Otonom modu başladığında bir sefer çalışacak olan kısım

    Serial.println("Autonomous Mode Started."); 
}


void autoLoop() {
    // Otonom modu başladığında döngü halinde çalışacak kısım

    basicBlink();
}


void teleopInit() {
    // Teleop modu başladığında bir sefer çalışacak olan kısım

    Serial.println("Teleop Mode Started."); 
}


void teleopLoop() {
    // Teleop modu başladığında döngü halinde çalışacak kısım

    // Joystick verilerini al
    if (tuna_pad.getButton(Button::A)) {
        digitalWrite(LED_BUILTIN, HIGH); 
    } else {
        digitalWrite(LED_BUILTIN, LOW);
    }
}


