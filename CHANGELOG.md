# Changelog

Tüm önemli değişiklikler burada dokümante edilir.
Biçim [Keep a Changelog](https://keepachangelog.com/), sürümler
[Semantic Versioning](https://semver.org/).

---

## [0.2.9] — Yarışma Hazırlığı

Bağlantı sağlamlaştırma (devam), servo desteği ve doküman yenileme.

### Eklendi
- **Tek-WS push mimarisi:** robot, durum+sağlık (`'S'`, ≥1 Hz, heartbeat
  görevi de görür) ve telemetriyi (`'T'`, değişince) WebSocket üzerinden
  kendisi yollar; arayüz artık HTTP poll yapmıyor (eskiden ~9 istek/sn).
  WS Init/Stop boyunca açık kalır; gamepad yokken istemci 2 sn'de bir
  `'P'` keepalive yollar. WS koparsa arayüz 1 Hz HTTP fallback'e döner.
- **Captive portal:** robota bağlanan tablet/telefonda karşılama sayfası
  kendiliğinden açılır (DNS catch-all + OS sonda yakalama). IP yazmak
  gerekmez. `PROBOT_CAPTIVE_PORTAL 0` ile kapatılır.
- **Saha teşhisleri:** STA katıl/ayrıl olayları MAC + IEEE reason
  koduyla loglanır; `/health` ve `'S'` çerçevesi `joyAgeMs` (son
  joystick paketinin yaşı), `sta` (istemci sayısı) ve `disc` (son kopuş
  nedeni) alanlarını taşır; Logs sayfasında görünür.
- **Çalışma anında kanal değişimi:** Logs sayfasından kanal seçilir,
  `/setChannel` NVS'e kaydeder ve 1-13 için **CSA ile canlı geçiş**
  yapar (istemciler bağlantıyı koparmadan takip eder). NVS > makro
  önceliği; `0` = açılışta otomatik seçim.
- **Telemetri tamponu artık kilitli** (kullanıcı çekirdeği yazar, ağ
  çekirdeği okur — race vardı) ve `copyBuffer` API'si eklendi.
- **RF/TCP ince ayarları** (datasheet/IDF kaynak araştırmasına dayalı):
  TX gücü düzeltildi — `WIFI_POWER_19_5dBm` API'de aşağı yuvarlanıp
  **18 dBm** veriyordu, artık gerçek maksimum **20 dBm** kullanılıyor
  (+2 dB); 802.11b hızları varsayılan kapalı (beacon airtime 6 kat
  azalır, `PROBOT_WIFI_ENABLE_11B` ile geri açılır); httpd soketlerine
  `TCP_NODELAY` (Nagle × delayed-ACK gecikmesi biter) ve TCP keepalive
  (~7 sn'de ölü client tespiti); WS gönderimleri mutex ile sıralandı
  (eşzamanlı `httpd_ws_send_frame_async` çerçeve bozuyor, esp-idf
  #14495); `send_wait_timeout` 5 sn → 2 sn; `max_open_sockets` 10.
  Yeni makrolar: `PROBOT_INPUT_TIMEOUT_MS`, `PROBOT_WIFI_ENABLE_11B`,
  `PROBOT_WIFI_PMF_REQUIRED`.
- **`probot::devices::Servo`** (`devices/servo/servo.hpp`): 50 Hz LEDC
  donanım PWM ile servo sınıfı. Kanalları üstten ayırır — `analogWrite`
  motor PWM'iyle timer çakışması (servo titremesinin 1 numaralı yazılım
  nedeni) yapısal olarak imkânsız. `attach/write/writeMicroseconds/detach`.
- **`PROBOT_WIFI_AP_CHANNEL 0` = otomatik kanal seçimi.** Açılışta band
  taranır, 1/5/9/13 içinden en boş kanal seçilir (~2-3 sn ek açılış).
  Seçilen kanal Serial'de ve `/info`'da raporlanır.
- **`PROBOT_DS_OWNER_TIMEOUT_MS`** makrosu (varsayılan 5000) — owner
  slotunun boşalma süresi artık yapılandırılabilir.
- **Yeni örnekler:** `TankDrive` (BTS7960 tarzı çift motor) ve
  `ServoTest` (joystick ile servo).
- **Doküman seti:** README yeniden yazıldı (derlenen quick-start, kanal
  planı, servo rehberi); `API.md` tek sayfa tam referans; `llms.txt`
  (yapay zekâ araçları için kurallar + ham linkler); `keywords.txt`.
- Telemetri halka tamponu için host unit testleri.

### Değişti
- **WS ping yerine görünür heartbeat** (`ws_joystick.hpp`): sunucu artık
  WS PING yerine 2 baytlık BINARY çerçeve (`'H'`, seq) yolluyor.
  Tarayıcılar PING'i JS'e göstermediği için istemci ölü linki ayırt
  edemiyordu; şimdi `onmessage` ile gerçek canlılık sinyali var.
  Sunucu tarafındaki 3-fail kapatma mantığı aynen korundu.
- **Web UI ölü-link tespiti düzeltildi:** kendi gönderimleri artık
  aktivite sayılmıyor (ölü TCP soketine `ws.send()` sessizce başarılı
  olur — sürüş sırasında kopan bağlantı hiç fark edilmiyordu). Stale
  eşiği 3 sn → 5 sn (2 kaçan heartbeat). Boştayken yaşanan sürekli
  kopma/yeniden bağlanma döngüsü de bu sayede bitti.
- **Web UI HTTP sağlamlaştırma:** telemetri 50 ms → 150 ms; telemetri ve
  durum sorgularına eşzamanlılık kilidi + zaman aşımı eklendi (tıkanan
  hatta istek yığılması önlenir); arka plandaki sekme sorgulamaz.
- **httpd core 0'a sabitlendi** — core 1 tamamen kullanıcı koduna kaldı.
- `/info` artık makro yerine gerçek (otomatik seçilmiş olabilecek)
  kanalı döndürür. Logs sayfasındaki anlamsız "Password" satırı kalktı.

### Düzeltildi
- **Owner state yarışı:** owner alanlarına httpd task'ı ile sysloop
  task'ı eşzamanlı erişiyordu; tüm erişimler `portMUX` kritik bölgesine
  alındı (log/G-Ç kritik bölge dışında).
- **Gamepad çift yazar yarışı:** `GamepadService::write` hem WS/HTTP
  handler'larından hem owner release yolundan çağrılıyor; yazarlar artık
  spinlock ile sıralanıyor (okuyucular kilitsiz kalır).
- Kullanılmayan sabitler temizlendi (`INIT_KILL_TIMEOUT_MS`, `PRIO_STATE`,
  `PRIO_UI`, `STACK_UI`).

### Yükseltme notları
- Davranış kıran API değişikliği yok; mevcut sketch'ler aynen derlenir.
- Servo kullanan takımlar `ESP32Servo` yerine `probot::devices::Servo`'ya
  geçmeli (README "Servo kullanımı").
- Kalabalık RF ortamında `#define PROBOT_WIFI_AP_CHANNEL 0` deneyin.

---

## [0.2.8] — Bağlantı Güvenilirliği

Tek odak: **link-layer dayanıklılığı**. Davranış değiştiren API yok;
mevcut sketch'ler değişiklik gerektirmeden derlenir ve çalışır.

### Eklendi
- **`PROBOT_DS_TIMEOUT_FORCE_STOP`** makrosu (`runtime.hpp`).
  - `1` (varsayılan, önceki davranışla aynı): DS timeout'unda
    `Status::STOP` set edilir, teleop/auto task'ları sonlandırılır.
  - `0`: sadece `forceDisconnect` çağrılır, WS oturumları kapatılır,
    gamepad nötrlenir. Kullanıcı loopları çalışmaya devam eder; bağlantı
    geri gelince init/start gerekmez.

### Değişti
- **WS `/joystick` ping toleransı** (`ws_joystick.hpp`).
  Tek `httpd_ws_send_frame_async` fail'inde oturum kapatılıyordu; artık
  fd başına sayaç tutulur ve **ardışık 3 fail**'den önce kapatma
  yapılmaz. Başarılı ping sayaç sıfırlar. Gürültülü RF'de sahte
  disconnect'leri ortadan kaldırır.
- **`/health` ve `/info` endpoint'leri owner-check'siz**
  (`driver_station_esp32.hpp`). Hakem/izleme cihazları aktif sürücüyü
  etkilemeden robot sağlığını okuyabilir.
- **`/info` response'undan `pw` alanı kaldırıldı.** Endpoint artık
  açık okunabilir olduğu için AP parolası dışarı sızmamalı.
- **`/joystick` WebSocket handshake'i ve her frame için owner
  doğrulaması** (`ws_joystick.hpp`, `driver_station_esp32.hpp`).
  Daha önce handshake her client'ı kabul ediyor, owner kontrolü sadece
  HTTP route'larında uygulanıyordu. Artık ikinci bir client `/joystick`
  üzerinden paralel frame yollayamaz.
- **Owner release'de gamepad state'i sıfırlanır**
  (`driver_station_esp32.hpp`). `releaseOwner()` artık
  `_gs.write(now, nullptr, 0, nullptr, 0)` çağırır; kullanıcı kodu
  stale axis/button değerleri okuyup motorlara göndermez.

### Düzeltildi
- Yok (davranış değişiklikleri yukarıda listelendi).

### Yükseltme notları
- **Yarışmada robot mutlaka durmaya devam etsin istiyorsanız:** hiçbir
  şey yapmayın. `PROBOT_DS_TIMEOUT_FORCE_STOP` varsayılan `1`.
- **Bağlantı kesintilerinde otomatik devam etsin istiyorsanız:**
  sketch'inize şunu ekleyin:
  ```cpp
  #define PROBOT_DS_TIMEOUT_FORCE_STOP 0
  ```
- Hakem laptop'u / 2. tablet `/health` üzerinden robotları izlemek
  istiyorsa artık mümkün — 403 almaz.

---

## [0.2.7] — 2026-02-08

Önceki son yayın. Bağlantı davranışı için bkz. git `0.2.7` tag'i.

---

# Changelog (EN)

All notable changes are documented here.
Format: [Keep a Changelog](https://keepachangelog.com/).
Versioning: [Semantic Versioning](https://semver.org/).

---

## [0.2.9] — Competition Readiness

Continued link hardening, servo support, documentation overhaul.

### Added
- **Single-WS push architecture:** the robot pushes state+health (`'S'`,
  ≥1 Hz, doubles as the heartbeat) and telemetry (`'T'`, on change) over
  the WebSocket; the UI no longer polls HTTP (was ~9 req/s). The WS
  stays open across Init/Stop; with no gamepad the client sends a `'P'`
  keepalive every 2 s. If the WS drops, the UI falls back to 1 Hz HTTP.
- **Captive portal:** joining the robot AP auto-opens a landing page
  (DNS catch-all + OS probe spoofing) — no IP typing. Disable with
  `PROBOT_CAPTIVE_PORTAL 0`.
- **Field diagnostics:** STA join/leave logged with MAC + IEEE reason
  code; `/health` and the `'S'` frame carry `joyAgeMs`, `sta` and
  `disc`; surfaced on the Logs page.
- **Runtime channel switching:** pick a channel on the Logs page;
  `/setChannel` persists to NVS and switches 1-13 **live via CSA**
  (clients migrate without dropping). NVS > macro precedence; `0` =
  auto-select at boot.
- **Telemetry buffer is now locked** (user-core writer vs network-core
  reader raced) with a new `copyBuffer` API.
- **RF/TCP tuning** (grounded in datasheet/IDF source research): TX
  power fix — `WIFI_POWER_19_5dBm` quantized down to **18 dBm** in the
  API; we now request the true API max of **20 dBm** (+2 dB); 802.11b
  rates disabled by default (6x less beacon airtime; re-enable with
  `PROBOT_WIFI_ENABLE_11B`); `TCP_NODELAY` on httpd sockets (kills the
  Nagle × delayed-ACK stall) and TCP keepalive (~7 s dead-client
  detection); WS sends serialized with a mutex (concurrent
  `httpd_ws_send_frame_async` corrupts frames, esp-idf #14495);
  `send_wait_timeout` 5 s → 2 s; `max_open_sockets` 10. New macros:
  `PROBOT_INPUT_TIMEOUT_MS`, `PROBOT_WIFI_ENABLE_11B`,
  `PROBOT_WIFI_PMF_REQUIRED`.
- **`probot::devices::Servo`** (`devices/servo/servo.hpp`): hobby-servo
  class on 50 Hz LEDC hardware PWM. Channels are allocated from the top
  of the range downward, so a timer collision with `analogWrite` motor
  PWM (the #1 software cause of servo jitter) is structurally
  impossible. `attach/write/writeMicroseconds/detach`.
- **`PROBOT_WIFI_AP_CHANNEL 0` = auto channel select.** Scans the band
  at boot and picks the least congested of 1/5/9/13 (~2-3 s added boot
  time). The chosen channel is reported on Serial and `/info`.
- **`PROBOT_DS_OWNER_TIMEOUT_MS`** macro (default 5000) — the owner-slot
  idle timeout is now configurable.
- **New examples:** `TankDrive` (BTS7960-style dual motor) and
  `ServoTest` (servo from joystick).
- **Documentation set:** rewritten README (a quick-start that actually
  compiles, channel planning, servo guide); `API.md` single-page full
  reference; `llms.txt` (rules + raw links for AI tools); `keywords.txt`.
- Host unit tests for the telemetry ring buffer.

### Changed
- **Visible heartbeat instead of WS ping** (`ws_joystick.hpp`): the
  server now sends a 2-byte BINARY frame (`'H'`, seq) instead of a WS
  PING. Browsers auto-pong pings invisibly to JS, so the client could
  never tell a live link from a dead one; a data frame fires
  `onmessage` and gives a real liveness signal. The server-side
  3-consecutive-failure close logic is unchanged.
- **Web UI dead-link detection fixed:** the client no longer counts its
  own sends as link activity (`ws.send()` into a dead TCP socket
  succeeds silently — a link dying mid-drive was never detected). Stale
  threshold 3 s → 5 s (2 missed heartbeats). This also ends the
  reconnect churn loop the UI used to enter while idle.
- **Web UI HTTP hardening:** telemetry polling 50 ms → 150 ms; in-flight
  guards + timeouts on the telemetry and state pollers (no request
  pile-up on a congested link); hidden tabs stop polling.
- **httpd pinned to core 0** — core 1 is now exclusively user code.
- `/info` reports the actual (possibly auto-selected) channel instead of
  the macro. The meaningless "Password" row was removed from Logs.

### Fixed
- **Owner-state race:** owner fields were accessed concurrently from the
  httpd task and sysloop; all access now goes through a `portMUX`
  critical section (logging/I-O kept outside).
- **Gamepad dual-writer race:** `GamepadService::write` is called from
  both WS/HTTP handlers and the owner-release path; writers are now
  serialized with a spinlock (readers stay lock-free).
- Removed dead constants (`INIT_KILL_TIMEOUT_MS`, `PRIO_STATE`,
  `PRIO_UI`, `STACK_UI`).

### Upgrade notes
- No breaking API changes; existing sketches compile unchanged.
- Teams using servos should switch from `ESP32Servo` to
  `probot::devices::Servo` (see README "Servo kullanımı").
- In crowded RF environments try `#define PROBOT_WIFI_AP_CHANNEL 0`.

---

## [0.2.8] — Connection Reliability

Single theme: **link-layer hardening**. No behavioral API changes;
existing sketches keep compiling and running.

### Added
- **`PROBOT_DS_TIMEOUT_FORCE_STOP`** macro (`runtime.hpp`).
  - `1` (default, matches prior behavior): DS timeout forces
    `Status::STOP`, tears down teleop/auto tasks.
  - `0`: only `forceDisconnect` runs, WS sessions are closed, gamepad
    is zeroed. User loops keep running; no init/start needed when the
    link returns.

### Changed
- **`/joystick` WS ping tolerance** (`ws_joystick.hpp`). A single
  `httpd_ws_send_frame_async` failure used to close the session. We
  now track per-fd consecutive failures and close only after
  **three in a row**. A successful send resets the counter. Kills
  spurious disconnects under noisy RF.
- **`/health` and `/info` endpoints are owner-free**
  (`driver_station_esp32.hpp`). Judges and monitoring stations can
  observe robot health without stealing the active driver's ownership
  slot.
- **`pw` field removed from `/info`**. The endpoint is now publicly
  readable, so the AP password must not leak.
- **Owner check at `/joystick` WS handshake and per-frame**
  (`ws_joystick.hpp`, `driver_station_esp32.hpp`). The handshake used
  to accept any client; enforcement only covered HTTP routes. A second
  driver can no longer open a parallel WS and race frames.
- **Gamepad state zeroed on owner release**
  (`driver_station_esp32.hpp`). `releaseOwner()` now calls
  `_gs.write(now, nullptr, 0, nullptr, 0)` so user code doesn't read
  stale axis/button values and drive motors with them.

### Fixed
- None (behavior changes listed above).

### Upgrade notes
- **If you want the robot to always hard-stop on disconnect:** do
  nothing. `PROBOT_DS_TIMEOUT_FORCE_STOP` defaults to `1`.
- **If you want automatic recovery instead:** add to your sketch
  ```cpp
  #define PROBOT_DS_TIMEOUT_FORCE_STOP 0
  ```
- Judge laptops / secondary tablets can now poll `/health` without
  receiving 403.

---

## [0.2.7] — 2026-02-08

Previous release. See git tag `0.2.7` for reference.
