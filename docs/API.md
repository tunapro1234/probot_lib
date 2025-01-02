
# API Referansı

Bu belge, **Probot Lib** kütüphanesindeki temel API işlevlerini açıklar.

---

## 🚀 **Genel Yapı**

Probot Lib, robotun farklı çalışma modlarında nasıl hareket ettiğini kontrol etmek için çeşitli fonksiyonlar sağlar.

### 🛠️ **Temel Fonksiyonlar**

#### `void robotInit()`
- **Açıklama:** Robot başlatıldığında bir kere çalışır.
- **Kullanım:** Başlangıç ayarları için kullanılır.

#### `void robotEnd()`
- **Açıklama:** Robot kapatıldığında bir kere çalışır.
- **Kullanım:** Temizlik işlemleri için kullanılır.

#### `void autoInit()`
- **Açıklama:** Otonom mod başladığında bir kere çalışır.
- **Kullanım:** Otonom başlatma ayarları için kullanılır.

#### `void autoLoop()`
- **Açıklama:** Otonom mod sırasında sürekli çalışır.
- **Kullanım:** Otonom iş akışını yönetir.

#### `void teleopInit()`
- **Açıklama:** Teleop modu başladığında bir kere çalışır.
- **Kullanım:** Teleop başlangıç ayarları için kullanılır.

#### `void teleopLoop()`
- **Açıklama:** Teleop mod sırasında sürekli çalışır.
- **Kullanım:** Joystick kontrolü ve kullanıcı girdileri burada işlenir.

---

## 📑 **Örnek Kullanım**

```cpp
#include <probot.h>

void robotInit() {
  Serial.begin(115200);
  Serial.println("Robot Başlatıldı");
}

void autoInit() {
  Serial.println("Otonom Başladı");
}

void autoLoop() {
  Serial.println("Otonom Döngüsü");
}
```

## 🔗 **Kaynaklar**
- [Probot Lib Repository](https://github.com/tunapro1234/probot_lib)
- [Blink Örneği](examples/RobotBlink.md)
