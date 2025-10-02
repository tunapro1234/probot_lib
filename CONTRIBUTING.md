# Katkı Rehberi (TR)

Katkınız için teşekkürler! Aşağıdaki rehber; nasıl çalıştığımızı, hangi dala PR açmanız gerektiğini ve iyi bir hata raporu/PR için neler beklediğimizi özetler.

## Çalışma Akışı ve Dallar
- `dev`: Aktif geliştirme dalı. Tüm değişiklikler önce buraya gelir.
- `stable`: Yayınlanan sürüm. Doğrudan commit yapılmaz; `dev` → PR/merge ile güncellenir.
- `gh-pages`: Dokümantasyon sitesi (https://docs.probotstudio.com/) bu daldan yayınlanır.
- `legacy`: Eski kütüphane yapısı, yalnızca inceleme amaçlı.

Önerilen akış:
1) `dev` üzerinden bir feature dalı açın: `feature/…`, `fix/…`, `docs/…`
2) Küçük ve odaklı değişiklikler yapın, örnekleri/README’yi gerekiyorsa güncelleyin
3) PR’ı `dev` hedefine açın; kısa açıklama, test adımları ve ekran çıktısı ekleyin
4) Onay sonrası `dev`’e birleştirilir; yayın döngüsünde `stable` güncellenir

Dokümantasyon katkıları için: İçerik `gh-pages` dalında tutulur. Doküman PR’larını `gh-pages` hedefine açın.

## Geliştirme Ön Koşulları
- Arduino IDE 2.x veya `arduino-cli`
- ESP32 core (ESP32‑S3 hedef kart)
- (Varsa) ilgili sensör/sürücü kütüphaneleri

> Not: Kütüphane **ESP32‑S3** hedeflenerek geliştirilmiştir. Diğer mikrokontrolcüler teknik olarak uyarlanabilir; ancak resmi destek kapsamı dışındadır. Önerilen kart: Boardoza Pulse S32‑S3 — satın alma: https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/

## Örnekleri Derlemek (Makefile)
- Listele: `make list`
- Derle: `make build EXAMPLE=ClosedLoopDemo`
- Yükle: `make upload EXAMPLE=ClosedLoopDemo PORT=/dev/ttyACM0`
- Seri monitör: `make serial` (115200 baud)

## Kod Stili ve İlkeler
- Anlamlı isimler; 1–2 harfli değişkenlerden kaçının
- Guard clause kullanın; kenar durumları önce ele alın
- Yorumları kısa ve “neden” odaklı tutun
- Boş `catch` kullanmayın; hataları anlamlı şekilde ele alın
- Public header değişikliklerinde örnekleri ve dokümanı da güncelleyin
- Değişiklikleri küçük PR’lara bölün; inceleme ve geri dönüş hızlanır

Commit mesajları (öneri): `feat: …`, `fix: …`, `docs: …`, `refactor: …`, `chore: …`

## Test Beklentileri
- En az bir örneği derleyip çalıştırın (örn. `ClosedLoopDemo`, `BasicTankDrive`)
- Seri loglarıyla temel akışı doğrulayın (115200 baud)
- Sürücü istasyonu/joystick varsa kısa bir manuel senaryo ekleyin

## Hata Bildirme (Issues)
İyi bir hata raporu hızlı çözüm getirir. Lütfen şunları ekleyin:
- Kart ve ESP32 core sürümü (örn. Boardoza Pulse S32‑S3, ESP32 core X.Y.Z)
- Örnek adı veya minimal kod parçası
- Seri loglar ve yeniden üretim adımları
- Beklenen davranış vs. gerçekleşen davranış
- İlgili commit SHA/branch

Hataları GitHub Issues’dan bildirin; doğrudan destek için WhatsApp: **+90 538 040 81 48** (Tuna Gül)

## Lisans
- MIT + Commons Clause (bkz. `LICENSE` ve `LICENSE-commercial`)
- Ticari lisans/kurumsal destek: tunagul54@gmail.com

---

# Contributing Guide (EN)

Thanks for contributing! This guide summarizes the workflow, branches, and what we expect in issues/PRs.

## Workflow and Branches
- `dev`: Active development. Open PRs against this branch.
- `stable`: Release branch. Updated via merges from `dev`.
- `gh-pages`: Documentation site (https://docs.probotstudio.com/) is published from here.
- `legacy`: Previous library layout for reference only.

Recommended flow:
1) Branch off `dev`: `feature/...`, `fix/...`, `docs/...`
2) Keep PRs small and focused; update examples/docs when user-facing behavior changes
3) Open a PR to `dev` with clear description and test steps
4) After review, merge into `dev`; `stable` is updated in the release cycle

Documentation contributions: open PRs targeting `gh-pages`.

## Prerequisites
- Arduino IDE 2.x or `arduino-cli`
- ESP32 core (target: ESP32‑S3)
- Additional libs for specific sensors/drivers when needed

> Note: The library targets **ESP32‑S3**. Other MCUs may be possible but are not officially supported. Recommended board: Boardoza Pulse S32‑S3 — purchase: https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/

## Building Examples (Makefile)
- List: `make list`
- Build: `make build EXAMPLE=ClosedLoopDemo`
- Upload: `make upload EXAMPLE=ClosedLoopDemo PORT=/dev/ttyACM0`
- Serial monitor: `make serial` (115200 baud)

## Code Style and Principles
- Clear, descriptive names; avoid 1–2 letter identifiers
- Prefer guard clauses and handle edge cases first
- Keep comments brief and “why”-focused
- Avoid empty catch blocks; handle errors meaningfully
- If public headers change, update examples and docs accordingly
- Split work into small PRs for faster reviews

Commit message convention (suggested): `feat: …`, `fix: …`, `docs: …`, `refactor: …`, `chore: …`

## Testing Expectations
- Compile and run at least one example (e.g., `ClosedLoopDemo`, `BasicTankDrive`)
- Validate basic flow via serial logs (115200 baud)
- If applicable, include a short manual scenario for driver station/joystick

## Reporting Issues
Please include:
- Board and ESP32 core version (e.g., Boardoza Pulse S32‑S3, ESP32 core X.Y.Z)
- Example name or minimal repro code
- Serial logs and reproduction steps
- Expected vs actual behavior
- Relevant commit SHA/branch

Report via GitHub Issues; for direct support: WhatsApp **+90 538 040 81 48** (Tuna Gül)

## License
- MIT + Commons Clause (see `LICENSE` and `LICENSE-commercial`)
- Commercial licensing/support: tunagul54@gmail.com 
