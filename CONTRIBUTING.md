# Contributing to Probot Lib

Teşekkürler! `probot_lib` kütüphanemize katkıda bulunmayı düşündüğünüz için minnettarız. Aşağıda, katkıda bulunma sürecimizle ilgili bazı yönergeler bulunmaktadır. Lütfen bu kurallara uyarak projeyi daha iyi hale getirmemize yardımcı olun.

## 1. **Katkıda Bulunma Öncesi Hazırlık**

- **Issue'ları Kontrol Edin:**
  - Mevcut sorunları veya iyileştirme önerilerini [Issues](https://github.com/tunapro1234/probot_lib/issues) sekmesinde kontrol edin. Benzer bir konuda katkıda bulunmayı düşünüyorsanız, yeni bir issue oluşturun veya mevcut bir issue'ya ek bilgiler sağlayın.

- **Öneri ve Geri Bildirim:**
  - Yeni özellikler veya iyileştirmeler için [Feature Request](https://github.com/tunapro1234/probot_lib/issues/new?template=feature_request.md) formatını kullanarak önerilerde bulunun.

## 2. **Kod Katkıları**

- **Fork ve Clone:**
  1. Projenin [GitHub repository'sini](https://github.com/tunapro1234/probot_lib) fork edin.
  2. Fork ettiğiniz repository'yi yerel makinenize klonlayın:
     ```bash
     git clone https://github.com/tunapro1234/probot_lib.git
     cd probot_lib
     ```

- **Yeni Bir Dal (Branch) Oluşturun:**
  - Katkınızın türüne uygun olarak yeni bir dal oluşturun:
    ```bash
    git checkout -b feature/özellik-adi
    ```
    veya
    ```bash
    git checkout -b bugfix/sorun-adi
    ```

- **Değişiklikleri Yapın:**
  - Kodunuzu geliştirin, hata düzeltmeleri yapın veya yeni özellikler ekleyin.
  - Değişikliklerinizin kodlama standartlarına ve mevcut yapıya uygun olduğundan emin olun.

- **Test Etme:**
  - Yapılan değişikliklerin mevcut testleri geçirdiğinden emin olun.
  - Yeni eklenen özellikler için gerekli testleri yazın.

- **Commit ve Push:**
  ```bash
  git add .
  git commit -m "Özellik: [özellik-adi] ekledi" # veya "Hata Düzeltme: [sorun-adi] çözdü"
  git push origin feature/özellik-adi
