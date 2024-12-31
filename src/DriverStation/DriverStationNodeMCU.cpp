#include "DriverStationNodeMCU.h"

// Constructor
DriverStationNodeMCU::DriverStationNodeMCU(const char* apPass)
    : AP_PASS(apPass), server(80),
      robotStatus(RobotStatus::INIT), enableAutonomous(false), autoPeriod(30), batteryVoltage(12.3), clientCount(0) {
    for (int i = 0; i < JOYSTICK_MAX_AXIS_COUNT; i++) {
        joystickAxes[i] = 0.0;
    }
    for (int i = 0; i < JOYSTICK_MAX_AXIS_COUNT; i++) {
        joystickButtons[i] = false;
    }
}

// Generate unique SSID
String DriverStationNodeMCU::generateSSID() {
    uint8_t mac[6];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
    macID.toUpperCase();
    return "probot_" + macID;
}

// Serve main HTML
void DriverStationNodeMCU::handleRoot() {
    server.send_P(200, "text/html", MAIN_page);
}

// // Joystick data endpoint 
// void DriverStationNodeMCU::handleUpdateController() {
//     if (server.method() == HTTP_POST) {
//         StaticJsonDocument<256> doc;
//         DeserializationError error = deserializeJson(doc, server.arg("plain"));
//         if (error) {
//             server.send(400, "text/plain", "Bad JSON");
//             return;
//         }

//         // Parse Joystick Axes
//         JsonArray axesArray = doc["axes"].as<JsonArray>();
//         this->joystickAxisCount = min((int)axesArray.size(), JOYSTICK_MAX_AXIS_COUNT);
//         for (int i = 0; i < this->joystickAxisCount; i++) {
//             joystickAxes[i] = axesArray[i].as<float>();
//         }

//         // Parse Joystick Buttons
//         JsonArray buttonsArray = doc["buttons"].as<JsonArray>();
//         this->joystickButtonCount = min((int)buttonsArray.size(), JOYSTICK_MAX_BUTTON_COUNT);
//         for (int i = 0; i < this->joystickButtonCount; i++) {
//             joystickButtons[i] = buttonsArray[i].as<bool>();
//         }

//         server.send(200, "text/plain", "OK");
//     } else {
//         server.send(405, "text/plain", "Method Not Allowed");
//     }
// }

// Joystick data endpoint 
void DriverStationNodeMCU::handleUpdateController() {
    if (server.method() == HTTP_POST) {
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        if (error) {
            server.send(400, "text/plain", "Bad JSON");
            Serial.println(F("JSON Parsing Error!"));
            return;
        }

        // Parse Joystick Axes
        if (doc.containsKey("axes")) {
            JsonArray axesArray = doc["axes"].as<JsonArray>();
            this->joystickAxisCount = min((int)axesArray.size(), JOYSTICK_MAX_AXIS_COUNT);
            for (int i = 0; i < this->joystickAxisCount; i++) {
                joystickAxes[i] = axesArray[i].as<float>();
            }
        }

        // Parse Joystick Buttons
        if (doc.containsKey("buttons")) {
            JsonArray buttonsArray = doc["buttons"].as<JsonArray>();
            this->joystickButtonCount = min((int)buttonsArray.size(), JOYSTICK_MAX_BUTTON_COUNT);
            for (int i = 0; i < this->joystickButtonCount; i++) {
                joystickButtons[i] = buttonsArray[i].as<bool>();
            }
        } else {
            Serial.println(F("Buttons array missing from payload!"));
        }

        // Serial.println("Joystick Data Updated:");
        // Serial.print("Axes: ");
        // for (int i = 0; i < joystickAxisCount; i++) {
        //     Serial.print(joystickAxes[i]);
        //     Serial.print(" ");
        // }
        // Serial.println();

        // Serial.print("Buttons: ");
        // for (int i = 0; i < joystickButtonCount; i++) {
            // Serial.print(joystickButtons[i] ? "Pressed " : "Released ");
        // }
        // Serial.println();

        server.send(200, "text/plain", "OK");
    } else {
        server.send(405, "text/plain", "Method Not Allowed");
    }
}


// Robot control endpoint
void DriverStationNodeMCU::handleRobotControl() {
    if (server.hasArg("cmd")) {
        String cmd = server.arg("cmd");
        bool autoFlag = (server.hasArg("auto") && server.arg("auto") == "1");
        int autoLen = server.hasArg("autoLen") ? server.arg("autoLen").toInt() : 30;

        if (batteryVoltage < 11.0 && (cmd == "init" || cmd == "start")) {
            server.send(200, "text/plain", "Battery too low, cannot proceed (server side).");
            return;
        }

        if (cmd == "init") robotStatus = RobotStatus::INIT;
        else if (cmd == "start") robotStatus = RobotStatus::START;
        else if (cmd == "stop") robotStatus = RobotStatus::STOP;

        enableAutonomous = autoFlag;
        autoPeriod = autoLen;

        if (robotStatus == RobotStatus::START) {
            digitalWrite(LED_BUILTIN, LOW); // LED ON
        } else {
            digitalWrite(LED_BUILTIN, HIGH); // LED OFF
        }

        Serial.print("[Robot Control] Status=");
        Serial.println(robotStatusToString(robotStatus));

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing 'cmd' arg");
    }
}

// Battery endpoint
void DriverStationNodeMCU::handleGetBattery() {
    server.send(200, "text/plain", String(batteryVoltage, 1));
}

// Begin the server
void DriverStationNodeMCU::begin() {
    Serial.begin(115200);
    delay(100);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    WiFi.mode(WIFI_AP);
    String uniqueSSID = generateSSID();
    WiFi.softAP(uniqueSSID.c_str(), AP_PASS);

    IPAddress myIP = WiFi.softAPIP();
    Serial.println("AP IP: " + myIP.toString());

    server.on("/", std::bind(&DriverStationNodeMCU::handleRoot, this));
    server.on("/updateController", std::bind(&DriverStationNodeMCU::handleUpdateController, this));
    server.on("/robotControl", std::bind(&DriverStationNodeMCU::handleRobotControl, this));
    server.on("/getBattery", std::bind(&DriverStationNodeMCU::handleGetBattery, this));

    server.begin();
    Serial.println("Server started.");
}

// Handle incoming clients
void DriverStationNodeMCU::handleClient() {
    server.handleClient();
    clientCount = WiFi.softAPgetStationNum();
}

// --- Getter Methods ---
float DriverStationNodeMCU::getJoystickAxis(int axis) {
    if (axis < 0 || axis >= JOYSTICK_MAX_AXIS_COUNT) {
        Serial.println(F("Invalid Joystick Axis"));
        return 0.0;
    }
    return joystickAxes[axis];
}

bool DriverStationNodeMCU::getJoystickButton(int button) {
    if (button < 0 || button >= JOYSTICK_MAX_BUTTON_COUNT) {
        Serial.println(F("Invalid Joystick Button"));
        return 0.0;
    }
    return joystickButtons[button];
}

int DriverStationNodeMCU::getJoystickAxisCount() {
    return joystickAxisCount;
}

int DriverStationNodeMCU::getJoystickButtonCount() {
    return joystickButtonCount;
}


RobotStatus DriverStationNodeMCU::getRobotStatus() {
    return robotStatus;
}

int DriverStationNodeMCU::getClientCount() {
    return clientCount;
}

// --- Setter Methods ---
void DriverStationNodeMCU::setBatteryVoltage(float voltage) {
    batteryVoltage = voltage;
}

// --- Helper ---
String DriverStationNodeMCU::robotStatusToString(RobotStatus status) {
    switch (status) {
        case RobotStatus::INIT: return "INIT";
        case RobotStatus::START: return "START";
        case RobotStatus::STOP: return "STOP";
        default: return "UNKNOWN";
    }
}
