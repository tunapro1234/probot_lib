# UI Testing Helpers

Bu klasör, ESP32'ye ihtiyaç duymadan driver station arayüzünü çalıştırmak için
kullanabileceğimiz küçük bir Python sunucusu içerir. Sunucu,
`src/platform/esp32s3/web/index_html.h` dosyasındaki HTML verisini doğrudan
okur, böylece ESP'ye flash edeceğimiz içerikle birebir aynı çıktıyı görürüz.

## Başlangıç

```bash
python3 ui-testing/server.py
# veya belirli bir port için:
python3 ui-testing/server.py --port 9000 --open-browser
```

- Sunucu açıldığında `http://localhost:<port>/` adresinden UI'ı görüntüleyebilirsiniz.
- Frontend'in kullandığı `/robotControl`, `/updateController` ve `/getBattery`
  uç noktaları temel stub cevapları döndürür; bu sayede UI hata vermeden çalışır.
- Konsolda yapılan isteklere dair loglar görünür, böylece gamepad POST'ları veya
  robot komutlarını takip edebilirsiniz.

## Boyut Raporu

Arayüzün ne kadar flash alanı kapladığını ve örnek bir derlemede toplam
kullanımı görmek için:

```bash
# Sadece HTML boyutunu ölç
python3 ui-testing/server.py --report-only

# HTML + DriverStationDemo derlemesiyle toplam kullanım raporu
python3 ui-testing/server.py --report-only --build-report
```

- HTML boyutu 6584 byte civarındadır ve 4 MiB flash'ın yaklaşık %0.16'sını kaplar.
- `--build-report` seçeneği `arduino-cli` ile
  `examples/__library_impl/DriverStationDemo/DriverStationDemo.ino` dosyasını
  derler ve hem flash hem de RAM kullanımını raporlar.
  Gerekirse `--example`, `--fqbn` veya `--build-dir` argümanlarıyla özelleştirebilirsiniz.
- Sadece rapor alıp çıkmak için `--report-only` yeterlidir; sunucu başlatılmaz.

## Varyasyonlarla Çalışma

- `ui-testing/variants/` klasöründe artık temel arayüz olarak yalnızca
  `minimal_stack.html` bulunur.
- Diğer örnekler (önceki `command_deck.html` ve `pit_wall.html` dahil olmak
  üzere `aero_hud.html`, `base.html`, `circuit_glow.html`, `control_tower.html`,
  `neon_grid.html`, `retro_terminal.html`, `zen_flow.html`) referans olarak
  `ui-testing/variants/archive/` altına taşındı; dilediğiniz zaman geri almak
  için bu klasörden çıkarabilirsiniz.
- Seçili bir varyasyonu denemek için:

```bash
python3 ui-testing/server.py --html ui-testing/variants/base.html
```

- `--html` parametresi verildiğinde hem sunucu hem de boyut raporu bu dosyayı
  kullanır. Parametreyi boş bırakırsanız kütüphanedeki gömülü `index_html.h`
  okunmaya devam eder (donanım gerçekliğini test etmek için yararlı).

## Tüm Varyantları Aynı Anda Görmek

- 10 örnek tasarımı (port 9030–9039) aynı anda açmak için:

  ```bash
  python3 ui-testing/server.py --suite
  ```

- Varsayılan port aralığı 9030'dan başlar. Gerekirse `--suite-base-port` veya
  `--suite-limit` ile özelleştirebilirsiniz. Sadece rapor almak isterseniz
  `--report-only --suite` komutu yeterlidir; sunucular başlatılmaz.

## Notlar

- Sunucu HTML içeriğini her istekte yeniden okuduğu için `index_html.h`
  üzerinde yaptığınız değişiklikleri görmek için sunucuyu yeniden başlatmanıza gerek yoktur.
- Python standart kütüphanesinden başka bir bağımlılık gerektirmez.
