/**
 * IBT2MotorDemo - IBT-2 (BTS7960) motor surucu dogrudan kontrol ornegi
 *
 * !! UYARI !!
 * Bu ornek probot driver siniflarini KULLANMAZ.
 * Dogrudan ESP32 PWM ve GPIO ile motor kontrol eder.
 * Herhangi bir sorun yasarsaniz probot kutuphanesi sorumluluk almaz.
 * Kendi riskinizle kullanin.
 *
 * ============================================================================
 * VOLTAJ UYUMSUZLUGU - ONEMLI!
 * ============================================================================
 * - IBT-2 modulu 5V mantik seviyesi ile calisir
 * - ESP32 GPIO cikislari 3.3V seviyesindedir
 * - Cogu IBT-2 modulu 3.3V sinyali kabul eder, AMA garanti degildir
 * - Guvenli kullanim icin LOGIC LEVEL CONVERTER (3.3V -> 5V) onerılır
 * - Logic converter kullanmazsaniz modül duzgun calismayabilir
 *
 * ============================================================================
 * KONTROL MANTIGI
 * ============================================================================
 * Bu ornekte kullanilan kontrol sekli:
 *   - RPWM: Ileri yon PWM (0-255)
 *   - LPWM: Geri yon PWM (0-255)
 *   - Ileri gitmek icin: RPWM=PWM, LPWM=0
 *   - Geri gitmek icin:  RPWM=0, LPWM=PWM
 *   - Durmak icin:       RPWM=0, LPWM=0
 *
 * Bazi motor suruculer farkli mantik kullanir:
 *   - DIR + PWM (tek PWM, ayri yon pini)
 *   - IN1 + IN2 + PWM (H-koprusu kontrolu)
 *   - IN1 + IN2 (PWM dahili)
 *
 * Sizin motor surucunuz farkli calisiyorsa setMotorPower() fonksiyonunu
 * kendi ihtiyaciniza gore degistirin.
 *
 * ============================================================================
 * IBT-2 BAGLANTI SEMASI
 * ============================================================================
 *   IBT-2 Pin  |  ESP32 Pin  |  Aciklama
 *   -----------|-------------|----------------------------------
 *   RPWM       |  GPIO 5     |  Ileri PWM (veya kendi pininiz)
 *   LPWM       |  GPIO 6     |  Geri PWM (veya kendi pininiz)
 *   R_EN       |  3.3V/5V    |  Sag enable (her zaman aktif)
 *   L_EN       |  3.3V/5V    |  Sol enable (her zaman aktif)
 *   VCC        |  5V         |  Mantik beslemesi
 *   GND        |  GND        |  Ortak toprak
 *   B+/B-      |  Motor      |  Motor baglantisi
 *   M+/M-      |  Guc kaynagi|  Motor guc beslemesi (12V-24V)
 *
 * ============================================================================
 */

#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"
#define PROBOT_WIFI_AP_CHANNEL 3  // Opsiyonel (varsayilan 1)

#include <probot.h>
#include <probot/io/joystick_api.hpp>

// ============================================================================
// PIN KONFIGURASYONU - Kendi kartiniza gore degistirin!
// ============================================================================
static constexpr int PIN_RPWM = 5;   // Ileri PWM pini
static constexpr int PIN_LPWM = 6;   // Geri PWM pini

// ============================================================================
// PWM KONFIGURASYONU
// ============================================================================
static constexpr int PWM_FREQ = 20000;      // 20kHz (motor gurultusu azaltir)
static constexpr int PWM_RESOLUTION = 8;    // 8-bit (0-255 aralik)

// Motor durumu
static float g_power = 0.0f;

/**
 * Motor gucunu ayarla
 *
 * @param power -1.0 (tam geri) ile 1.0 (tam ileri) arasinda
 *
 * NOT: Bu fonksiyon IBT-2 icin yazilmistir.
 * Farkli motor surucu kullaniyorsaniz bu fonksiyonu degistirin.
 */
void setMotorPower(float power) {
  // -1.0 ile 1.0 arasinda sinirla
  power = constrain(power, -1.0f, 1.0f);
  g_power = power;

  // PWM degerini hesapla (0-255)
  int pwmValue = abs(power) * 255;

  if (power > 0.01f) {
    // Ileri: RPWM aktif, LPWM 0
    ledcWrite(PIN_RPWM, pwmValue);
    ledcWrite(PIN_LPWM, 0);
  } else if (power < -0.01f) {
    // Geri: LPWM aktif, RPWM 0
    ledcWrite(PIN_RPWM, 0);
    ledcWrite(PIN_LPWM, pwmValue);
  } else {
    // Durdur: Her iki PWM 0
    ledcWrite(PIN_RPWM, 0);
    ledcWrite(PIN_LPWM, 0);
  }
}

void robotInit() {
  Serial.begin(115200);
  delay(100);

  // ESP32 LEDC PWM kurulumu
  ledcAttach(PIN_RPWM, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(PIN_LPWM, PWM_FREQ, PWM_RESOLUTION);

  // Baslangicta durdur
  setMotorPower(0.0f);

  Serial.println("============================================");
  Serial.println("[IBT2MotorDemo] UYARI: Driver-siz ornek!");
  Serial.println("Sorun yasarsaniz probot sorumluluk almaz.");
  Serial.println("============================================");
  Serial.printf("  RPWM pin: %d, LPWM pin: %d\n", PIN_RPWM, PIN_LPWM);
  Serial.printf("  PWM freq: %d Hz, resolution: %d-bit\n", PWM_FREQ, PWM_RESOLUTION);
  Serial.println("  Logic level converter kullanmaniz onerilir!");
}

void robotEnd() {
  setMotorPower(0.0f);
  Serial.println("[IBT2MotorDemo] Motor durduruldu");
}

void teleopInit() {
  Serial.println("[IBT2MotorDemo] Sol stick Y ile motor kontrol");
}

void teleopLoop() {
  auto js = probot::io::joystick_api::makeDefault();

  float power = js.getLeftY();
  setMotorPower(power);

  probot::telemetry::printf("IBT2: %.2f\n", g_power);
  Serial.printf("[IBT2] power=%.2f\n", g_power);

  delay(50);
}

void autonomousInit() {
  setMotorPower(0.0f);
}

void autonomousLoop() {
  delay(50);
}
