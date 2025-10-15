#pragma once
#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <probot/robot/state.hpp>
#include <probot/io/gamepad.hpp>
#include "index_html.h"

#if defined(PROBOT_WITH_DS) && !defined(PROBOT_WIFI_AP_PASSWORD)
#warning "DriverStation AP password not provided. Call PROBOT_SET_DRIVER_STATION_PASSWORD(\"yourpassword\") in your sketch (or define PROBOT_WIFI_AP_PASSWORD). If you already set it via the macro, you can ignore this warning."
#endif
#ifdef PROBOT_WIFI_AP_PASSWORD
static_assert(sizeof(PROBOT_WIFI_AP_PASSWORD) - 1 >= 8, "PROBOT_WIFI_AP_PASSWORD must be at least 8 characters.");
#endif

namespace probot::platform::esp32 {
  class DriverStation {
  public:
    DriverStation(robot::StateService& rs, io::GamepadService& gs, const char* apPass = nullptr)
    : _rs(rs), _gs(gs), _server(80), _apPass(apPass) {}

    void begin(){
      const char* pw = nullptr;
#ifdef PROBOT_WIFI_AP_PASSWORD
      pw = PROBOT_WIFI_AP_PASSWORD;
#endif
      if (!pw) pw = _apPass;
      if (!pw || strlen(pw) < 8){
        Serial.println("[DS   ] AP password not set or too short; DriverStation AP disabled");
        return;
      }
      String ssid = generateSSID();
      WiFi.mode(WIFI_AP);
      WiFi.softAP(ssid.c_str(), pw);

      Serial.println("[DS   ] ========================================");
      Serial.print("[DS   ] WiFi SSID: ");
      Serial.println(ssid);
      Serial.print("[DS   ] Password:  ");
      Serial.println(pw);
      Serial.print("[DS   ] IP Address: ");
      Serial.println(WiFi.softAPIP());
      Serial.println("[DS   ] ========================================");

      _server.on("/", HTTP_GET, [this](){ if (!enforceOwner()) return; handleRoot(); });
      _server.on("/updateController", HTTP_POST, [this](){ if (!enforceOwner()) return; handleUpdateController(); });
      _server.on("/robotControl", HTTP_GET, [this](){ if (!enforceOwner()) return; handleRobotControl(); });
      _server.on("/getBattery", HTTP_GET, [this](){ handleGetBattery(); });
      _server.begin();
    }

    void handleClient(){ _server.handleClient(); }

  private:
    bool enforceOwner(){
      IPAddress ip = _server.client().remoteIP();
      if (!_owner_set){ _owner = ip; _owner_set = true; _rs.setClientCount(millis(), 1); return true; }
      if (ip == _owner) return true;
      _server.send(403, "text/plain", "Another client is already connected.");
      return false;
    }

    static bool parseFloatArray(const String& body, const char* key, float* out, uint32_t maxCount, uint32_t& written){
      written = 0; int keyPos = body.indexOf(key); if (keyPos < 0) return false;
      int lb = body.indexOf('[', keyPos); if (lb < 0) return false; int rb = body.indexOf(']', lb); if (rb < 0) return false;
      const char* p = body.c_str() + lb + 1; const char* end = body.c_str() + rb;
      while (p < end && written < maxCount){
        while (p < end && (*p==' '||*p=='\n'||*p=='\r'||*p=='\t'||*p==',')) p++;
        if (p>=end) break;
        char* q = nullptr; float v = strtof(p, &q);
        if (q==p){ break; }
        out[written++] = v; p = q;
      }
      return true;
    }

    static bool parseBoolArray(const String& body, const char* key, bool* out, uint32_t maxCount, uint32_t& written){
      written = 0; int keyPos = body.indexOf(key); if (keyPos < 0) return false;
      int lb = body.indexOf('[', keyPos); if (lb < 0) return false; int rb = body.indexOf(']', lb); if (rb < 0) return false;
      const char* p = body.c_str() + lb + 1; const char* end = body.c_str() + rb;
      while (p < end && written < maxCount){
        while (p < end && (*p==' '||*p=='\n'||*p=='\r'||*p=='\t'||*p==',')) p++;
        if (p>=end) break;
        if ((end - p) >= 4 && strncmp(p, "true", 4)==0){ out[written++]=true; p+=4; }
        else if ((end - p) >= 5 && strncmp(p, "false", 5)==0){ out[written++]=false; p+=5; }
        else { break; }
      }
      return true;
    }

    String generateSSID(){ uint64_t mac=ESP.getEfuseMac(); char ssid[32]; snprintf(ssid, sizeof(ssid), "ProBot-%06X", (unsigned int)(mac & 0xFFFFFF)); return String(ssid); }

    void handleRoot(){
      _server.setContentLength(strlen_P(MAIN_page));
      _server.send_P(200, "text/html", MAIN_page);
    }

    void handleGetBattery(){
      auto s = _rs.read();
      char buf[16]; dtostrf(s.batteryVoltage, 0, 1, buf);
      _server.send(200, "text/plain", buf);
    }

    void handleRobotControl(){
      String cmd = _server.arg("cmd");
      bool enAuto = _server.arg("auto").toInt() != 0;
      int autoLen = _server.arg("autoLen").toInt();
      if (cmd == "init"){
        _rs.setStatus(millis(), robot::Status::INIT);
        _rs.setDeadlineMiss(millis(), false);
      } else if (cmd == "start"){
        _rs.setAutonomous(millis(), enAuto);
        if (autoLen > 0) _rs.setAutoPeriodSeconds(millis(), autoLen);
        _rs.setStatus(millis(), robot::Status::START);
      } else if (cmd == "stop"){
        _rs.setStatus(millis(), robot::Status::STOP);
      }
      _server.send(200, "text/plain", "OK");
    }

    void handleUpdateController(){
      if (_server.method() != HTTP_POST){ _server.send(405, "text/plain", "Method Not Allowed"); return; }
      String body = _server.arg("plain");
      float axes[20]; bool buttons[20]; uint32_t nA=0, nB=0;
      parseFloatArray(body, "axes", axes, 20, nA);
      parseBoolArray(body, "buttons", buttons, 20, nB);
      _gs.write(millis(), axes, nA, buttons, nB);
      _server.send(200, "text/plain", "OK");
    }

    robot::StateService& _rs;
    io::GamepadService&  _gs;
    WebServer            _server;
    const char*          _apPass;
    bool                 _owner_set=false;
    IPAddress            _owner;
  };
}
#endif // ESP32 