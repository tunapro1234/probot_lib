/**
 * MotorOpenLoopDemo - Açık çevrim motor kontrol örneği
 *
 * Bu örnek probot'un motor driver soyutlamasını kullanarak
 * tek bir motoru joystick ile kontrol eder.
 *
 * ============================================================================
 * NE ÖĞRENECEKSINIZ?
 * ============================================================================
 * - IMotorController arayüzünün kullanımı
 * - BoardozaVNH5019MotorController driver'ının kurulumu
 * - Joystick'ten motor gücü okuma
 * - Motor yön terslemesi (invert)
 * - Açık çevrim (open-loop) kontrol mantığı
 *
 * ============================================================================
 * AÇIK ÇEVRİM vs KAPALI ÇEVRİM
 * ============================================================================
 * Açık çevrim: Motora güç gönderirsin, ne olduğunu kontrol etmezsin.
 *              Encoder yok, PID yok. Basit ama hassas değil.
 *
 * Kapalı çevrim: Encoder ile hızı ölçersin, PID ile düzeltirsin.
 *                Daha karmaşık ama hassas kontrol sağlar.
 *
 * Bu örnek AÇIK ÇEVRİM kullanır - encoder gerektirmez.
 *
 * ============================================================================
 * DONANIM BAĞLANTISI
 * ============================================================================
 * Boardoza VNH5019 motor driver kullanılıyor:
 *
 *   VNH5019 Pin  |  ESP32 Pin  |  Açıklama
 *   -------------|-------------|----------------------------------
 *   INA          |  GPIO 39    |  Yön kontrolü A
 *   INB          |  GPIO 40    |  Yön kontrolü B
 *   PWM          |  GPIO 41    |  Hız kontrolü (PWM)
 *   ENA/ENB      |  -1 (yok)   |  Enable pinleri (3.3V'a bağlı)
 *
 * Pin numaralarını kendi kartınıza göre değiştirin!
 *
 * ============================================================================
 * KONTROLLER
 * ============================================================================
 * - Sol stick Y ekseni: Motor gücü (-1.0 ile 1.0 arası)
 * - Sağ tetik: Basılıyken motor yönünü tersler
 *
 * ============================================================================
 */

#define PROBOT_WIFI_AP_SSID     "ProBot"
#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"
#define PROBOT_WIFI_AP_CHANNEL  3

#include <probot.h>
#include <probot/io/joystick_api.hpp>
#include <probot/devices/motors/boardoza_vnh5019_motor_controller.hpp>

// ============================================================================
// PIN AYARLARI - Kendi kartınıza göre değiştirin!
// ============================================================================
static constexpr int PIN_INA = 39;   // Yön A pini
static constexpr int PIN_INB = 40;   // Yön B pini
static constexpr int PIN_PWM = 41;   // PWM pini
static constexpr int PIN_ENA = -1;   // Enable A (-1 = kullanılmıyor)
static constexpr int PIN_ENB = -1;   // Enable B (-1 = kullanılmıyor)

// Motor controller nesnesi
// Bu sınıf IMotorController arayüzünü uygular
static probot::motor::BoardozaVNH5019MotorController motor(PIN_INA, PIN_INB, PIN_PWM, PIN_ENA, PIN_ENB);

/**
 * Robot başlatıldığında çağrılır (bir kez)
 */
void robotInit() {
  Serial.begin(115200);
  delay(100);

  // Motor driver'ı başlat
  // Bu fonksiyon pin modlarını ayarlar ve PWM'i yapılandırır
  motor.begin();

  // Fren modu: false = coast (serbest), true = brake (fren)
  // Coast modunda motor serbest döner, brake modunda dirençle durur
  motor.setBrakeMode(false);

  // Yön terslemesi: false = normal, true = ters
  motor.setInverted(false);

  Serial.println("[MotorOpenLoopDemo] Motor driver hazır");
  Serial.println("  Sol stick Y: Motor gücü");
  Serial.println("  Sağ tetik: Yön tersle");
}

/**
 * Robot durdurulduğunda çağrılır
 */
void robotEnd() {
  // Güvenlik: Motoru durdur
  motor.setPower(0.0f);
  Serial.println("[MotorOpenLoopDemo] Motor durduruldu");
}

/**
 * Teleop modu başladığında çağrılır
 */
void teleopInit() {
  Serial.println("[MotorOpenLoopDemo] Teleop başladı");
}

/**
 * Teleop döngüsü (sürekli çağrılır)
 */
void teleopLoop() {
  // Joystick verilerini al
  auto js = probot::io::joystick_api::makeDefault();

  // Sol stick Y ekseninden motor gücü oku
  // Değer -1.0 (tam geri) ile 1.0 (tam ileri) arasında
  float power = js.getLeftY();

  // Sağ tetik basılıysa yönü tersle
  bool invert = js.getRightTriggerAxis() > 0.5f;
  motor.setInverted(invert);

  // Motora güç gönder
  // setPower() değeri -1.0 ile 1.0 arasında alır
  motor.setPower(power);

  // Debug çıktısı
  Serial.printf("[Motor] güç=%.2f ters=%d çıkış=%.2f\n",
                power, invert ? 1 : 0, motor.getPower());

  // Telemetri (DriverStation'da görünür)
  probot::telemetry::printf("Motor: %.2f\n", motor.getPower());

  delay(40);  // 25 Hz güncelleme
}

/**
 * Otonom modu başladığında çağrılır
 */
void autonomousInit() {
  Serial.println("[MotorOpenLoopDemo] Otonom başladı - motor duracak");
}

/**
 * Otonom döngüsü
 */
void autonomousLoop() {
  // Otonomda motoru durdur (joystick yok)
  motor.setPower(0.0f);
  delay(20);
}
