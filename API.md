# Probot API Referansı (0.2.9)

Tek sayfalık tam referans. Kurulum ve örnekler için: [README.md](README.md)

## Program iskeleti

`setup()` ve `loop()` kütüphaneye aittir — sketch'te **tanımlanmaz**.
Sketch şu altı fonksiyonu tanımlamak **zorundadır** (boş olabilirler):

```cpp
void robotInit();        // Arayüzde Init'e basılınca 1 kez
void robotEnd();         // Stop'ta 1 kez — motorları güvenli konuma al
void teleopInit();       // Teleop fazı başlarken 1 kez
void teleopLoop();       // Teleop boyunca ~50 Hz tekrar çağrılır
void autonomousInit();   // Otonom fazı başlarken 1 kez
void autonomousLoop();   // Otonom boyunca ~50 Hz tekrar çağrılır
```

Faz akışı (arayüzdeki tek buton yönetir):

```
STOP ──Init──▶ INITED ──Start──▶ [AUTONOMOUS (N sn)] ──▶ TELEOP ──Stop──▶ STOP
                                  (kapatılabilir)
```

- Otonom süresi ve aç/kapa arayüzden seçilir; süre bitince teleop'a
  kendiliğinden geçilir.
- `Stop`: teleop/otonom task'ları sonlandırılır, `robotEnd()` çağrılır
  (1 sn içinde dönmezse zorla kesilir).
- Loop'lar ayrı FreeRTOS task'ında, **core 1**'de koşar. WiFi/sunucu
  core 0'dadır — kullanıcı kodu ağı yavaşlatmaz.
- Bir loop çağrısı **2 saniyeden** uzun bloke olursa "deadline miss"
  sayılır: teleop'ta uyarı verilir, otonomdaysa otonom öldürülüp
  teleop'a geçilir. LED kırmızı yanıp söner.

## Joystick

```cpp
#include <probot.h>   // joystick_api dahildir

auto js = probot::io::joystick_api::makeDefault();
```

`makeDefault()` her çağrıda hafif bir sarmalayıcı döndürür; loop içinde
her seferinde çağırmak normaldir.

| Metod | Dönüş | Açıklama |
|---|---|---|
| `getLeftX()`, `getLeftY()` | `float` -1..+1 | Sol çubuk. Y yukarı = pozitif. Deadzone 0.08 |
| `getRightX()`, `getRightY()` | `float` -1..+1 | Sağ çubuk |
| `getLeftTriggerAxis()`, `getRightTriggerAxis()` | `float` 0 / 1 | Tetikler (buton olarak okunur) |
| `getA()`, `getB()`, `getX()`, `getY()` | `bool` | Xbox isimleri |
| `getCross()`, `getCircle()`, `getSquare()`, `getTriangle()` | `bool` | PlayStation eşdeğerleri |
| `getLB()`, `getRB()` | `bool` | Omuz butonları |
| `getBack()`, `getStart()`, `getOptions()` | `bool` | Orta butonlar |
| `getLeftStickButton()`, `getRightStickButton()` | `bool` | Çubuğa basma (L3/R3) |
| `getPOV()` | `int` | D-Pad: -1 yok, 0 yukarı, 90 sağ, 180 aşağı, 270 sol |
| `getDpadUp()/Right()/Down()/Left()` | `bool` | D-Pad tek yön |
| `getRawAxis(i)`, `getRawButton(i)` | `float` / `bool` | Ham erişim (mapping'siz) |
| `isConnected()` | `bool` | En az bir eksen/buton verisi geldi mi |
| `getSeq()`, `getMs()` | `uint32_t` | Paket sayacı / son paket zamanı |
| `getAxisCount()`, `getButtonCount()` | `uint32_t` | Kumandanın bildirdiği sayılar |

**Failsafe:** joystick verisi 500 ms kesilirse tüm eksen/butonlar
sıfır okunur. Ayrıca bağlantı koptuğunda state anında sıfırlanır.
Motor kodunu doğrudan eksen değerine bağlamak güvenlidir.

**Deadzone/ayarlar:**

```cpp
probot::io::joystick_api::Options opt;
opt.deadzone = 0.12f;          // varsayılan 0.08
auto js = probot::io::joystick_api::makeDefault(opt);
```

**Kumanda eşlemesi (mapping):** varsayılan `logitech-f310` (W3C
standart düzenle aynı). Farklı kumanda için:

```cpp
probot::io::joystick_mapping::setActiveByName("standard"); // robotInit içinde
// isimler: "logitech-f310"/"f310", "standard"/"xbox"/"ds4", "axis9-dpad", "tuna-default"
```

## Telemetri

Driver Station arayüzündeki panele yazar (256 baytlık halka tampon —
eskiyen satırlar düşer):

```cpp
probot::print("merhaba");
probot::println("satır");
probot::printf("hiz=%.2f\n", hiz);
probot::clearTelemetry();
```

## Servo

50 Hz LEDC donanım PWM; kanalları üstten ayırır, `analogWrite` motor
PWM'iyle timer çakışması yaşamaz (titreme nedeni #1). Detay ve güç
uyarıları: README "Servo kullanımı".

```cpp
probot::devices::Servo kol;
kol.attach(4);                  // pin; opsiyonel: attach(pin, minUs, maxUs)
kol.write(90.0f);               // 0-180 derece
kol.writeMicroseconds(1500);    // 500-2500 µs
kol.readMicroseconds();         // son yazılan değer
kol.attached();                 // bool
kol.detach();                   // sinyali kes (servo gevşer)
```

`attach()` ilk `write()`'a kadar darbe üretmez — robot açılışta zıplamaz.

## Durum LED'i (NeoPixel)

Kütüphane durum renklerini kendisi sürer (README'de tablo). Pin
varsayılanı GPIO 3; `#define NEOPIXEL_PIN 48` ile değiştirilir.
El ile renk basmak isterseniz:

```cpp
probot::builtinled::setColor(255, 0, 255);
probot::builtinled::setBrightness(64);     // 0-255, varsayılan 32
```

## Robot durumu (ileri seviye)

```cpp
auto s = probot::robot::state().read();   // atomik snapshot
// s.status : Status::INIT/START/STOP
// s.phase  : Phase::NOT_INIT/INITED/AUTONOMOUS/TELEOP
// s.autonomousEnabled, s.autoPeriodSeconds, s.autoStartMs
// s.clientCount, s.deadlineMiss, s.batteryVoltage
```

## Yapılandırma makroları

Tamamı `#include <probot.h>`'den **önce** tanımlanır. Tablo:
README "Ayar makroları". Zorunlu olanlar: `PROBOT_WIFI_AP_PASSWORD`
(≥8 karakter) ve `PROBOT_WIFI_AP_CHANNEL` (1-13 veya 0 = otomatik).

## HTTP / WebSocket arayüzü

Robot `192.168.4.1:80`'de tek sunucu çalıştırır. Kendi DS istemcinizi
yazacaksanız:

| Endpoint | Metod | Sahiplik | Açıklama |
|---|---|---|---|
| `/` | GET | gerekli | Driver Station arayüzü (SPA) |
| `/joystick` | WS | gerekli | Binary joystick akışı (aşağıda) + 2 sn'de bir sunucudan heartbeat |
| `/updateController` | POST | gerekli | JSON fallback: `{"axes":[...],"buttons":[...]}` |
| `/robotControl?cmd=init\|start\|stop\|cancelAuto&auto=0\|1&autoLen=N` | GET | gerekli | Faz komutları |
| `/getState` | GET | gerekli | `{"phase":N,"autonomousEnabled":b,"autoPeriodSeconds":N,"autoRemainingMs":N}` |
| `/telemetry` | GET | gerekli | Telemetri tamponunun içeriği (text) |
| `/getBattery` | GET | serbest | Pil gerilimi (şu an kullanıcı beslemeli) |
| `/health` | GET | serbest | `{"rssi":N,"up":ms,"heap":N,"dm":b}` — izleme/hakem için |
| `/info` | GET | serbest | SSID, kanal, IP, çip/heap/flash bilgisi |

**Sahiplik (owner) modeli:** korumalı endpoint'e ilk istek atan IP
sahip olur; diğer IP'ler `403 Forbidden` alır. Sahip
`PROBOT_DS_OWNER_TIMEOUT_MS` (5 sn) sessiz kalırsa slot boşalır.
Sahip düştüğünde gamepad verisi anında sıfırlanır.

**WS binary joystick çerçevesi** (istemci → robot):

```
[0]      uint8   0x4A ('J' sihirli bayt)
[1]      uint8   eksen sayısı   (maks 20)
[2]      uint8   buton sayısı   (maks 20)
[3]      uint8   rezerve (0)
[4..]    int16   eksenler, big-endian, değer = float × 32767
[sonra]  uint8[] butonlar, bit-paketli, LSB önce
```

Robot → istemci: 2 saniyede bir 2 baytlık heartbeat `[0x48, seq]`.
5 saniye heartbeat alamayan istemci bağlantıyı ölü saymalıdır.

**Bağlantı kesilme zinciri:**

1. Joystick verisi 500 ms kesilir → eksenler sıfır okunur.
2. Sahip 5 sn istek atmaz → owner slotu boşalır, gamepad sıfırlanır.
3. DS 10 sn tamamen sessiz → `PROBOT_DS_TIMEOUT_FORCE_STOP=1` (varsayılan)
   ise robot STOP'a geçer; `0` ise loop'lar sürer, bağlantı dönünce
   kaldığı yerden devam eder.

## Derleme hedefleri

- Arduino IDE / arduino-cli: `library.properties` ile (`make build EXAMPLE=JoystickTest`)
- PlatformIO: `lib_deps = https://github.com/probot-studio/probot-core.git`
- ESP-IDF + Arduino component: `probot::runtime_setup()` çağırın
- Host unit testleri: `make test` (donanımsız, g++ ile)
