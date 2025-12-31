# Probot Lib

MEB robot yarışmaları için geliştirilmiş Arduino kütüphanesi. PID kontrolü, WiFi sürücü istasyonu ve ESP32-S3 desteği ile geliyor.

**Tüm dokümantasyon için:** https://docs.probotstudio.com/yazilim/

---

## Hızlı Başlangıç

**Kurulum:**
Arduino IDE'nin Library Manager'ından "Probot Lib" arayıp yükleyin.

**İlk robot kodunuz:**
1. `File → Examples → Probot Lib → command_based → TankDriveDemo` açın
2. ESP32-S3'e yükleyin
3. `Probot-XXXX` WiFi ağına bağlanın
4. Tarayıcıdan `http://192.168.4.1` adresini açın
5. Joystick ile robotunuzu kontrol edin

---

## Örnekler

Kütüphane seviyelerine göre düzenlenmiş örneklerle geliyor:

**Başlangıç seviyesi:**
- `command_based/TankDriveDemo` - Tank sürüş sistemi ve joystick kontrolü
- `MotorOpenLoopDemo` - Motor kontrolcu test ve kalibrasyonu

**Orta seviye:**
- `MotorControllerDemo` - PID tabanlı hız kontrolü (PidMotorWrapper)
- `command_based/AutonomousDemo` - Otonom hareket (mesafe ve dönüş)

**İleri seviye:**
- `command_based/MecanumDriveDemo` - Mecanum sürüş ve kinematik kontrol
- `ShooterDemo` - Kapalı çevrim atıcı kontrolü

Her örnek doğrudan çalışır durumda ve yorumlarla açıklanmıştır.

---

## Platform Desteği

- **Arduino IDE / arduino-cli:** `library.properties` ve `Makefile` üzerinden doğrudan desteklenir. `make build EXAMPLE=command_based/TankDriveDemo` komutu, `arduino-cli` ile örnekleri derler.
- **PlatformIO (Arduino framework):** Kütüphaneyi `lib_deps = /path/to/probot-lib` ya da Git URL'siyle ekleyin. `library.json` sürüm bilgisi `VERSION` dosyasından otomatik güncellenir.
- **ESP-IDF + Arduino bileşeni:** Depoyu IDF projenizin `components/` klasörüne yerleştirip `idf.py build` çalıştırabilirsiniz. `idf_component.yml` otomatik olarak Arduino bileşenine bağımlıdır; `app_main` içinde `probot::runtime_setup()` çağırarak Arduino dışındaki uygulamalarda da aynı robot yaşam döngüsünü başlatabilirsiniz.

Tüm platformlarda sürüm numarası `VERSION` dosyasından yönetilir; `make version-sync` çağrısı metadata dosyalarını günceller.

---

## Ne içeriyor?

Kütüphane şunları sağlar:
- WiFi tabanlı driver station (web arayüzü)
- PID ve feedforward
- State-space kontrol araçları (Kalman filtre, LQR)
- Tank ve mecanum sürüş soyutlamaları
- Mekanizma yardımcıları (kol, asansör, slider)
- 20ms periyotlu gerçek zamanlı görev yöneticisi

Detaylı API dokümantasyonu ve kullanım örnekleri için https://docs.probotstudio.com/yazilim/ adresini ziyaret edin.

---

## Donanım

**Önerilen:** [Boardoza Pulse S32-S3](https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/)

Kütüphane ESP32-S3 için geliştirilmiştir. Motor kontrolcusu olarak herhangi bir PWM kontrolcu kullanabilirsiniz (Boardoza VNH5019, BTS7960B, TB6612, vb.)

---

## Katkıda Bulunma

Katkılarınızı bekliyoruz. Hata bildirimi veya özellik önerisi için GitHub Issues kullanabilirsiniz. Pull request'ler için küçük ve odaklı değişiklikler tercih edilir.

Geliştirme için:
```bash
git clone https://github.com/nfrproducts/probot-lib
cd probot-lib
make libs        # gerekli Arduino kütüphanelerini (Adafruit NeoPixel) kur
make test
```

Detaylar için `CONTRIBUTING.md` dosyasına bakın.

---

## Lisans

Proje MIT lisansı ile yayınlanır. Ticari kullanım için Commons Clause koşulu geçerlidir.

Eğitim ve yarışma amaçlı kullanım ücretsizdir. Ticari lisans için: tunagul54@gmail.com

---

## Destek

**Dokümantasyon:** https://docs.probotstudio.com/yazilim/  
**WhatsApp:** +90 538 040 81 48  
**Hata bildirimi:** [GitHub Issues](https://github.com/nfrproducts/probot-lib/issues)

Amacımız ekiplerin yarışma gününe hazır robotlarla çıkmasını sağlamak.

---

# Probot Lib (EN)

Arduino library built for Ministry of Education robot competitions. Includes PID control, a WiFi driver station, and ESP32-S3 support.

**Full documentation:** https://docs.probotstudio.com/yazilim/

---

## Quick Start

**Installation:**
Open the Arduino IDE Library Manager, search for "Probot Lib", and install it.

**Your first robot code:**
1. Open `File → Examples → Probot Lib → command_based → TankDriveDemo`
2. Upload it to the ESP32-S3
3. Connect to the `Probot-XXXX` WiFi network
4. Visit `http://192.168.4.1` in your browser
5. Control the robot with the joystick

---

## Examples

The library ships with examples organized by proficiency level:

**Beginner level:**
- `command_based/TankDriveDemo` - Tank drive system with joystick control
- `MotorOpenLoopDemo` - Motor controller open-loop test and calibration

**Intermediate:**
- `MotorControllerDemo` - PID-based speed control (PidMotorWrapper)
- `command_based/AutonomousDemo` - Autonomous motion (distance and turn)

**Advanced:**
- `command_based/MecanumDriveDemo` - Mecanum drive and kinematic control
- `ShooterDemo` - Closed-loop shooter control

Every example runs out of the box and is documented with inline comments.

---

## Platform Support

- **Arduino IDE / arduino-cli:** build examples with `make build EXAMPLE=command_based/TankDriveDemo`; metadata comes from `library.properties`.
- **PlatformIO (Arduino framework):** add `lib_deps = /path/to/probot-lib` or the Git URL; `library.json` stays in sync with `VERSION`.
- **ESP-IDF with the Arduino component:** drop the repository under your project's `components/` directory (or use `idf_component.yml` via the component manager) and call `idf.py build`. Invoke `probot::runtime_setup()` from `app_main()` to reuse the Arduino lifecycle on pure ESP-IDF projects.

`make version-sync` keeps all manifests aligned with the single `VERSION` file.

---

## What's inside?

The library provides:
- WiFi-based driver station (web interface)
- PID and feedforward utilities
- State-space control tools (Kalman filter, LQR)
- Tank and mecanum drive abstractions
- Mechanism helpers (arm, elevator, slider)
- A real-time task manager with a 20 ms period

For in-depth API docs and usage guides, visit https://docs.probotstudio.com/yazilim/.

---

## Hardware

**Recommended:** [Boardoza Pulse S32-S3](https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/)

The library targets the ESP32-S3. You can use any PWM motor controller board (Boardoza VNH5019, BTS7960B, TB6612, etc.).

---

## Contributing

We welcome contributions. Please use GitHub Issues for bug reports or feature requests. Keep pull requests small and focused.

For development:
```bash
git clone https://github.com/nfrproducts/probot-lib
cd probot-lib
make test
```

See `CONTRIBUTING.md` for more details.

---

## License

The project is released under the MIT license. Commercial use follows the Commons Clause terms.

Educational and competition use is free. For commercial licensing, contact: tunagul54@gmail.com

---

## Support

**Documentation:** https://docs.probotstudio.com/yazilim/  
**WhatsApp:** +90 538 040 81 48  
**Bug reports:** [GitHub Issues](https://github.com/nfrproducts/probot-lib/issues)

Our goal is to help teams arrive on competition day with ready-to-run robots.
