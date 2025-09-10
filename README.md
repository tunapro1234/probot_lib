# Probot Lib

Probot Lib, Arduino tabanlı MEB robot yarışmalarında hızlı ve güvenilir bir başlangıç yapmanız için hazırlanmış bir kütüphanedir. ESP32‑S3 üzerinde çalışır, teleoperasyon ve otonom modlar için zamanlayıcı, durum yönetimi, sürücü istasyonu (Wi‑Fi), joystick/gampad girişi, PID tabanlı kontrolörler ve örnek uygulamalar içerir. Kütüphane modülerdir; motor, enkoder ve led gibi cihaz soyutlamaları ile kontrol katmanları birlikte çalışır.

Kütüphane, iki çekirdeğin görev dağılımını kullanır: bir çekirdek kullanıcı arayüzü ve girişleri işlerken diğer çekirdek kontrol döngüleri ve kullanıcı kodunu yürütür. Yapılandırılmış yaşam döngüsü sayesinde robotInit, teleopInit/teleopLoop ve autonomousInit/autonomousLoop fonksiyonları ile kodunuzu temiz bir şekilde bölümlendirebilirsiniz. Sürücü İstasyonu, cihazın bir erişim noktası olarak yayın yapmasını sağlar ve güvenli bağlantı için en az sekiz karakterli bir şifre zorunludur.

Projeyi klonladıktan sonra Arduino ortamında Examples menüsü üzerinden örnekleri derleyebilir ve doğrudan deneyebilirsiniz. Varsayılan örnekler BlinkPid gibi basit denemelerden, kapalı çevrim motor kontrolü ve tank sürüşüne kadar farklı seviyelerde işlevler sunar. Kod yapısı okunabilirlik ve güvenilirliğe öncelik verir; zamanlayıcı ve bekçi köpeği (watchdog) ile görevler denetlenir, durum servisi üzerinden robotun fazı ve zamanlaması izlenir.

Depo adresi günceldir ve proje bundan sonra probot-lib adıyla sürdürülür. Kaynak kod ve sorun takip için GitHub üzerindeki depo kullanılmalıdır. Belgeler ve örneklerin barındırıldığı site GitHub Pages üzerinde barındırılır ve docs.probotstudio.com alan adı üzerinden yayınlanır. Yeni linkler aşağıdadır.

Kaynak kod: https://github.com/tunapro1234/probot-lib
Belgeler: https://docs.probotstudio.com/

Lisans bilgisi ve katkı rehberi depoda yer alır. Geri bildirim ve katkılar memnuniyetle karşılanır; hata bildirmek veya öneride bulunmak için GitHub Issues bölümünü kullanabilirsiniz. Kullanım senaryoları geliştikçe belgeleri sade ve anlaşılır tutmaya özen göstereceğiz. Şu anki yapı ESP32‑S3 üzerinde yoğunlaşmıştır; farklı donanımlar eklenirse belgeler güncellenecektir.
