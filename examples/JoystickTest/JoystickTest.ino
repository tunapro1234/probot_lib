/**
 * JoystickTest - Joystick veri okuma örneği
 *
 * Bu örnek DriverStation uygulamasından gelen joystick verilerini
 * seri port üzerinden yazdırır. Motor veya başka donanım kullanmaz.
 *
 * ============================================================================
 * NE ÖĞRENECEKSINIZ?
 * ============================================================================
 * - joystick_api arayüzünün kullanımı
 * - Analog eksenlerin okunması (stick'ler, tetikler)
 * - Dijital butonların okunması (A, B, X, Y, bumper'lar)
 * - D-pad (POV) değerinin okunması
 * - Sequence numarasıyla veri güncelliğini kontrol etme
 *
 * ============================================================================
 * JOYSTICK API GENEL BAKIŞ
 * ============================================================================
 * probot::io::joystick_api::makeDefault() ile varsayılan joystick'e
 * erişirsiniz. Bu fonksiyon bir JoystickData nesnesi döner.
 *
 * ANALOG EKSENLER (float, -1.0 ile 1.0 arası):
 *   getLeftX()   - Sol stick X ekseni (sola=-1, sağa=+1)
 *   getLeftY()   - Sol stick Y ekseni (geri=-1, ileri=+1)
 *   getRightX()  - Sağ stick X ekseni
 *   getRightY()  - Sağ stick Y ekseni
 *
 * TETİKLER (float, 0.0 ile 1.0 arası):
 *   getLeftTriggerAxis()  - Sol tetik (LT/L2)
 *   getRightTriggerAxis() - Sağ tetik (RT/R2)
 *
 * DİJİTAL BUTONLAR (bool):
 *   getA(), getB(), getX(), getY()  - Yüz butonları
 *   getLB(), getRB()                - Omuz butonları (bumper)
 *   getStart(), getBack()           - Menü butonları
 *
 * D-PAD (int, derece cinsinden):
 *   getPOV() - Yön: 0=yukarı, 90=sağ, 180=aşağı, 270=sol, -1=basılı değil
 *
 * SEQUENCE NUMARASI:
 *   getSeq() - Her yeni veri paketinde artar. Verinin güncel olup
 *              olmadığını kontrol etmek için kullanılır.
 *
 * ============================================================================
 * İPUÇLARI
 * ============================================================================
 * - Stick dead zone: Joystick ortada bile tam 0.0 vermeyebilir.
 *   Genelde |değer| < 0.1 ise 0 kabul edilir.
 *
 * - Veri güncelliği: getSeq() değeri değişmiyorsa DriverStation bağlı
 *   olmayabilir veya veri göndermiyor olabilir.
 *
 * - Teleop modu: Joystick verisi sadece teleop modunda aktif olarak
 *   güncellenir. Otonom modda joystick okunmaz.
 *
 * ============================================================================
 */

#define PROBOT_WIFI_AP_PASSWORD "ProBot1234"
#define PROBOT_WIFI_AP_CHANNEL 3  // Opsiyonel (varsayilan 1)

#include <probot.h>
#include <probot/io/joystick_api.hpp>

/**
 * Robot başlatıldığında çağrılır (bir kez)
 */
void robotInit() {
  Serial.begin(115200);
  delay(100);

  Serial.println("============================================");
  Serial.println("[JoystickTest] Joystick veri okuma örneği");
  Serial.println("============================================");
  Serial.println("DriverStation'dan bağlanın ve joystick kullanın.");
  Serial.println("");
  Serial.println("Veri formatı:");
  Serial.println("  seq   = Paket sıra numarası");
  Serial.println("  LX/LY = Sol stick X/Y ekseni");
  Serial.println("  RX/RY = Sağ stick X/Y ekseni");
  Serial.println("  A/B/X/Y = Yüz butonları (0 veya 1)");
}

/**
 * Robot durdurulduğunda çağrılır
 */
void robotEnd() {
  Serial.println("[JoystickTest] Bitti");
}

/**
 * Teleop modu başladığında çağrılır
 */
void teleopInit() {
  Serial.println("[JoystickTest] Teleop başladı - joystick verilerini izleyin");
  Serial.println("");
}

/**
 * Teleop döngüsü (sürekli çağrılır)
 *
 * Bu fonksiyon joystick verilerini okur ve hem seri porta hem de
 * telemetri sistemine yazdırır. DriverStation'da telemetri panelinde
 * bu veriler görünür.
 */
void teleopLoop() {
  // Joystick API'sinden veri al
  // makeDefault() her çağrıda en güncel veriyi döner
  auto js = probot::io::joystick_api::makeDefault();

  // =========================================================================
  // SERİ PORT ÇIKTISI
  // =========================================================================
  // Seri monitörde görünür (USB üzerinden)
  Serial.printf("seq=%lu | LX=%.2f LY=%.2f | RX=%.2f RY=%.2f | LT=%.2f RT=%.2f\n",
                static_cast<unsigned long>(js.getSeq()),
                js.getLeftX(), js.getLeftY(),
                js.getRightX(), js.getRightY(),
                js.getLeftTriggerAxis(), js.getRightTriggerAxis());

  Serial.printf("       | A=%d B=%d X=%d Y=%d | LB=%d RB=%d | POV=%d\n",
                js.getA(), js.getB(), js.getX(), js.getY(),
                js.getLB(), js.getRB(),
                js.getPOV());

  // =========================================================================
  // TELEMETRİ ÇIKTISI
  // =========================================================================
  // DriverStation uygulamasında telemetri panelinde görünür.
  // HTTP polling ile 50ms'de bir güncellenir.
  //
  // API:
  //   clear()   - Buffer'ı temizle (her döngü başında çağır)
  //   print()   - String yaz
  //   println() - String yaz + yeni satır
  //   printf()  - Formatlanmış çıktı

  // Her döngüde buffer'ı temizle, yoksa veriler birikir
  probot::telemetry::clear();

  // Başlık
  probot::telemetry::println("=== JOYSTICK TEST ===");

  // Sol stick
  probot::telemetry::printf("Sol Stick:  X=%.2f  Y=%.2f\n",
                            js.getLeftX(), js.getLeftY());

  // Sağ stick
  probot::telemetry::printf("Sag Stick:  X=%.2f  Y=%.2f\n",
                            js.getRightX(), js.getRightY());

  // Tetikler
  probot::telemetry::printf("Tetikler:   LT=%.2f  RT=%.2f\n",
                            js.getLeftTriggerAxis(), js.getRightTriggerAxis());

  // Butonlar - basılı olanları göster
  probot::telemetry::print("Butonlar:   ");
  if (js.getA()) probot::telemetry::print("A ");
  if (js.getB()) probot::telemetry::print("B ");
  if (js.getX()) probot::telemetry::print("X ");
  if (js.getY()) probot::telemetry::print("Y ");
  if (js.getLB()) probot::telemetry::print("LB ");
  if (js.getRB()) probot::telemetry::print("RB ");
  if (js.getStart()) probot::telemetry::print("START ");
  if (js.getBack()) probot::telemetry::print("BACK ");
  probot::telemetry::println("");

  // D-Pad (POV)
  int pov = js.getPOV();
  probot::telemetry::print("D-Pad:      ");
  if (pov == -1) {
    probot::telemetry::println("-");
  } else if (pov == 0) {
    probot::telemetry::println("YUKARI");
  } else if (pov == 90) {
    probot::telemetry::println("SAG");
  } else if (pov == 180) {
    probot::telemetry::println("ASAGI");
  } else if (pov == 270) {
    probot::telemetry::println("SOL");
  } else {
    probot::telemetry::printf("%d derece\n", pov);
  }

  // Sequence numarası
  probot::telemetry::printf("Seq: %lu\n", static_cast<unsigned long>(js.getSeq()));

  // 10 Hz güncelleme (100ms)
  delay(100);
}

/**
 * Otonom modu başladığında çağrılır
 */
void autonomousInit() {
  Serial.println("[JoystickTest] Otonom - joystick okumuyor");
}

/**
 * Otonom döngüsü
 *
 * Bu örnekte otonom modda bir şey yapılmıyor.
 * Joystick verisi otonom modda güncellenmez.
 */
void autonomousLoop() {
  delay(100);
}
