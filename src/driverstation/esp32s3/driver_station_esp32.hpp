#pragma once
#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <probot/robot/state.hpp>
#include <probot/io/gamepad.hpp>
#include <probot/telemetry/telemetry.hpp>
#include "index_html.h"
#include "ws_joystick.hpp"

#ifndef PROBOT_WIFI_AP_PASSWORD
#error "Driver station AP password not provided. Define PROBOT_WIFI_AP_PASSWORD (>=8 chars) before including probot.h."
#endif
static_assert(sizeof(PROBOT_WIFI_AP_PASSWORD) - 1 >= 8, "PROBOT_WIFI_AP_PASSWORD must be at least 8 characters.");

#ifndef PROBOT_WIFI_AP_SSID
  #define PROBOT_WIFI_AP_SSID "Probot"
  #ifndef PROBOT_WIFI_AP_SSID_MAC_SUFFIX
    #define PROBOT_WIFI_AP_SSID_MAC_SUFFIX
  #endif
  #warning "PROBOT_WIFI_AP_SSID not defined. Using auto-generated SSID (Probot-XXXXXX). Define a custom SSID for better identification."
#endif
static_assert(sizeof(PROBOT_WIFI_AP_SSID) - 1 >= 1, "PROBOT_WIFI_AP_SSID must be at least 1 character.");
#ifdef PROBOT_WIFI_AP_SSID_MAC_SUFFIX
static_assert(sizeof(PROBOT_WIFI_AP_SSID) - 1 <= 25, "PROBOT_WIFI_AP_SSID must be 25 characters or fewer when MAC suffix is enabled.");
#else
static_assert(sizeof(PROBOT_WIFI_AP_SSID) - 1 <= 32, "PROBOT_WIFI_AP_SSID must be 32 characters or fewer.");
#endif

#ifndef PROBOT_WIFI_AP_CHANNEL
#error "WiFi AP channel not provided. Define PROBOT_WIFI_AP_CHANNEL (1-13) before including probot.h."
#endif
static_assert(PROBOT_WIFI_AP_CHANNEL >= 1 && PROBOT_WIFI_AP_CHANNEL <= 13,
              "PROBOT_WIFI_AP_CHANNEL must be between 1 and 13.");

namespace probot::driverstation::esp32 {
  class DriverStation {
  public:
    DriverStation(robot::StateService& rs, io::GamepadService& gs)
    : _rs(rs), _gs(gs), _ws(gs), _server(80) {}

    void begin(){
      const char* pw = PROBOT_WIFI_AP_PASSWORD;
      String ssid = String(PROBOT_WIFI_AP_SSID);
#ifdef PROBOT_WIFI_AP_SSID_MAC_SUFFIX
      char suffix[8];
      snprintf(suffix, sizeof(suffix), "-%06X", (unsigned int)(ESP.getEfuseMac() & 0xFFFFFF));
      ssid += suffix;
#endif
      ap_ssid_ = ssid;
      WiFi.mode(WIFI_AP);
      wifi_country_t country = { .cc = "TR", .schan = 1, .nchan = 13, .policy = WIFI_COUNTRY_POLICY_MANUAL };
      esp_wifi_set_country(&country);
      WiFi.softAP(ssid.c_str(), pw, PROBOT_WIFI_AP_CHANNEL);
      esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);
      esp_wifi_set_ps(WIFI_PS_NONE);
      WiFi.setTxPower(WIFI_POWER_19_5dBm);

      Serial.println("[DS   ] ========================================");
      Serial.print("[DS   ] WiFi SSID: ");
      Serial.println(ssid);
      Serial.print("[DS   ] Password:  ");
      Serial.println("********");
      Serial.print("[DS   ] Channel:   ");
      Serial.println(PROBOT_WIFI_AP_CHANNEL);
      Serial.print("[DS   ] IP Address: ");
      Serial.println(WiFi.softAPIP());
      Serial.println("[DS   ] ========================================");

      _server.on("/", HTTP_GET, [this](){ if (!enforceOwner()) return; handleRoot(); });
      _server.on("/updateController", HTTP_POST, [this](){ if (!enforceOwner()) return; handleUpdateController(); });
      _server.on("/robotControl", HTTP_GET, [this](){ if (!enforceOwner()) return; handleRobotControl(); });
      _server.on("/getState", HTTP_GET, [this](){ if (!enforceOwner()) return; handleGetState(); });
      _server.on("/getBattery", HTTP_GET, [this](){ handleGetBattery(); });
      _server.on("/telemetry", HTTP_GET, [this](){ if (!enforceOwner()) return; handleTelemetry(); });
      _server.on("/health", HTTP_GET, [this](){ if (!enforceOwner()) return; handleHealth(); });
      _server.on("/info", HTTP_GET, [this](){ if (!enforceOwner()) return; handleInfo(); });
      _server.begin();
      _ws.begin(81);
    }

    void handleClient(){
      _server.handleClient();
      expireOwnerIfIdle();
    }

  private:
    bool enforceOwner(){
      uint32_t now = millis();
      IPAddress ip = _server.client().remoteIP();
      if (_owner_set && _owner_timeout_ms > 0 &&
          (uint32_t)(now - _owner_last_ms) > _owner_timeout_ms){
        releaseOwner(now);
      }
      if (!_owner_set){
        _owner = ip;
        _owner_set = true;
        _owner_last_ms = now;
        _rs.setClientCount(now, 1);
        return true;
      }
      if (ip == _owner){
        _owner_last_ms = now;
        return true;
      }
      _server.send(403, "text/plain", "Another client is already connected.");
      return false;
    }

    void expireOwnerIfIdle(){
      if (!_owner_set || _owner_timeout_ms == 0) return;
      uint32_t now = millis();
      if ((uint32_t)(now - _owner_last_ms) > _owner_timeout_ms){
        releaseOwner(now);
      }
    }

    void releaseOwner(uint32_t now_ms){
      _owner_set = false;
      _owner = IPAddress();
      _rs.setClientCount(now_ms, 0);
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

    String generateSSID(){ uint64_t mac=ESP.getEfuseMac(); char ssid[32]; snprintf(ssid, sizeof(ssid), "Probot-%06X", (unsigned int)(mac & 0xFFFFFF)); return String(ssid); }

    void handleRoot(){
      _server.setContentLength(strlen_P(MAIN_page));
      _server.send_P(200, "text/html", MAIN_page);
    }

    void handleGetBattery(){
      auto s = _rs.read();
      char buf[16]; dtostrf(s.batteryVoltage, 0, 1, buf);
      _server.send(200, "text/plain", buf);
    }

    void handleGetState(){
      auto s = _rs.read();
      uint32_t now_ms = millis();
      uint32_t remaining_ms = 0;
      if (s.phase == probot::robot::Phase::AUTONOMOUS && s.autonomousEnabled &&
          s.autoStartMs != 0 && s.autoPeriodSeconds > 0) {
        uint32_t total_ms = static_cast<uint32_t>(s.autoPeriodSeconds) * 1000u;
        uint32_t elapsed = now_ms - s.autoStartMs;
        remaining_ms = (elapsed >= total_ms) ? 0u : (total_ms - elapsed);
      }
      char buf[128];
      snprintf(buf, sizeof(buf),
               "{\"phase\":%u,\"autonomousEnabled\":%s,\"autoPeriodSeconds\":%d,\"autoRemainingMs\":%u}",
               static_cast<unsigned>(s.phase),
               s.autonomousEnabled ? "true" : "false",
               (int)s.autoPeriodSeconds,
               (unsigned)remaining_ms);
      _server.send(200, "application/json", buf);
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
      } else if (cmd == "cancelAuto"){
        _rs.setAutonomous(millis(), false);
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

    void handleTelemetry(){
      _server.send(200, "text/plain", probot::telemetry::getBuffer());
    }

    void handleHealth(){
      int8_t rssi = -100;
      wifi_sta_list_t sta_list;
      if (esp_wifi_ap_get_sta_list(&sta_list) == ESP_OK && sta_list.num > 0) {
        rssi = sta_list.sta[0].rssi;
      }
      auto s = _rs.read();
      char buf[128];
      snprintf(buf, sizeof(buf),
        "{\"rssi\":%d,\"up\":%lu,\"heap\":%lu,\"dm\":%s}",
        (int)rssi,
        (unsigned long)millis(),
        (unsigned long)ESP.getFreeHeap(),
        s.deadlineMiss ? "true" : "false");
      _server.send(200, "application/json", buf);
    }

    void handleInfo(){
      char buf[256];
      snprintf(buf, sizeof(buf),
        "{\"ssid\":\"%s\",\"ch\":%d,\"pw\":\"%s\",\"ip\":\"%s\"}",
        ap_ssid_.c_str(),
        PROBOT_WIFI_AP_CHANNEL,
        PROBOT_WIFI_AP_PASSWORD,
        WiFi.softAPIP().toString().c_str());
      _server.send(200, "application/json", buf);
    }

    robot::StateService& _rs;
    io::GamepadService&  _gs;
    WsJoystick           _ws;
    WebServer            _server;
    bool                 _owner_set=false;
    IPAddress            _owner;
    uint32_t             _owner_last_ms=0;
    uint32_t             _owner_timeout_ms=5000;
    String               ap_ssid_;
  };
}
#endif // ESP32 
