#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>    // Required for JSON parsing
#include "index_html.h"     // Our multi-page HTML

// Password for the AP
const char* AP_PASS = "robotpro1234";

// Web server on port 80
ESP8266WebServer server(80);

/****************************************************
 * Generate a unique SSID by appending last 3 bytes of MAC
 ****************************************************/
String generateSSID() {
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);

  // Convert last 3 bytes to hex
  String macID = String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
  macID.toUpperCase();

  // e.g. "probot_AB12F3"
  return "probot_" + macID;
}

/****************************************************
 * Serve the main HTML page (sidebar + multiple sections)
 ****************************************************/
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

/****************************************************
 * Receive JSON from Joystick Test page
 * (axes + buttons), but do nothing with it here.
 ****************************************************/
void handleUpdateController() {
  if (server.method() == HTTP_POST) {
    // Parse JSON (axes/buttons)
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      server.send(400, "text/plain", "Bad JSON");
      return;
    }

    // We do not do anything with doc["axes"] or doc["buttons"] here.
    server.send(200, "text/plain", "OK");
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

/****************************************************
 * Handle robot control commands (Init, Start, Stop)
 * We just print them here; do your logic as needed.
 ****************************************************/
void handleRobotControl() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    // Do your actual logic here
    Serial.print("[Robot Control] Command received: ");
    Serial.println(cmd);

    // Acknowledge
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing 'cmd' argument");
  }
}

/****************************************************
 * Return a placeholder battery voltage (12.0).
 * Replace with actual reading logic if you have it.
 ****************************************************/
void handleGetBattery() {
  server.send(200, "text/plain", "12.0"); // e.g., 12.0 V
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Set AP mode
  WiFi.mode(WIFI_AP);

  // Create a unique SSID
  String uniqueSSID = generateSSID();
  WiFi.softAP(uniqueSSID.c_str(), AP_PASS);

  IPAddress myIP = WiFi.softAPIP();
  Serial.println("\n=== ProBot Multi-Page AP ===");
  Serial.print("SSID: ");
  Serial.println(uniqueSSID);
  Serial.print("Password: ");
  Serial.println(AP_PASS);
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Routes
  server.on("/", handleRoot);
  server.on("/updateController", handleUpdateController);
  server.on("/robotControl", handleRobotControl);
  server.on("/getBattery", handleGetBattery);

  // Start server
  server.begin();
  Serial.println("Web server started. Browse to http://192.168.4.1/");
}

void loop() {
  server.handleClient();
}
