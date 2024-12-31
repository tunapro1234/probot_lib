#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>  // Make sure to install ArduinoJson library via Library Manager
#include "index_html.h"   // Our HTML page

// The AP password
const char* AP_PASS = "robotpro1234";

// Create a web server on port 80
ESP8266WebServer server(80);

// Generate a unique SSID by appending the last 3 bytes of the MAC address
String generateSSID() {
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);

  // Convert the last 3 bytes to hex
  String macID = String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
  macID.toUpperCase();

  // e.g. "probot_AB12F3"
  return "probot_" + macID;
}

// Serve the main page (from index_html.h)
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

// Parse JSON with all button states + axes and print to Serial
void handleControllerUpdate() {
  // We'll read the POST body which contains JSON data
  if (server.method() == HTTP_POST) {
    // The JSON object might be relatively small, but we need a buffer:
    // e.g. 32 for overhead + up to 64 for axes + 32 for buttons ...
    // We'll try 256. Adjust as needed if you have more data.
    const size_t capacity = 256;
    StaticJsonDocument<capacity> doc;

    // Parse the incoming JSON
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      server.send(400, "text/plain", "Bad JSON");
      return;
    }

    // "axes" is an array, "buttons" is an array of booleans
    if (doc.containsKey("axes") && doc.containsKey("buttons")) {
      JsonArray axes = doc["axes"];
      JsonArray buttons = doc["buttons"];

      Serial.println("\n--- Controller Update ---");

      // Print Axes
      Serial.print("Axes: ");
      for (size_t i = 0; i < axes.size(); i++) {
        float val = axes[i];
        Serial.print(val, 2); // 2 decimal places
        if (i < axes.size() - 1) Serial.print(", ");
      }
      Serial.println();

      // Print Buttons
      Serial.print("Buttons: ");
      for (size_t i = 0; i < buttons.size(); i++) {
        bool pressed = buttons[i];
        Serial.print(pressed ? "1" : "0");
        if (i < buttons.size() - 1) Serial.print(", ");
      }
      Serial.println("\n------------------------");
    }
    else {
      Serial.println("Invalid JSON structure");
    }

    server.send(200, "text/plain", "OK");
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Configure Wi-Fi in AP mode
  WiFi.mode(WIFI_AP);

  // Generate a unique SSID
  String uniqueSSID = generateSSID();

  // Create the AP
  WiFi.softAP(uniqueSSID.c_str(), AP_PASS);

  // Print info
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("\n=== ProBot Gamepad AP ===");
  Serial.print("SSID: ");
  Serial.println(uniqueSSID);
  Serial.print("Password: ");
  Serial.println(AP_PASS);
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Set up web endpoints
  server.on("/", handleRoot);
  server.on("/updateController", handleControllerUpdate);

  // Start server
  server.begin();
  Serial.println("Web server started. Open browser at http://192.168.4.1/");
}

void loop() {
  server.handleClient();
}
