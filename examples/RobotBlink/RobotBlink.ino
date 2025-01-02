#include <probot.h>


// Gamepad, driver station üzerinden gelen verileri işleyecek
TunaGamepad gamepad(robot);

// blink için gerekli yapılar
unsigned long last_blink = 0;
unsigned long blink_len = 500;

void basicBlink() {
  // ne kadar süre geçtiğini kontrol et
  unsigned long elapsed = (robot.getAutoElapsed() - last_blink);

  // eğer çok süre geçtiyse
  if (elapsed > blink_len) {
    // led yanıyorsa söndür, sönükse yak
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    // bu işlemi yaptığımız zamanı kaydet ki, ne kadar süre geçtiğine bakabilelim
    last_blink = robot.getAutoElapsed();
  }
}


void robotInit() {
    // Robot başlatıldığında sadece bir kez çalışacak olan kısım

    Serial.begin(115200);
    Serial.println("Robot Enabled.");
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

    // Teleop başında led kapalı olsun
    digitalWrite(LED_BUILTIN, HIGH); 
}


void teleopLoop() {
    // Teleop modu başladığında döngü halinde çalışacak kısım

    // Joystick verilerini al
    if (gamepad.getButton(Button::A)) {
        digitalWrite(LED_BUILTIN, LOW); 
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}


