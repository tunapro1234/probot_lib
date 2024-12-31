#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "index_html.h"

// Serial communication with ESP8266-01
SoftwareSerial espSerial(10, 11); // RX (Arduino Pin 10) → TX (ESP), TX (Arduino Pin 11) → RX (ESP)

// Robot state
String g_robotState = "init";
bool   g_enableAutonomous = false;
int    g_autoPeriod = 30;
float  g_batteryVoltage = 12.3;  // placeholder battery reading

// ESP8266 Configuration
const char* SSID = "probot_arduino";
const char* PASSWORD = "robotpro1234";
const int SERVER_PORT = 80;

// --- Helper Functions ---
void sendCommand(const String &cmd, const String &ack, unsigned long timeout = 2000) {
  espSerial.println(cmd);
  unsigned long start = millis();
  String response = "";
  while (millis() - start < timeout) {
    if (espSerial.available()) {
      response += char(espSerial.read());
      if (response.indexOf(ack) != -1) {
        Serial.println("ESP8266 Response: " + response);
        return;
      }
    }
  }
  Serial.println("ESP8266 Command Timeout: " + cmd);
}

// --- Setup WiFi on ESP8266 ---
void setupWiFi() {
  sendCommand("AT", "OK");
  sendCommand("AT+CWMODE=2", "OK");  // Set as Access Point
  sendCommand("AT+CWSAP=\"" + String(SSID) + "\",\"" + String(PASSWORD) + "\",5,3", "OK");
  sendCommand("AT+CIPMUX=1", "OK");  // Enable multiple connections
  sendCommand("AT+CIPSERVER=1," + String(SERVER_PORT), "OK");  // Start server
}

// --- Serve HTML Page ---
void serveHTML() {
  String html = MAIN_page; // From index_html.h
  int length = html.length();
  
  sendCommand("AT+CIPSEND=0," + String(length), ">", 3000);
  espSerial.print(html);
  delay(100);
  Serial.println("HTML page sent to client.");
}

// --- Handle Joystick Data (JSON) ---
void handleUpdateController(const String& jsonPayload) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, jsonPayload);

  if (error) {
    Serial.println("Failed to parse JSON in /updateController");
    return;
  }

  float axisX = doc["axes"][0];
  float axisY = doc["axes"][1];
  bool button1 = doc["buttons"][0];

  Serial.print("Joystick X: "); Serial.println(axisX);
  Serial.print("Joystick Y: "); Serial.println(axisY);
  Serial.print("Button 1: "); Serial.println(button1);

  // Respond OK
  sendCommand("AT+CIPSEND=0,2", ">", 1000);
  espSerial.println("OK");
}

// --- Handle Robot Control ---
void handleRobotControl(const String& query) {
  if (query.indexOf("cmd=") != -1) {
    String cmd = query.substring(query.indexOf("cmd=") + 4, query.indexOf("&", query.indexOf("cmd=")));
    bool autoFlag = (query.indexOf("auto=1") != -1);
    int autoLen = query.indexOf("autoLen=") != -1 ? query.substring(query.indexOf("autoLen=") + 8).toInt() : 30;

    if (g_batteryVoltage < 11.0 && (cmd == "init" || cmd == "start")) {
      sendCommand("AT+CIPSEND=0,50", ">", 1000);
      espSerial.println("Battery too low, cannot proceed (server side).");
      return;
    }

    g_robotState = cmd;
    g_enableAutonomous = autoFlag;
    g_autoPeriod = autoLen;

    if (g_robotState == "start") {
      digitalWrite(LED_BUILTIN, LOW);   // LED ON
    } else {
      digitalWrite(LED_BUILTIN, HIGH);  // LED OFF
    }

    Serial.printf("[Robot Control] cmd=%s, auto=%d, autoLen=%d\n",
                  g_robotState.c_str(), g_enableAutonomous, g_autoPeriod);

    sendCommand("AT+CIPSEND=0,2", ">", 1000);
    espSerial.println("OK");
  } else {
    sendCommand("AT+CIPSEND=0,18", ">", 1000);
    espSerial.println("Missing 'cmd' arg");
  }
}

// --- Handle Battery Endpoint ---
void handleGetBattery() {
  String batteryStr = String(g_batteryVoltage, 1);
  sendCommand("AT+CIPSEND=0," + String(batteryStr.length()), ">", 1000);
  espSerial.println(batteryStr);
  Serial.println("Battery Voltage Sent: " + batteryStr);
}

// --- Handle HTTP Requests ---
void handleClient(String request) {
  if (request.indexOf("/") != -1 && request.indexOf("favicon") == -1) {
    serveHTML();
  } else if (request.indexOf("/updateController") != -1) {
    String jsonPayload = request.substring(request.indexOf("{"));
    handleUpdateController(jsonPayload);
  } else if (request.indexOf("/robotControl") != -1) {
    handleRobotControl(request);
  } else if (request.indexOf("/getBattery") != -1) {
    handleGetBattery();
  } else {
    sendCommand("AT+CIPSEND=0,10", ">", 1000);
    espSerial.println("404 Not Found");
  }
}

// --- Main Arduino Setup ---
void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED off initially

  Serial.println("Setting up ESP8266...");
  setupWiFi();
  Serial.println("ESP8266 Web Server Ready!");
}

// --- Main Loop ---
void loop() {
  if (espSerial.available()) {
    String data = "";
    while (espSerial.available()) {
      data += char(espSerial.read());
      delay(10);
    }

    Serial.println("ESP Received: " + data);

    if (data.indexOf("+IPD") != -1) {
      int index = data.indexOf("GET");
      if (index != -1) {
        String request = data.substring(index, data.indexOf(" HTTP"));
        handleClient(request);
      }
    }
  }
}
