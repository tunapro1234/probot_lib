# Probot Lib

MEB robot yarışmaları için geliştirilmiş Arduino kütüphanesi. PID kontrolü, motion profiling, WiFi sürücü istasyonu ve ESP32-S3 desteği ile geliyor.

**GitHub Deposu:** https://github.com/nfrproducts/probot-lib

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
