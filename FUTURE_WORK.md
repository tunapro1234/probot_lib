# Gelecek Çalışmalar (TR)

Kütüphane 0.2.7'den beri yalnızca iletişim katmanıdır; motor/encoder/IMU
maddeleri bu listeden çıkarılmıştır (gerekirse git geçmişine bakın).

- **Pil gerilimi ölçümü:** ESP32 ADC + gerilim bölücü ile `batteryVoltage`
  alanını doldur, arayüzde göster (`/getBattery` ve UI hazır, veri yok).
- **Saha test kampanyası:** `connection-test/` düzeneği ile C senaryosu
  (30+ dk stabilite) ve B senaryosu (worst-case tek kanal) koşulmadı —
  0.2.8/0.2.9 bağlantı değişikliklerini sahada doğrula.
- **Kanal değişikliği için NVS:** kanalı yeniden derlemeden değiştirmek
  için arayüzden seçim + NVS'te saklama (şimdilik `CHANNEL 0` otomatik
  seçim var).
- **Donanım doğrulaması:** joystick hattını gerçek robotta 10 ms örnekleme
  + 20 ms kontrol döngüsüyle ölç (gecikme/jitter karakterizasyonu).
- **Telemetri tamponu:** 256 bayt yarışma sırasında küçük kalabiliyor;
  WS üzerinden push + daha büyük tampon değerlendir.

---

# Future Work (EN)

The library is communication-only since 0.2.7; motor/encoder/IMU items
were dropped from this list (see git history if needed).

- **Battery voltage:** feed `batteryVoltage` via ESP32 ADC + divider;
  `/getBattery` and the UI exist but receive no data today.
- **Field test campaign:** run connection-test scenario C (30+ min
  stability) and scenario B (worst-case single channel) to validate the
  0.2.8/0.2.9 connectivity changes under real RF load.
- **NVS channel override:** change the WiFi channel from the UI without
  reflashing (compile-time `CHANNEL 0` auto-select exists today).
- **Hardware validation:** measure joystick latency/jitter on a real
  robot at 10 ms sampling + 20 ms control loop.
- **Telemetry buffer:** 256 bytes is tight during matches; consider WS
  push and a larger ring buffer.
