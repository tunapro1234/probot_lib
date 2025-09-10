# Probot Lib

Bu depo, MEB robot yarışmaları için pratik, güvenilir ve sade bir başlangıç kütüphanesidir. ESP32‑S3 üzerinde çalışır; teleoperasyon ve otonom modları destekler.

- Dokümanlar: https://docs.probotstudio.com/
- Kaynak kod: https://github.com/tunapro1234/probot-lib

## Nedir, ne sağlar?
Probot Lib, takımların temel altyapı ile uğraşmadan doğrudan robotlarına odaklanabilmeleri için yazıldı. Amaç; kurulumda sürünmemek, yarışma gününde sürpriz yaşamamak ve yeni başlayan öğrencilerin hızla sonuç almasını sağlamak.

- Basit başlangıç: Örnekler üzerinden "çalışan" bir robota hızlıca ulaşılır.
- Tutarlı çalışma: Kontrol döngüleri ve görev zamanlaması önceden çözümlenmiştir.
- Sürücü istasyonu: Wi‑Fi üzerinden tarayıcı tabanlı arayüz ve joystick girişi.
- Modüler yapı: Motor, enkoder, LED ve denetleyiciler (PID vb.) birlikte çalışır.
- Genişletilebilirlik: Kendi cihaz/alt sistemlerinizi eklemek kolaydır.

Detaylar, kurulum ve kullanım senaryoları için dokümana bakın: https://docs.probotstudio.com/

## Hızlı bakış (özet)
- Hedef kart: ESP32‑S3 (diğerleri için uyarlama yapılabilir)
- Örnekler: `File > Examples > Probot Lib` altında
- Seri haberleşme: 115200 baud

## Katkıda bulunma
Katkılarınızı bekliyoruz. Küçük düzeltmelerden yeni örneklere kadar her katkı değerlidir.

- Issue açın: Hata, öneri veya soru → "Issues" kısmından
- Küçük PR'lar: Net kapsamlı, odaklı değişiklikler hızlıca gözden geçirilir
- Yeni örnekler: Öğrencilerin hızlıca anlayacağı sade örnekler tercih edilir

PR sürecinde açıklayıcı başlık/açıklama, küçük parçalı commit’ler ve mümkünse kısa bir test notu rica ederiz. Büyük değişiklikler için önce bir tartışma başlatmanız iyi olur.

## Lisans
Proje MIT lisansı ile yayımlanır. Ticari kullanım için ek olarak Commons Clause koşulu geçerlidir.

- MIT: `LICENSE`
- Commons Clause (satış kısıtı ve ticari lisans için iletişim): `LICENSE-commercial`

Ticari lisans veya özel destek için iletişim: tunagul54@gmail.com

## Durum ve yol haritası
- `stable`: Hazır, tek commit’lik sade geçmiş
- `dev`: Geliştirme dalı
- `legacy`: Eski sürümler (inceleme amaçlı)

Geri bildirim, soru ve önerilerinizi bekliyoruz. Amacımız; öğrencilerin hızlıca üretmesine yardım eden güvenilir ve anlaşılır bir temel sunmak.
