# Probot Lib

MEB robot yarışmalarında ekiplerin hızlı, güvenilir ve açıklayıcı bir temelle ilerlemesi için geliştirilmiş bir kütüphane. Odak: sahada işleri basitleştirmek, riskleri azaltmak ve öğrenci ekiplerin kısa sürede sonuç alabilmesini sağlamak.

- Dokümanlar: https://docs.probotstudio.com/

> Durum: Aktif geliştirme. Zaman zaman hatalarla karşılaşabilirsiniz; lütfen GitHub Issues üzerinden bildirin. Doğrudan destek için WhatsApp: +90 538 040 81 48

## Proje ne sunuyor?
Probot Lib, yarışma koşullarında ihtiyacınız olan temel taşları bir araya getirir. Amacımız karmaşık ayrıntılarla vakit kaybettirmeden takımların robotlarına odaklanmasını sağlamak.

- ESP32‑S3 üzerinde kararlı çalışma (tavsiye edilen kart aşağıda)
- Tarayıcı tabanlı sürücü istasyonu (Wi‑Fi) ve joystick entegrasyonu
- Zamanlanmış kontrol döngüleri ve anlaşılır robot yaşam döngüsü
- Motor/enkoder/LED gibi cihaz soyutlamaları ve PID tabanlı denetleyiciler
- Örneklerle hızlı ilerleme ve pratik öğrenme

Detaylı kurulum ve kullanım için dokümana bakın: https://docs.probotstudio.com/

## Neden Arduino tabanı?
Bu kütüphaneyi özellikle özgür ve erişilebilir olması için Arduino tabanı üzerine kurduk. Arduino’yu zaten bilen öğrenciler, kütüphaneye dair hiçbir şey bilmeseler bile alışık oldukları akışla hızlıca robot yapmaya başlayabilir. Ayrıca Arduino ekosistemi; sensör, sürücü ve pek çok modülü otomatik ya da çok az eforla desteklememizi sağlıyor.

## Donanım desteği
- Hedef kart: ESP32‑S3
- Önerilen kart: Boardoza Pulse S32‑S3 (ESP32‑S3) — Satın alma: https://boardoza.com/product/boardoza-pulse-s32-s3-breakout-board/
- Diğer mikrokontrolcüler: Teknik olarak uyarlamak mümkün olabilir; ancak resmi destek kapsamımızda değildir.

## Hızlı başlama (özet)
- Arduino IDE 2.x ve ESP32 desteğini kurun
- Depodaki örneklerden birini yükleyin (BasicTankDrive vb.)
- ESP’nin Wi‑Fi ağına bağlanıp sürücü istasyonunu açın
- Joystick’i test edin, ardından kendi robot kodunuza geçin

Ayrıntılar ve ekran görüntüleri: https://docs.probotstudio.com/

## Katkıda bulunma
Geri bildiriminiz ve katkınız projeyi güçlendirir. Hata/öneri/soru için GitHub Issues açın; küçük ve odaklı PR’lar hızlıca gözden geçirilir. Büyük değişiklikler için önce kısa bir tartışma başlatmanızı öneririz.

## Lisans
Proje MIT lisansı ile yayımlanır. Ticari kullanım için ek olarak Commons Clause koşulu geçerlidir.

- MIT: `LICENSE`
- Commons Clause (satış kısıtı ve ticari lisans için iletişim): `LICENSE-commercial`

Ticari lisans veya kurumsal destek için: tunagul54@gmail.com

## Dallar
- `stable`: Yayınlanan sürüm (tek commit’lik sade geçmiş)
- `dev`: Aktif geliştirme
- `gh-pages`: Dokümantasyonun yayınlandığı dal (GitHub Pages)
- `legacy`: Eski kütüphane yapısı (inceleme amaçlı)

Sorularınız için Issues açabilir veya WhatsApp’tan yazabilirsiniz. Amacımız, ekiplerin yarışma gününe daha hazırlıklı ve kendinden emin çıkmasını sağlamak.
