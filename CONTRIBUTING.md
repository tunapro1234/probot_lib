# Katkı Rehberi (TR)

Teşekkürler! Bu projeye katkı sağlamak için aşağıdaki adımları izleyin.

## Başlangıç
- Depoyu Arduino/libraries/probot-lib altına koyun veya kökteki tek Makefile’ı kullanın.
- Gerekli: arduino-cli, ESP32 core, Adafruit NeoPixel (builtin LED için).

### Örnekleri derleme (tek Makefile)
- Örnekleri listele: `make list`
- Derle: `make build EXAMPLE=ClosedLoopDemo`
- Yükle: `make upload EXAMPLE=ClosedLoopDemo PORT=/dev/ttyACM0`
- Seri monitör: `make serial`

## Geliştirme İlkeleri
- Küçük ve odaklı PR’lar gönderin.
- Yeni özellikleri örneklerle gösterin/güncelleyin.
- Mevcut kod stiline ve formatına uyun.
- Public header kırıyorsanız örnekleri de güncelleyin.

## Kod Stili
- Anlamlı, açıklayıcı isimler; 1–2 harfli isimlerden kaçının.
- Guard clause kullanın; kenar durumları önce ele alın.
- Kısa ve “neden” odaklı yorumlar yazın.
- (Desteklenen platformlarda) işe yaramayan try/catch kullanmayın.

## Test
- `ClosedLoopDemo` ve `LoopPeriodStress` derleyerek kontrol döngüsü ve scheduler’ı doğrulayın.
- Platform sürücüleri `src/platform/<soc>/drivers/` altında ve makrolarla koşullandırılmış olmalı.

## Hata Bildirme
- Kart, ESP32 core sürümü ve örnek adıyla birlikte bildirin.
- Seri loglar, yeniden üretim adımları, beklenen/gerçek davranışları ekleyin.

## Lisans
- MIT + Commons Clause (LICENSE ve LICENSE-commercial bakın).

## İletişim
- Sürdürüm: Tuna Gül <tunagul54@gmail.com>

---

# Contributing Guide (EN)

Thanks for your interest in contributing!

## Getting Started
- Place this repo under Arduino/libraries/probot-lib or use the unified Makefile.
- Requirements: arduino-cli, ESP32 core, Adafruit NeoPixel (for builtin LED).

### Build examples (unified Makefile)
- List examples: `make list`
- Build example: `make build EXAMPLE=ClosedLoopDemo`
- Upload: `make upload EXAMPLE=ClosedLoopDemo PORT=/dev/ttyACM0`
- Serial monitor: `make serial`

## Development Guidelines
- Prefer small, focused PRs.
- Add/update examples to demonstrate new features.
- Match formatting and existing code style.
- If you break public headers, update examples accordingly.

## Code Style
- Clear, descriptive names (avoid 1–2 letter identifiers).
- Use guard clauses; handle edge cases first.
- Keep comments brief; explain the “why,” not the “how.”
- Avoid empty catch blocks (where applicable).

## Testing
- Compile `ClosedLoopDemo` and `LoopPeriodStress` to validate control loop and scheduler.
- Put platform drivers under `src/platform/<soc>/drivers/` and gate them with macros.

## Reporting Issues
- Include board, ESP32 core version, and example name.
- Provide serial logs, repro steps, and expected vs actual behavior.

## License
- MIT + Commons Clause (see LICENSE and LICENSE-commercial).

## Contact
- Maintainer: Tuna Gül <tunagul54@gmail.com> 