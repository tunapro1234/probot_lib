# Changelog

Tüm önemli değişiklikler burada dokümante edilir.
Biçim [Keep a Changelog](https://keepachangelog.com/), sürümler
[Semantic Versioning](https://semver.org/).

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
