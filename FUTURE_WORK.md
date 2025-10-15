# Gelecek Çalışmalar Notları (TR)

- **S-Eğrisi Motion Profile Desteği**: Önceden hesaplanmış trajeler için yüksek bellek tüketimi (en kötü 720KB) nedeniyle şu anda devre dışı. Gelecekteki seçenekler: (1) Kaydırmalı pencere yaklaşımı (motor başına 2.4KB), (2) Analitik formülasyon (dinamik bellek yok), (3) Hibrit yaklaşım. Trapez profiller mevcut ve çoğu kullanım senaryosu için yeterli.
- Resmi ölçüler geldikten sonra NFR şasi geometrisi sabitlerini (iz genişliği, dingil mesafesi, tekerlek çapı) güncelle.
- Boardoza motor kontrolcüsü desteğini entegre et (donanım ekibinden gelecek PWM/CAN detayları bekleniyor).
- ESP32-S3 için donanımsal quadrature encoder sürücüsü ekle (temel örnek: PCNT + 1024 CPR tekerlek).
- IMotorController arayüzünü PIDF + feedforward slotları ve isteğe bağlı motion profile zamanlaması (trapez/S-eğrisi) ile genişlet.
- NFR örneklerinde kullanılan NullMotorController yer tutucularını gerçek IMotorController uygulamalarıyla değiştir.
- Her aktarma için motor motion profile ve feedforward ön ayarlarını yapılandıracak şasi seviyesinde yardımcı fonksiyonlar sun.
- MPU6050 entegrasyonunu sağlamlaştır (kalibrasyon akışı, hata yönetimi) ve ilerideki MPU9050/BNO varyantlarına hazırlık yap.
- Robotlar hazır olduğunda joystick hattını donanım üzerinde 10 ms örnekleme + 20 ms kontrol döngüsü ile doğrula.
- Nihai şasi parametrelerini kullanarak otonom şablonlar ekle (10 cm ileri → 90° dönüş → 10 cm ileri).
- Robotlar hazır olduğunda donanım-iç-döngü ve saha test kampanyasını planla.
- ESP32 ADC (gerilim bölücü devresi) ile pil gerilimi ölçümü uygula ve driver station arayüzünde göster.
- Yarışma sırasında bağlantı koptuğunda driver station için otomatik WiFi yeniden bağlanma mekanizması ekle.
- Motion profile bellek kullanımını gözden geçirip optimize et (SCurveProfile en kötü durumda 720KB ayırabiliyor).

---

# Future Work Notes (EN)

- **S-Curve Motion Profile Support**: Currently disabled due to high memory usage (up to 720KB worst-case for pre-computed trajectories). Future implementation options: (1) Sliding window approach (2.4KB per motor), (2) Analytical formulation (zero dynamic memory), or (3) Hybrid approach. Trapezoid profiles are available and sufficient for most use cases.
- Update NFR chassis geometry constants (track width, wheel base, wheel diameter) once official dimensions arrive.
- Integrate Boardoza motor controller support (PWM/CAN specifics pending from hardware team).
- Add hardware quadrature encoder driver implementation for ESP32-S3 (baseline example: PCNT + 1024 CPR wheel).
- Extend IMotorController to PIDF + feedforward slots and optional motion profile scheduling (trapezoid/S-curve).
- Swap placeholder NullMotorController usages with real IMotorController implementations in NFR examples.
- Expose chassis-level helpers to configure motor motion profiles and feedforward presets per drivetrain.
- Harden MPU6050 integration (calibration flow, failure handling) and prepare for future MPU9050/BNO variants.
- Validate joystick pipeline at 10 ms sampling + 20 ms control loop on hardware once robots are available.
- Add autonomous templates (10 cm forward → 90° turn → 10 cm forward) using finalized chassis parameters.
- Plan hardware-in-the-loop / field testing campaign when robots are ready.
- Implement battery voltage measurement using ESP32 ADC (voltage divider circuit) and expose via driver station UI.
- Add WiFi auto-reconnection mechanism for driver station when connection drops during competition.
- Review and optimize memory usage for motion profiles (SCurveProfile can allocate up to 720KB in worst case).
