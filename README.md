# Probot

ESP32 tabanlı robot yarışması iletişim kütüphanesi. Robot bir WiFi
erişim noktası açar, tarayıcıdan çalışan Driver Station arayüzü sunar
ve joystick verisini WebSocket ile düşük gecikmeyle robota taşır.

**Sürüm 0.2.9** · ESP32 / ESP32-S3 · [API Referansı](API.md) ·
[English summary below](#probot-en)

---

## Kurulum

1. **Kütüphane:** Arduino IDE → Library Manager → **"probot"** ara → Install.
   Ya da en güncel sürüm için:
   ```bash
   git clone https://github.com/probot-studio/probot-core ~/Arduino/libraries/probot-core
   ```
2. **ESP32 core:** Boards Manager → "esp32" (Espressif) → **3.x** kurulu olmalı.
3. **Kart:** `ESP32S3 Dev Module` (veya `ESP32 Dev Module`).
4. **Partition:** Tools → Partition Scheme → **Huge APP (3MB No OTA)**.
   Bu ayar şart — varsayılan bölüm yetersiz, derleme sığmaz.

## İlk robot (5 dakika)

```cpp
#define PROBOT_WIFI_AP_SSID     "MyRobot"
#define PROBOT_WIFI_AP_PASSWORD "robot1234"   // en az 8 karakter
#define PROBOT_WIFI_AP_CHANNEL  1             // 1-13, veya 0 = otomatik seç
#include <probot.h>

void robotInit() {}                // Init'e basınca 1 kez
void robotEnd() {}                 // Stop'ta 1 kez — motorları burada durdur
void teleopInit() {}               // teleop başlarken 1 kez

void teleopLoop() {                // ~50 Hz tekrar çağrılır
  auto js = probot::io::joystick_api::makeDefault();
  float ileri = js.getLeftY();     // -1..+1 (ileri pozitif)
  bool  buton = js.getA();
  // motor kodun burada
  delay(20);
}

void autonomousInit() {}
void autonomousLoop() { delay(100); }
```

> `setup()` ve `loop()` **tanımlamayın** — kütüphane kendisi tanımlar.
> Altı fonksiyonun altısı da sketch'te bulunmak zorundadır.

1. Yükle → Serial monitörde IP'yi gör (`192.168.4.1`).
2. Tablet/telefonu `MyRobot` WiFi ağına bağla.
3. Tarayıcıda `http://192.168.4.1` aç.
4. Kumandayı tablete bağla (USB/Bluetooth) → **Init** → **Start**.

## Örnekler

| Örnek | Ne yapar |
|---|---|
| `JoystickTest` | Eksen/buton değerlerini Serial'e ve telemetri paneline basar. İlk deneme için. |
| `TankDrive` | Çift motor tank sürüşü (BTS7960/IBT-2 tarzı sürücü). Motor kodunun şablonu. |
| `ServoTest` | Joystick ile servo kontrolü — titreşimsiz servo kullanımının doğru yolu. |

## Ayar makroları

Hepsi `#include <probot.h>` satırından **önce** tanımlanır:

| Makro | Varsayılan | Açıklama |
|---|---|---|
| `PROBOT_WIFI_AP_SSID` | `"Probot"` | AP adı (1-32 karakter) |
| `PROBOT_WIFI_AP_PASSWORD` | — (zorunlu) | AP şifresi (≥8 karakter) |
| `PROBOT_WIFI_AP_CHANNEL` | — (zorunlu) | 1-13, veya **0 = açılışta en boş kanalı otomatik seç** |
| `PROBOT_WIFI_AP_SSID_MAC_SUFFIX` | kapalı | SSID sonuna `-XXXXXX` (MAC) ekler |
| `PROBOT_DS_TIMEOUT_MS` | `10000` | DS'ten veri kesilirse timeout (ms) |
| `PROBOT_DS_TIMEOUT_FORCE_STOP` | `1` | `1`: timeout'ta robot STOP. `0`: loop sürer, joystick nötr, bağlantı dönünce devam |
| `PROBOT_DS_OWNER_TIMEOUT_MS` | `5000` | Sahip client sessiz kalırsa slotun boşalma süresi |
| `NEOPIXEL_PIN` / `NEOPIXEL_COUNT` | `3` / `1` | Durum LED'i pini/adedi |

## Yarışma günü: kanal planı

- 2.4 GHz'te birbirini **ezmeyen** kanallar: **1, 5, 9, 13**. Aynı anda
  çalışan robotlar bu dörtlüden farklı kanallara dağıtılmalı.
- `PROBOT_WIFI_AP_CHANNEL 0` → robot açılışta ortamı tarar, en boş
  kanalı kendisi seçer (açılışa ~2-3 sn ekler). Pit alanı gibi kalabalık
  RF ortamında en pratik çözüm; seçilen kanal Serial'de ve arayüzün
  Logs sayfasında görünür.
- Telefon hotspot'ları ve seyirci cihazları da 2.4 GHz'i doldurur —
  maç sırasında robot çevresinde hotspot açtırmayın.
- Sinyal sorunlarını sahada ayıklamak için `/health` endpoint'i RSSI
  verir; -70 dBm'den kötüyse mesafe/anten sorununa bakın.

## Servo kullanımı (titreme çözümü)

Servo titremesinin iki yaygın sebebi var; ikisi de kütüphane dışında:

1. **Timer çakışması:** `analogWrite` (motorlar, 1 kHz) ile servo
   kütüphaneleri (50 Hz) aynı LEDC timer'ına düşerse biri diğerinin
   frekansını bozar. Çözüm: `probot::devices::Servo` kullanın — kanalları
   üstten ayırır, motor PWM'iyle asla çakışmaz:
   ```cpp
   probot::devices::Servo kol;
   void robotInit() { kol.attach(4); }      // GPIO 4
   void teleopLoop() { kol.write(90); ... } // 0-180°
   ```
2. **Güç:** Servoyu ESP32'nin 5V/3V3 pininden beslemeyin. WiFi anlık
   akım çekişleri gerilimi düşürür, servo seğirir. Servoya **ayrı 5-6V
   kaynak (BEC/UBEC)** verin, toprakları ortak bağlayın.

PCA9685 kullanıyorsanız: servo çıkışları için PWM frekansı **50 Hz**
olmalı (1 kHz'te servo darbe genişliği fiziksel olarak üretilemez).

## Durum LED'i

| Renk | Anlam |
|---|---|
| Mavi sabit | Açık, DS bağlı değil |
| Mavi yanıp sönüyor | DS bağlı, Init bekleniyor |
| Sarı sabit | Init tamam, Start bekleniyor |
| Turuncu yanıp sönüyor | Otonom çalışıyor |
| Yeşil yanıp sönüyor | Teleop çalışıyor |
| Kırmızı yanıp sönüyor | Deadline miss — loop 2 sn'den uzun bloke oldu |

## Bağlantı davranışı (güvenlik)

- Joystick verisi **500 ms** kesilirse eksen/butonlar otomatik sıfırlanır
  → motorlar son komutla kaçmaz.
- DS **10 sn** tamamen sessiz kalırsa robot STOP'a geçer
  (`PROBOT_DS_TIMEOUT_FORCE_STOP 0` ile yumuşak moda alınabilir).
- Aynı anda **tek client** kontrol edebilir (ilk bağlanan IP sahip olur).
  İkinci cihaz arayüzü açarsa `403` alır. `/health` ve `/info` ise
  sahiplik gerektirmez — hakem/izleme cihazları serbestçe okuyabilir.

## Yapay zeka ile kod yazma

Gemini / ChatGPT / Claude'a robot kodu yazdırırken bu satırları
prompt'unuzun başına ekleyin:

```text
ESP32 için "probot" kütüphanesiyle (0.2.9) Arduino kodu yaz.
Önce API referansını oku:
https://raw.githubusercontent.com/probot-studio/probot-core/stable/API.md
Kurallar:
- setup()/loop() TANIMLAMA; robotInit, robotEnd, teleopInit, teleopLoop,
  autonomousInit, autonomousLoop — altısı da tanımlı olacak.
- Joystick: auto js = probot::io::joystick_api::makeDefault();
  js.getLeftY() vb. (-1..+1). probot::io::gamepad() üzerinde getLeftX gibi
  metodlar YOKTUR.
- Servo için probot::devices::Servo kullan, ESP32Servo kullanma.
- teleopLoop ~50 Hz çağrılır; içinde sonsuz döngü/uzun blocking yapma.
```

Makine-okur özet: [`llms.txt`](llms.txt) · Tam referans: [`API.md`](API.md)

## Sık sorunlar

| Belirti | Çözüm |
|---|---|
| "Sketch too big" | Partition Scheme → Huge APP (3MB No OTA) |
| `#error ... PASSWORD` | Makroları `#include <probot.h>`'den önce yazın |
| Arayüz açılmıyor / 403 | Başka bir cihaz bağlı (tek client kuralı). Diğerini kapatın, ~5 sn bekleyin |
| Joystick görünmüyor | Kumandada herhangi bir tuşa basın (tarayıcı gamepad'i tuşa basılınca tanır) |
| Sık kopma | Kanal çakışması — `PROBOT_WIFI_AP_CHANNEL 0` deneyin veya 1/5/9/13'e dağıtın |
| Servo titriyor | Yukarıdaki "Servo kullanımı" bölümü |

## Destek ve lisans

- Hata bildirimi: https://github.com/probot-studio/probot-core/issues
- WhatsApp: +90 538 040 81 48
- Lisans: MIT + Commons Clause — eğitim ve yarışma kullanımı ücretsiz,
  ticari lisans için tunagul54@gmail.com

---

# Probot (EN)

ESP32 communication library for educational robotics competitions:
the robot hosts a WiFi AP and a browser-based driver station; joystick
input streams over a binary WebSocket at 50 Hz with automatic failsafes
(input zeroing after 500 ms, robot stop after 10 s of DS silence).

**Install:** Arduino IDE Library Manager → "probot", or clone
https://github.com/probot-studio/probot-core into `~/Arduino/libraries/`.
Requires arduino-esp32 core 3.x and the **Huge APP (3MB No OTA)**
partition scheme.

**Minimal sketch:** see the Turkish quick start above — the code is
identical. Define the three `PROBOT_WIFI_*` macros, include `probot.h`,
implement the six lifecycle hooks (`robotInit`, `robotEnd`,
`teleopInit`, `teleopLoop`, `autonomousInit`, `autonomousLoop`), and
read input via `probot::io::joystick_api::makeDefault()`. Do not define
`setup()`/`loop()` — the library owns them.

Full API reference: [API.md](API.md) · Machine-readable index:
[llms.txt](llms.txt) · Changes: [CHANGELOG.md](CHANGELOG.md)
