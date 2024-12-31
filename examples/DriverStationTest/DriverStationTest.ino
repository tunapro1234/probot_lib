#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Harici HTML içeriğini aldığımız başlık dosyası:
#include "index_html.h"

// AP şifremiz (herkesin bağlanacağı parola):
const char* AP_PASS = "robotpro1234";

// Web sunucusu 80 portu:
ESP8266WebServer server(80);

// Dahili LED (NodeMCU D4 = GPIO2) veya farklı pin kullanabilirsiniz
const int LED_PIN = LED_BUILTIN;

// ---------------------------------------------------------
// 1) Benzersiz SSID üretmek için MAC adresinin son 3 baytını ekleyen fonksiyon
String generateSSID() {
  // MAC adresini al
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);

  // Son 3 baytı hex formatında birleştirip büyük harfe çeviriyoruz
  String macID = String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
  macID.toUpperCase();

  // Nihai SSID: "probot_" + son 3 bayt
  String ssid = "probot_" + macID;
  return ssid;
}

// ---------------------------------------------------------
// 2) mDNS’i başlatan fonksiyon (AP modunda her zaman garanti çalışmaz, 
//    çoğu cihazda STA modunda sorunsuzdur.)
void setupMDNS(const char* hostName) {
  if (MDNS.begin(hostName)) {
    Serial.println("mDNS baslatildi. 'probot.local' olarak erisilebilir.");
  } else {
    Serial.println("mDNS baslatilamadi (AP modunda destek sinirli olabilir).");
  }
}

// ---------------------------------------------------------
// "/" isteğini karşılayan, index_html.h içindeki sayfayı gönderen fonksiyon
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

// ---------------------------------------------------------
// Slider değerini alan endpoint: "/slider?val=X"
void handleSlider() {
  if (server.hasArg("val")) {
    String val = server.arg("val");
    Serial.println("Slider Value: " + val);
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // LED çıkış ayarı (NodeMCU dahili LED genelde aktif düşük)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // İlk durumda "kapalı"

  // 1) Access Point modunu başlat
  WiFi.mode(WIFI_AP);

  // 2) SSID’yi MAC'e göre üret
  String uniqueSSID = generateSSID();

  // 3) Access Point’i oluştur
  WiFi.softAP(uniqueSSID.c_str(), AP_PASS);

  // Seri Monitör’e bilgi yaz
  Serial.println("\n=== ProBot AP Olusturuldu ===");
  Serial.print("SSID: ");
  Serial.println(uniqueSSID);
  Serial.print("Sifre: ");
  Serial.println(AP_PASS);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP Adresi: ");
  Serial.println(myIP);

  // 4) mDNS hizmetini başlat (AP modunda deneme amaçlı)
  setupMDNS("probot");

  // 5) Web sunucusu endpoint’leri
  server.on("/", handleRoot);
  server.on("/slider", handleSlider);

  // 6) Sunucuyu başlat
  server.begin();
  Serial.println("Web sunucusu baslatildi. Tarayicida IP adresine girerek erisebilirsiniz.");
}

void loop() {
  // Web sunucusu isteklerini yönet
  server.handleClient();
}
