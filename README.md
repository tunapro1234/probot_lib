# Probot Lib

MEB robot yarışmaları için ESP32 tabanlı Arduino kütüphanesi. Kablosuz
driver station, WiFi AP, WebSocket üzerinden düşük gecikmeli joystick
aktarımı ve FreeRTOS tabanlı çift çekirdek görev yönetimi sunar.

**Dokümantasyon:** https://docs.probotstudio.com/yazilim/

> **0.2.8** bir **bağlantı güvenilirliği** sürümüdür. Tam liste için
> aşağıdaki "Bu sürümde neler değişti" bölümüne ve `CHANGELOG.md`
> dosyasına bakın.

---

## Hızlı Başlangıç

**Kurulum (Arduino IDE):** Library Manager'dan "Probot Lib" arayıp yükleyin.

**Kurulum (arduino-cli):**
```bash
arduino-cli lib install "Probot Lib"
# veya doğrudan git deposundan:
git clone https://github.com/probot-studio/probot-core ~/Arduino/libraries/probot-core
```

**Kart ayarı:** `ESP32 Dev Module` veya `ESP32S3 Dev Module`, partition
şeması **`Huge APP (3MB No OTA)`** seçin — varsayılan 1.2MB bölümü
yetersizdir.

**İlk sketch:**
```cpp
#define PROBOT_WIFI_AP_SSID     "MyRobot"
#define PROBOT_WIFI_AP_PASSWORD "robot1234"
#define PROBOT_WIFI_AP_CHANNEL  1
#include <probot.h>

void robotInit()       {}
void robotEnd()        {}
void teleopInit()      {}
void teleopLoop() {
  auto& gp = probot::io::gamepad();
  // gp.getLeftX(), gp.getA(), … ile motor kodunu buraya yaz
}
void autonomousInit()  {}
void autonomousLoop()  { delay(100); }
```

ESP'ye yükle → `MyRobot` WiFi ağına bağlan → `http://192.168.4.1`'i aç
→ joystick'le kontrol et.

---

## Örnekler

`examples/JoystickTest/` — joystick eksenlerini ve butonlarını seri
porta ve telemetri paneline yazdırır. API'yı öğrenmek için başlangıç
noktası.

Motor sürücü entegrasyonları (tank drive, mecanum, PID, kapalı çevrim)
bu sürümde kullanıcı tarafında yazılır; referans implementasyonlar
ileriki sürümlerde gelecek.

---

## Bu sürümde neler değişti (0.2.8)

Tümü bağlantı dayanıklılığına odaklı:

1. **WS ping 3-fail toleransı** — Tek başarısız ping'de kapatma
   yerine ardışık 3 fail'de kapat. Gürültülü RF ortamında sahte
   kopmaları ortadan kaldırır.
2. **`/health` ve `/info` owner'sız** — Hakem/izleme cihazları, aktif
   sürücünün sahiplik slotunu çalmadan robot sağlığını görebilir.
3. **Owner release'de gamepad nötr** — Bağlantı kopunca son eksen/buton
   state'i buffer'dan sıfırlanır. Kullanıcı kodu stale input okuyup
   motorları sürmez.
4. **`PROBOT_DS_TIMEOUT_FORCE_STOP`** — Varsayılan `1` (güvenli,
   bağlantı kopunca robot STOP). `0` yaparsan loop çalışmaya devam
   eder, joystick nötrlenir, bağlantı dönünce restart gerekmez.
5. **`/joystick` WS handshake'de owner kontrolü** — Handshake ve her
   frame'de yeniden doğrulama. İkinci bir client slot'u ele geçiremez.

Detay: `CHANGELOG.md`.

---

## Yapılandırma makroları

`probot.h`'yi include etmeden önce tanımla:

```cpp
#define PROBOT_WIFI_AP_SSID     "RobotAdi"    // 1-32 karakter
#define PROBOT_WIFI_AP_PASSWORD "en-az-8-char" // en az 8 karakter
#define PROBOT_WIFI_AP_CHANNEL  1             // 1-13
// isteğe bağlı:
#define PROBOT_WIFI_AP_SSID_MAC_SUFFIX        // SSID'ye -XXXXXX ekle
#define PROBOT_DS_TIMEOUT_MS    10000         // DS aktivite timeout
#define PROBOT_DS_TIMEOUT_FORCE_STOP 1        // 0 = soft, 1 = STOP
```

---

## Platform desteği

- **Arduino IDE / arduino-cli**: `library.properties` üzerinden doğrudan
  derlenir. `make build EXAMPLE=JoystickTest`.
- **PlatformIO (Arduino framework)**: `lib_deps`'e path veya git URL ekle.
- **ESP-IDF + Arduino component**: `components/` altına kopyala,
  `app_main` içinden `probot::runtime_setup()` çağır.

Sürüm numarası `VERSION` dosyasından yönetilir; `make version-sync`
metadata dosyalarını eşitler.

---

## Donanım

Kütüphane **ESP32 ve ESP32-S3** ile uyumludur (WROOM, WROVER, S3 DevKit).
Test edilmiş kart: [Boardoza Pulse S32-S3](https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/).

PWM çıkışı veren herhangi bir motor sürücü kullanılabilir: VNH5019,
BTS7960B, TB6612, L298N, vb.

---

## Lisans

MIT + Commons Clause. Eğitim ve yarışma kullanımı ücretsiz; ticari
lisans için: tunagul54@gmail.com

---

## Destek

- **Dokümantasyon:** https://docs.probotstudio.com/yazilim/
- **WhatsApp:** +90 538 040 81 48
- **Hata bildirimi:** https://github.com/probot-studio/probot-core/issues

---
---

# Probot Lib (EN)

ESP32-based Arduino library for Ministry of Education robotics
competitions. Provides a wireless driver station over WiFi AP,
low-latency joystick transport via WebSocket, and a dual-core
FreeRTOS-based task manager.

**Documentation:** https://docs.probotstudio.com/yazilim/

> **0.2.8** is a **connection-reliability** release. See "What changed
> in this release" below and `CHANGELOG.md` for the full list.

---

## Quick start

**Install (Arduino IDE):** Library Manager → search "Probot Lib" → install.

**Install (arduino-cli):**
```bash
arduino-cli lib install "Probot Lib"
# or from git:
git clone https://github.com/probot-studio/probot-core ~/Arduino/libraries/probot-core
```

**Board setup:** Pick `ESP32 Dev Module` or `ESP32S3 Dev Module`. Set
partition scheme to **`Huge APP (3MB No OTA)`** — the default 1.2 MB
slot is not enough.

**Minimal sketch:**
```cpp
#define PROBOT_WIFI_AP_SSID     "MyRobot"
#define PROBOT_WIFI_AP_PASSWORD "robot1234"
#define PROBOT_WIFI_AP_CHANNEL  1
#include <probot.h>

void robotInit()       {}
void robotEnd()        {}
void teleopInit()      {}
void teleopLoop() {
  auto& gp = probot::io::gamepad();
  // drive motors with gp.getLeftX(), gp.getA(), …
}
void autonomousInit()  {}
void autonomousLoop()  { delay(100); }
```

Flash → join `MyRobot` WiFi → visit `http://192.168.4.1` → drive with a
joystick.

---

## Examples

`examples/JoystickTest/` — prints joystick axes and buttons to Serial
and the telemetry panel. Starting point for learning the API.

Motor integrations (tank drive, mecanum, PID, closed-loop) are written
by the user in this release; reference implementations will ship in a
later version.

---

## What changed in this release (0.2.8)

All focused on connection reliability:

1. **WS ping 3-fail tolerance** — Closing on a single failed ping
   caused spurious disconnects under noisy RF. The link now survives
   short interference bursts (closes only after three consecutive
   failures).
2. **`/health` and `/info` are open** — Judges and monitoring stations
   can observe robot health without stealing the active driver's
   ownership slot. `/info` no longer leaks the WiFi password.
3. **Gamepad state zeroed on owner release** — When the link dies the
   gamepad buffer is cleared so user code reading axes/buttons sees
   neutral values instead of whatever the driver held at the moment
   of disconnect.
4. **`PROBOT_DS_TIMEOUT_FORCE_STOP`** — Default `1` (safe: set Status
   to STOP when DS goes quiet). Set to `0` for a softer behavior: user
   loops keep running with neutral input, no manual restart needed on
   reconnect.
5. **Owner check at `/joystick` WS handshake and per-frame** —
   Previously the WebSocket accepted any client at handshake and only
   HTTP routes enforced ownership. Now a second driver can't open a
   parallel WS and race joystick frames.

Full details: `CHANGELOG.md`.

---

## Configuration macros

Define before including `probot.h`:

```cpp
#define PROBOT_WIFI_AP_SSID     "RobotName"   // 1-32 chars
#define PROBOT_WIFI_AP_PASSWORD "minimum8"    // >= 8 chars
#define PROBOT_WIFI_AP_CHANNEL  1             // 1-13
// optional:
#define PROBOT_WIFI_AP_SSID_MAC_SUFFIX        // append -XXXXXX
#define PROBOT_DS_TIMEOUT_MS    10000         // DS activity timeout
#define PROBOT_DS_TIMEOUT_FORCE_STOP 1        // 0 = soft, 1 = hard stop
```

---

## Platform support

- **Arduino IDE / arduino-cli**: built directly via `library.properties`.
  Try `make build EXAMPLE=JoystickTest`.
- **PlatformIO (Arduino framework)**: add via `lib_deps` path or git URL.
- **ESP-IDF + Arduino component**: drop into `components/`, call
  `probot::runtime_setup()` from `app_main`.

The single source of truth for version is `VERSION`; `make version-sync`
keeps the manifests aligned.

---

## Hardware

Works on **ESP32 and ESP32-S3** variants (WROOM, WROVER, S3 DevKit).
Tested on [Boardoza Pulse S32-S3](https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/).

Any PWM motor controller works: VNH5019, BTS7960B, TB6612, L298N, etc.

---

## License

MIT + Commons Clause. Educational and competition use is free; contact
tunagul54@gmail.com for commercial licensing.

---

## Support

- **Docs:** https://docs.probotstudio.com/yazilim/
- **WhatsApp:** +90 538 040 81 48
- **Issues:** https://github.com/probot-studio/probot-core/issues
