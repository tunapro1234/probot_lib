#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "index_html.h"

// Herkesin kullanacağı parola (Access Point için):
const char* AP_PASS = "robotpro1234";

ESP8266WebServer server(80);

// Dahili LED (NodeMCU'da D4 = GPIO2), test amaçlı kullanabiliriz
const int LED_PIN = LED_BUILTIN;

// ----------------------------
// Benzersiz SSID üreten fonksiyon
String generateSSID() {
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  // Son 3 baytı hex olarak birleştir
  String macID = String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
  macID.toUpperCase();
  // "probot_" + macID => Örneğin: probot_AB12F3
  String ssid = "probot_" + macID;
  return ssid;
}

// ----------------------------
// mDNS başlatan fonksiyon (AP modunda her cihazda çalışmayabilir)
void setupMDNS(const char* hostName) {
  if (MDNS.begin(hostName)) {
    Serial.println("mDNS baslatildi. 'probot.local' olarak erisilebilir.");
  } else {
    Serial.println("mDNS baslatilamadi (AP modunda destegi sinirli olabilir).");
  }
}

// ----------------------------
// "/" isteğini karşılayan fonksiyon: index_html.h içeriğini gönderiyor
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

// ----------------------------
// "/joystick" isteğini karşılayan fonksiyon: x ve y parametresi geliyor
void handleJoystick() {
  if (server.hasArg("x") && server.hasArg("y")) {
    String xVal = server.arg("x");
    String yVal = server.arg("y");
    
    // Bu noktada xVal ve yVal değerlerini float’a çevirip
    // motor kontrolü, servo, vb. istediğiniz işlemleri yapabilirsiniz.
    // Şimdilik sadece Serial’a yazdırıyoruz.
    Serial.print("Joystick -> X: ");
    Serial.print(xVal);
    Serial.print(", Y: ");
    Serial.println(yVal);

    // LED'i basitçe yanıp sönsün diye örnek:
    // X > 0.5 ise LED'i aç, aksi halde kapat. (Aktif düşük olabileceğini unutmayın.)
    float xFloat = xVal.toFloat();
    if (xFloat > 0.5) {
      digitalWrite(LED_PIN, LOW);  // Aktif düşük LED
    } else {
      digitalWrite(LED_PIN, HIGH);
    }
  }

  // İstemciye yanıt
  server.send(200, "text/plain", "Joystick verisi alindi.");
}

// ----------------------------
void setup() {
  Serial.begin(115200);
  delay(100);

  // LED ayarı (NodeMCU dahili LED)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Başlangıçta kapalı

  // Access Point moduna geç
  WiFi.mode(WIFI_AP);
  
  // Benzersiz SSID üret
  String uniqueSSID = generateSSID();

  // AP oluştur
  WiFi.softAP(uniqueSSID.c_str(), AP_PASS);

  IPAddress myIP = WiFi.softAPIP();

  Serial.println("\n=== ProBot Joystick AP Olusturuldu ===");
  Serial.print("SSID: ");
  Serial.println(uniqueSSID);
  Serial.print("Sifre: ");
  Serial.println(AP_PASS);
  Serial.print("IP Adresi: ");
  Serial.println(myIP);

  // mDNS olayı (AP modunda her zaman garantili değil)
  setupMDNS("probot");

  // Web sunucusu yönlendirmeleri
  server.on("/", handleRoot);
  server.on("/joystick", handleJoystick);

  // Sunucuyu başlat
  server.begin();
  Serial.println("Web sunucusu baslatildi. Tarayicida IP adresine girerek erisebilirsiniz.");
}

// ----------------------------
void loop() {
  server.handleClient();
}
