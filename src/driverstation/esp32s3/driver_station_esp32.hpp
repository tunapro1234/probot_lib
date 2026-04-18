#pragma once
#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include <lwip/sockets.h>
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
    : _rs(rs), _gs(gs), _ws(gs) {}

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

      // Start ESP-IDF httpd (single server, port 80)
      httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
      cfg.server_port    = 80;
      cfg.ctrl_port      = 32768;
      cfg.stack_size     = 8192;
      cfg.max_uri_handlers = 12;
      cfg.lru_purge_enable = true;

      if (httpd_start(&_server, &cfg) != ESP_OK) {
        Serial.println("[DS   ] Failed to start HTTP server");
        return;
      }

      // Register HTTP routes
      registerUri("/",                HTTP_GET,  handleRoot);
      registerUri("/updateController", HTTP_POST, handleUpdateController);
      registerUri("/robotControl",     HTTP_GET,  handleRobotControl);
      registerUri("/getState",         HTTP_GET,  handleGetState);
      registerUri("/getBattery",       HTTP_GET,  handleGetBattery);
      registerUri("/telemetry",        HTTP_GET,  handleTelemetry);
      registerUri("/health",           HTTP_GET,  handleHealth);
      registerUri("/info",             HTTP_GET,  handleInfo);

      // Attach WebSocket handler
      _ws.attach(_server);

      Serial.println("[DS   ] HTTP server started on port 80");
    }

    void expireOwnerIfIdle(){
      if (!_owner_set || _owner_timeout_ms == 0) return;
      uint32_t now = millis();
      if ((uint32_t)(now - _owner_last_ms) > _owner_timeout_ms){
        uint32_t idle = now - _owner_last_ms;
        Serial.printf("[DS   ] Owner expired: %s idle %lu ms\n",
                      _owner_str, (unsigned long)idle);
        releaseOwner(now);
      }
    }

    void forceDisconnect(uint32_t now_ms){
      Serial.println("[DS   ] Force disconnect: connection timeout");
      if (_owner_set) releaseOwner(now_ms);
      _ws.closeAll();
      Serial.println("[DS   ] Owner released, WS connections closed");
    }

  private:
    // ── Helpers ──

    void registerUri(const char* uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t*)) {
      httpd_uri_t u = {
        .uri      = uri,
        .method   = method,
        .handler  = handler,
        .user_ctx = this,
      };
      httpd_register_uri_handler(_server, &u);
    }

    static DriverStation* self(httpd_req_t* req) {
      return static_cast<DriverStation*>(req->user_ctx);
    }

    // ── Client IP extraction ──

    static bool getClientIP(httpd_req_t* req, char* out, size_t out_len) {
      int sockfd = httpd_req_to_sockfd(req);
      struct sockaddr_in6 addr;
      socklen_t addr_len = sizeof(addr);
      if (getpeername(sockfd, (struct sockaddr*)&addr, &addr_len) != 0) {
        out[0] = '\0';
        return false;
      }
      // IPv4-mapped IPv6: extract the IPv4 part
      if (addr.sin6_family == AF_INET6 && IN6_IS_ADDR_V4MAPPED(&addr.sin6_addr)) {
        struct in_addr v4;
        memcpy(&v4, &addr.sin6_addr.s6_addr[12], 4);
        inet_ntop(AF_INET, &v4, out, out_len);
      } else if (addr.sin6_family == AF_INET) {
        inet_ntop(AF_INET, &((struct sockaddr_in*)&addr)->sin_addr, out, out_len);
      } else {
        inet_ntop(AF_INET6, &addr.sin6_addr, out, out_len);
      }
      return true;
    }

    // ── Owner enforcement ──

    bool enforceOwner(httpd_req_t* req) {
      uint32_t now = millis();
      char ip[48];
      if (!getClientIP(req, ip, sizeof(ip))) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Cannot determine client IP");
        return false;
      }

      // Check timeout on current owner
      if (_owner_set && _owner_timeout_ms > 0 &&
          (uint32_t)(now - _owner_last_ms) > _owner_timeout_ms) {
        uint32_t idle = now - _owner_last_ms;
        Serial.printf("[DS   ] Owner timeout: %s idle %lu ms (limit %lu ms)\n",
                      _owner_str, (unsigned long)idle, (unsigned long)_owner_timeout_ms);
        releaseOwner(now);
      }

      if (!_owner_set) {
        strncpy(_owner_str, ip, sizeof(_owner_str) - 1);
        _owner_str[sizeof(_owner_str) - 1] = '\0';
        _owner_set = true;
        _owner_last_ms = now;
        __atomic_store_n(&probot::robot::g_ds_last_activity_ms, now, __ATOMIC_SEQ_CST);
        _rs.setClientCount(now, 1);
        Serial.printf("[DS   ] Owner acquired: %s\n", _owner_str);
        return true;
      }

      if (strcmp(ip, _owner_str) == 0) {
        _owner_last_ms = now;
        __atomic_store_n(&probot::robot::g_ds_last_activity_ms, now, __ATOMIC_SEQ_CST);
        return true;
      }

      Serial.printf("[DS   ] Rejected %s (owner: %s)\n", ip, _owner_str);
      httpd_resp_set_status(req, "403 Forbidden");
      httpd_resp_send(req, "Another client is already connected.", HTTPD_RESP_USE_STRLEN);
      return false;
    }

    void releaseOwner(uint32_t now_ms) {
      Serial.printf("[DS   ] Owner released: %s\n", _owner_str);
      _owner_set = false;
      _owner_str[0] = '\0';
      // Zero the gamepad state so user code reading axes/buttons does
      // not see stale values (last-command runaway when link dies).
      _gs.write(now_ms, nullptr, 0, nullptr, 0);
      _rs.setClientCount(now_ms, 0);
    }

    // ── Parsers (unchanged) ──

    static bool parseFloatArray(const char* body, const char* key, float* out, uint32_t maxCount, uint32_t& written) {
      written = 0;
      const char* keyPos = strstr(body, key);
      if (!keyPos) return false;
      const char* lb = strchr(keyPos, '[');
      if (!lb) return false;
      const char* rb = strchr(lb, ']');
      if (!rb) return false;
      const char* p = lb + 1;
      while (p < rb && written < maxCount) {
        while (p < rb && (*p==' '||*p=='\n'||*p=='\r'||*p=='\t'||*p==',')) p++;
        if (p >= rb) break;
        char* q = nullptr;
        float v = strtof(p, &q);
        if (q == p) break;
        out[written++] = v;
        p = q;
      }
      return true;
    }

    static bool parseBoolArray(const char* body, const char* key, bool* out, uint32_t maxCount, uint32_t& written) {
      written = 0;
      const char* keyPos = strstr(body, key);
      if (!keyPos) return false;
      const char* lb = strchr(keyPos, '[');
      if (!lb) return false;
      const char* rb = strchr(lb, ']');
      if (!rb) return false;
      const char* p = lb + 1;
      while (p < rb && written < maxCount) {
        while (p < rb && (*p==' '||*p=='\n'||*p=='\r'||*p=='\t'||*p==',')) p++;
        if (p >= rb) break;
        if ((rb - p) >= 4 && strncmp(p, "true", 4) == 0) { out[written++] = true; p += 4; }
        else if ((rb - p) >= 5 && strncmp(p, "false", 5) == 0) { out[written++] = false; p += 5; }
        else break;
      }
      return true;
    }

    // ── Route handlers ──

    static esp_err_t handleRoot(httpd_req_t* req) {
      auto* ds = self(req);
      if (!ds->enforceOwner(req)) return ESP_OK;

      httpd_resp_set_type(req, "text/html");
      const char* ptr = MAIN_page;
      size_t remaining = strlen_P(MAIN_page);
      while (remaining > 0) {
        size_t chunk = (remaining > 2048) ? 2048 : remaining;
        if (httpd_resp_send_chunk(req, ptr, chunk) != ESP_OK) {
          httpd_resp_send_chunk(req, NULL, 0);
          return ESP_FAIL;
        }
        ptr += chunk;
        remaining -= chunk;
      }
      httpd_resp_send_chunk(req, NULL, 0);
      return ESP_OK;
    }

    static esp_err_t handleUpdateController(httpd_req_t* req) {
      auto* ds = self(req);
      if (!ds->enforceOwner(req)) return ESP_OK;

      char body[512];
      int len = httpd_req_recv(req, body, sizeof(body) - 1);
      if (len <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
        return ESP_OK;
      }
      body[len] = '\0';

      float axes[20]; bool buttons[20]; uint32_t nA = 0, nB = 0;
      parseFloatArray(body, "axes", axes, 20, nA);
      parseBoolArray(body, "buttons", buttons, 20, nB);
      ds->_gs.write(millis(), axes, nA, buttons, nB);

      httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
      return ESP_OK;
    }

    static esp_err_t handleRobotControl(httpd_req_t* req) {
      auto* ds = self(req);
      if (!ds->enforceOwner(req)) return ESP_OK;

      char query[128] = {0};
      httpd_req_get_url_query_str(req, query, sizeof(query));

      char cmd[32] = {0};
      char autoVal[8] = {0};
      char autoLenVal[8] = {0};
      httpd_query_key_value(query, "cmd", cmd, sizeof(cmd));
      httpd_query_key_value(query, "auto", autoVal, sizeof(autoVal));
      httpd_query_key_value(query, "autoLen", autoLenVal, sizeof(autoLenVal));

      bool enAuto = atoi(autoVal) != 0;
      int autoLen = atoi(autoLenVal);

      if (strcmp(cmd, "init") == 0) {
        ds->_rs.setStatus(millis(), robot::Status::INIT);
        ds->_rs.setDeadlineMiss(millis(), false);
      } else if (strcmp(cmd, "start") == 0) {
        ds->_rs.setAutonomous(millis(), enAuto);
        if (autoLen > 0) ds->_rs.setAutoPeriodSeconds(millis(), autoLen);
        ds->_rs.setStatus(millis(), robot::Status::START);
      } else if (strcmp(cmd, "cancelAuto") == 0) {
        ds->_rs.setAutonomous(millis(), false);
      } else if (strcmp(cmd, "stop") == 0) {
        ds->_rs.setStatus(millis(), robot::Status::STOP);
      }

      httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
      return ESP_OK;
    }

    static esp_err_t handleGetState(httpd_req_t* req) {
      auto* ds = self(req);
      if (!ds->enforceOwner(req)) return ESP_OK;

      auto s = ds->_rs.read();
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

      httpd_resp_set_type(req, "application/json");
      httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
      return ESP_OK;
    }

    static esp_err_t handleGetBattery(httpd_req_t* req) {
      auto* ds = self(req);
      auto s = ds->_rs.read();
      char buf[16];
      dtostrf(s.batteryVoltage, 0, 1, buf);
      httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
      return ESP_OK;
    }

    static esp_err_t handleTelemetry(httpd_req_t* req) {
      auto* ds = self(req);
      if (!ds->enforceOwner(req)) return ESP_OK;

      httpd_resp_send(req, probot::telemetry::getBuffer(), HTTPD_RESP_USE_STRLEN);
      return ESP_OK;
    }

    // /health and /info are OPEN (no owner enforcement).
    // Monitoring stations (judge/referee) need to observe robot liveness
    // without grabbing the DS ownership slot away from the active driver.
    static esp_err_t handleHealth(httpd_req_t* req) {
      auto* ds = self(req);

      int8_t rssi = -100;
      wifi_sta_list_t sta_list;
      if (esp_wifi_ap_get_sta_list(&sta_list) == ESP_OK && sta_list.num > 0) {
        rssi = sta_list.sta[0].rssi;
      }
      auto s = ds->_rs.read();
      char buf[128];
      snprintf(buf, sizeof(buf),
        "{\"rssi\":%d,\"up\":%lu,\"heap\":%lu,\"dm\":%s}",
        (int)rssi,
        (unsigned long)millis(),
        (unsigned long)ESP.getFreeHeap(),
        s.deadlineMiss ? "true" : "false");

      httpd_resp_set_type(req, "application/json");
      httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
      return ESP_OK;
    }

    static esp_err_t handleInfo(httpd_req_t* req) {
      auto* ds = self(req);

      // /info is now open (no owner check) so the password field is
      // omitted — anyone connected already has the password; we don't
      // want other teams' monitoring stations harvesting it.
      char buf[512];
      snprintf(buf, sizeof(buf),
        "{\"ssid\":\"%s\",\"ch\":%d,\"ip\":\"%s\","
        "\"chip\":\"%s\",\"cpuMhz\":%lu,\"sdk\":\"%s\","
        "\"totalHeap\":%lu,\"totalFlash\":%lu,"
        "\"sketchSize\":%lu,\"freeSketch\":%lu,\"psram\":%lu}",
        ds->ap_ssid_.c_str(),
        PROBOT_WIFI_AP_CHANNEL,
        WiFi.softAPIP().toString().c_str(),
        ESP.getChipModel(),
        (unsigned long)ESP.getCpuFreqMHz(),
        ESP.getSdkVersion(),
        (unsigned long)ESP.getHeapSize(),
        (unsigned long)ESP.getFlashChipSize(),
        (unsigned long)ESP.getSketchSize(),
        (unsigned long)ESP.getFreeSketchSpace(),
        (unsigned long)ESP.getPsramSize());

      httpd_resp_set_type(req, "application/json");
      httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
      return ESP_OK;
    }

    // ── Members ──
    robot::StateService& _rs;
    io::GamepadService&  _gs;
    WsJoystick           _ws;
    httpd_handle_t       _server = nullptr;
    bool                 _owner_set = false;
    char                 _owner_str[48] = {0};
    uint32_t             _owner_last_ms = 0;
    uint32_t             _owner_timeout_ms = 5000;
    String               ap_ssid_;
  };
}
#endif // ESP32
