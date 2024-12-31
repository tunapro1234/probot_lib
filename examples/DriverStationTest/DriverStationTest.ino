#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "index_html.h"

// Password for the AP
const char* AP_PASS = "robotpro1234";
ESP8266WebServer server(80);

// Robot state
String g_robotState = "init";
bool   g_enableAutonomous = false;
int    g_autoPeriod       = 30;
float  g_batteryVoltage   = 12.3;  // placeholder battery reading

// Generate unique SSID
String generateSSID() {
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
  macID.toUpperCase();
  return "probot_" + macID;
}

// Serve main HTML
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

// Joystick data endpoint
void handleUpdateController() {
  if (server.method() == HTTP_POST) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      server.send(400, "text/plain", "Bad JSON");
      return;
    }
    // doc["axes"], doc["buttons"] ...
    server.send(200, "text/plain", "OK");
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

// Robot control endpoint (Init → Start → Stop)
void handleRobotControl() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");  // "init", "start", "stop"
    bool autoFlag = (server.hasArg("auto") && server.arg("auto") == "1");
    int  autoLen  = server.hasArg("autoLen") ? server.arg("autoLen").toInt() : 30;

    // Basic server-side battery check
    if (g_batteryVoltage < 11.0 && (cmd == "init" || cmd == "start")) {
      server.send(200, "text/plain", "Battery too low, cannot proceed (server side).");
      return;
    }

    // Update the global state
    g_robotState       = cmd;
    g_enableAutonomous = autoFlag;
    g_autoPeriod       = autoLen;

    // Turn on LED if "start", otherwise turn off
    // (NodeMCU built-in LED is typically on D4 = GPIO2, active LOW)
    if (g_robotState == "start") {
      digitalWrite(LED_BUILTIN, LOW);   // LED ON
    } else {
      digitalWrite(LED_BUILTIN, HIGH);  // LED OFF
    }

    Serial.printf("[Robot Control] cmd=%s, auto=%d, autoLen=%d\n",
                  g_robotState.c_str(), g_enableAutonomous, g_autoPeriod);

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing 'cmd' arg");
  }
}

// Battery endpoint
void handleGetBattery() {
  // Replace with real ADC reading if desired
  server.send(200, "text/plain", String(g_batteryVoltage, 1));
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Set up LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn LED off initially (active low)

  // Set AP mode
  WiFi.mode(WIFI_AP);
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

  // Register routes
  server.on("/", handleRoot);
  server.on("/updateController", handleUpdateController);
  server.on("/robotControl", handleRobotControl);
  server.on("/getBattery", handleGetBattery);

  server.begin();
  Serial.println("Web server started. Visit http://192.168.4.1/");
}

void loop() {
  server.handleClient();
}
