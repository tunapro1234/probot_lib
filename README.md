# Probot Lib

MEB robot yarışmaları için geliştirilmiş Arduino kütüphanesi. PID kontrolü, motion profiling, WiFi sürücü istasyonu ve ESP32-S3 desteği ile geliyor.

**Tüm dokümantasyon için:** https://docs.probotstudio.com/yazilim/

---

## Hızlı Başlangıç

**Kurulum:**
Arduino IDE'nin Library Manager'ından "Probot Lib" arayıp yükleyin.

**İlk robot kodunuz:**
1. `File → Examples → Probot Lib → BasicTankDrive` açın
2. ESP32-S3'e yükleyin
3. `Probot-XXXX` WiFi ağına bağlanın
4. Tarayıcıdan `http://192.168.4.1` adresini açın
5. Joystick ile robotunuzu kontrol edin

---

## Örnekler

Kütüphane seviyelerine göre düzenlenmiş örneklerle geliyor:

**Başlangıç seviyesi:**
- `BasicTankDrive` - Tank sürüş sistemi ve joystick kontrolü
- `MotorTest` - Motor test ve kalibrasyon

**Orta seviye:**
- `ClosedLoopMotorTest` - PID tabanlı hız kontrolü
- `TankDriveAuto` - Otonom hareket (mesafe ve dönüş)

**İleri seviye:**
- `FullRobotDemo` - Tam donanımlı robot (sürüş + mekanizmalar)
- `nfr/AdvancedTankDrive` - Trajectory following ve motion profiling

Her örnek doğrudan çalışır durumda ve yorumlarla açıklanmıştır.

---

## Ne içeriyor?

Kütüphane şunları sağlar:
- WiFi tabanlı driver station (web arayüzü)
- PID, feedforward ve motion profiling
- State-space kontrol araçları (Kalman filtre, LQR)
- Tank ve mecanum sürüş soyutlamaları
- Mekanizma yardımcıları (kol, asansör, slider)
- 20ms periyotlu gerçek zamanlı görev yöneticisi

Detaylı API dokümantasyonu ve kullanım örnekleri için https://docs.probotstudio.com/yazilim/ adresini ziyaret edin.

---

## Donanım

**Önerilen:** [Boardoza Pulse S32-S3](https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/)

Kütüphane ESP32-S3 için geliştirilmiştir. Motor sürücü olarak herhangi bir PWM sürücü kullanabilirsiniz (Boardoza BA6208, TB6612, vb.)

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

Arduino library built for Ministry of Education robot competitions. Includes PID control, motion profiling, a WiFi driver station, and ESP32-S3 support.

**Full documentation:** https://docs.probotstudio.com/yazilim/

---

## Quick Start

**Installation:**
Open the Arduino IDE Library Manager, search for "Probot Lib", and install it.

**Your first robot code:**
1. Open `File → Examples → Probot Lib → BasicTankDrive`
2. Upload it to the ESP32-S3
3. Connect to the `Probot-XXXX` WiFi network
4. Visit `http://192.168.4.1` in your browser
5. Control the robot with the joystick

---

## Examples

The library ships with examples organized by proficiency level:

**Beginner level:**
- `BasicTankDrive` - Tank drive system with joystick control
- `MotorTest` - Motor testing and calibration

**Intermediate:**
- `ClosedLoopMotorTest` - PID-based speed control
- `TankDriveAuto` - Autonomous motion (distance and turn)

**Advanced:**
- `FullRobotDemo` - Full-featured robot (drive + mechanisms)
- `nfr/AdvancedTankDrive` - Trajectory following and motion profiling

Every example runs out of the box and is documented with inline comments.

---

## What's inside?

The library provides:
- WiFi-based driver station (web interface)
- PID, feedforward, and motion profiling utilities
- State-space control tools (Kalman filter, LQR)
- Tank and mecanum drive abstractions
- Mechanism helpers (arm, elevator, slider)
- A real-time task manager with a 20 ms period

For in-depth API docs and usage guides, visit https://docs.probotstudio.com/yazilim/.

---

## Hardware

**Recommended:** [Boardoza Pulse S32-S3](https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/)

The library targets the ESP32-S3. You can use any PWM motor driver (Boardoza BA6208, TB6612, etc.).

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
