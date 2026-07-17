# Gelecek Çalışmalar (TR)

Kütüphane 0.2.7'den beri yalnızca iletişim katmanıdır; motor/encoder/IMU
maddeleri bu listeden çıkarılmıştır (gerekirse git geçmişine bakın).

- **Pil gerilimi ölçümü:** ESP32 ADC + gerilim bölücü ile `batteryVoltage`
  alanını doldur, arayüzde göster (`/getBattery` ve UI hazır, veri yok).
- **Saha test kampanyası:** `connection-test/` düzeneği ile C senaryosu
  (30+ dk stabilite) ve B senaryosu (worst-case tek kanal) koşulmadı —
  0.2.8/0.2.9 bağlantı değişikliklerini sahada doğrula.
- **Donanım doğrulaması:** joystick hattını gerçek robotta 10 ms örnekleme
  + 20 ms kontrol döngüsüyle ölç (gecikme/jitter karakterizasyonu). Ayrıca
  havadan doğrula: 11b devre dışı bırakma gerçekten beacon'ları 6 Mbps'e
  taşıyor mu (sniffer ile), CSA kanal geçişini tabletler takip ediyor mu.
- **Telemetri tamponu:** 256 bayt yarışma sırasında küçük kalabiliyor;
  daha büyük tampon değerlendir (WS push 0.2.9'da geldi).
- **Kopma teşhisi (disconnect replay):** robot maç sırasında koptuğunda
  kopma anını, IEEE reason kodunu ve o andaki durumu (faz, son joystick,
  RSSI/ping) tek kayıtta ilişkilendir. Arayüzdeki Logs → History grafikleri
  + `Last Disconnect` alanının üstüne kurulur; "koptuğu an neredeydi/ne
  yapıyordu" sorusunu sahada cevaplar. (Tuna fikri, 2026-07-16.)
- **ESP-NOW el kumandası:** bağlantısız kontrol linki (yeniden bağlanma
  problemi yapısal olarak yok); ESP32 el kumandası + robot tarafında
  IGamepadSource implementasyonu.
- **ESP32-C5 değerlendirmesi:** 5 GHz softAP — kalabalık 2.4 GHz salon
  sorununun yapısal çözümü.

---

# Future Work (EN)

The library is communication-only since 0.2.7; motor/encoder/IMU items
were dropped from this list (see git history if needed).

- **Battery voltage:** feed `batteryVoltage` via ESP32 ADC + divider;
  `/getBattery` and the UI exist but receive no data today.
- **Field test campaign:** run connection-test scenario C (30+ min
  stability) and scenario B (worst-case single channel) to validate the
  0.2.8/0.2.9 connectivity changes under real RF load.
- **Hardware validation:** measure joystick latency/jitter on a real
  robot at 10 ms sampling + 20 ms control loop. Also verify over the
  air: does the 11b disable really move beacons to 6 Mbps (sniffer),
  and do tablets follow the CSA channel switch.
- **Telemetry buffer:** 256 bytes is tight during matches; consider a
  larger ring buffer (WS push shipped in 0.2.9).
- **ESP-NOW handheld controller:** connectionless control link (the
  reconnection problem is structurally absent); ESP32 handheld + an
  IGamepadSource implementation on the robot side.
- **ESP32-C5 evaluation:** 5 GHz softAP — the structural fix for
  crowded 2.4 GHz venues.
