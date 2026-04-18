#pragma once
#ifdef ESP32
#include <esp_http_server.h>
#include <probot/io/gamepad.hpp>
#include <probot/robot/state.hpp>

namespace probot::driverstation::esp32 {

  /**
   * WebSocket joystick handler (attaches to existing ESP-IDF httpd)
   *
   * Binary frame format:
   *   [0]       uint8   0x4A ('J' magic)
   *   [1]       uint8   axisCount   (max 20)
   *   [2]       uint8   buttonCount (max 20)
   *   [3]       uint8   reserved
   *   [4..]     int16[] axes (big-endian, value = float * 32767)
   *   [4+nA*2]  uint8[] buttons (packed bits, LSB first)
   */
  class WsJoystick {
  public:
    explicit WsJoystick(io::GamepadService& gs) : _gs(gs) {}

    void attach(httpd_handle_t server) {
      _server = server;

      httpd_uri_t ws_uri = {
        .uri      = "/joystick",
        .method   = HTTP_GET,
        .handler  = wsHandler,
        .user_ctx = this,
        .is_websocket             = true,
        .handle_ws_control_frames = true,
      };
      httpd_register_uri_handler(_server, &ws_uri);

      // Periodic ping to detect dead connections
      _pingTimer = xTimerCreate("ws_ping", pdMS_TO_TICKS(2000), pdTRUE, this, pingTimerCb);
      if (_pingTimer) xTimerStart(_pingTimer, 0);

      Serial.println("[WS   ] WebSocket handler attached to /joystick");
    }

    void closeAll() {
      if (!_server) return;
      size_t fds = 8;
      int clients[8];
      if (httpd_get_client_list(_server, &fds, clients) != ESP_OK) return;
      for (size_t i = 0; i < fds; i++) {
        if (httpd_ws_get_fd_info(_server, clients[i]) == HTTPD_WS_CLIENT_WEBSOCKET) {
          httpd_sess_trigger_close(_server, clients[i]);
        }
      }
    }

  private:
    static constexpr uint8_t  MAGIC    = 0x4A;
    static constexpr uint32_t MAX_AXES = 20;
    static constexpr uint32_t MAX_BTNS = 20;
    static constexpr size_t   MAX_FRAME = 4 + MAX_AXES * 2 + (MAX_BTNS + 7) / 8;

    // Close a WS session only after this many consecutive ping send
    // failures. Tolerates brief RF hiccups in noisy environments (a
    // single lost frame used to close the connection).
    static constexpr uint8_t  PING_MAX_FAILS   = 3;
    static constexpr uint8_t  PING_TRACK_SLOTS = 8;

    struct PingState { int fd = -1; uint8_t fails = 0; };

    static esp_err_t wsHandler(httpd_req_t* req) {
      if (req->method == HTTP_GET) {
        return ESP_OK; // WS handshake — just accept
      }

      auto* self = static_cast<WsJoystick*>(req->user_ctx);

      // Step 1: 0-length receive to learn frame size & type
      httpd_ws_frame_t frame = {};
      frame.payload = nullptr;
      esp_err_t ret = httpd_ws_recv_frame(req, &frame, 0);
      if (ret != ESP_OK) return ret;

      // Handle control frames
      if (frame.type == HTTPD_WS_TYPE_CLOSE) return ESP_OK;
      if (frame.type == HTTPD_WS_TYPE_PING) {
        httpd_ws_frame_t pong = {};
        pong.type = HTTPD_WS_TYPE_PONG;
        return httpd_ws_send_frame(req, &pong);
      }
      if (frame.type != HTTPD_WS_TYPE_BINARY) return ESP_OK;

      // Step 2: bounds check, then receive payload
      if (frame.len == 0 || frame.len > MAX_FRAME) return ESP_OK;

      uint8_t buf[MAX_FRAME];
      frame.payload = buf;
      ret = httpd_ws_recv_frame(req, &frame, frame.len);
      if (ret != ESP_OK) return ret;

      self->parseFrame(buf, frame.len);
      return ESP_OK;
    }

    void parseFrame(const uint8_t* data, size_t len) {
      if (len < 4 || data[0] != MAGIC) return;

      uint32_t nA = data[1];
      uint32_t nB = data[2];
      if (nA > MAX_AXES) nA = MAX_AXES;
      if (nB > MAX_BTNS) nB = MAX_BTNS;

      size_t expected = 4 + nA * 2 + (nB + 7) / 8;
      if (len < expected) return;

      float axes[MAX_AXES];
      const uint8_t* p = data + 4;
      for (uint32_t i = 0; i < nA; i++) {
        int16_t raw = static_cast<int16_t>((p[0] << 8) | p[1]);
        axes[i] = static_cast<float>(raw) / 32767.0f;
        p += 2;
      }

      bool buttons[MAX_BTNS];
      for (uint32_t i = 0; i < nB; i++) {
        buttons[i] = (p[i / 8] >> (i % 8)) & 1;
      }

      _gs.write(millis(), axes, nA, buttons, nB);
      __atomic_store_n(&probot::robot::g_ds_last_activity_ms, millis(), __ATOMIC_SEQ_CST);
    }

    uint8_t* trackPingFd(int fd) {
      for (auto& s : _pingState) if (s.fd == fd) return &s.fails;
      for (auto& s : _pingState) if (s.fd == -1) { s.fd = fd; s.fails = 0; return &s.fails; }
      return nullptr;
    }

    void clearPingFd(int fd) {
      for (auto& s : _pingState) if (s.fd == fd) { s.fd = -1; s.fails = 0; }
    }

    static void pingTimerCb(TimerHandle_t t) {
      auto* self = static_cast<WsJoystick*>(pvTimerGetTimerID(t));
      if (!self->_server) return;
      httpd_ws_frame_t ping = {};
      ping.type = HTTPD_WS_TYPE_PING;
      size_t fds = 8;
      int clients[8];
      if (httpd_get_client_list(self->_server, &fds, clients) != ESP_OK) return;

      // Prune tracked fds that are no longer WS clients
      for (auto& s : self->_pingState) {
        if (s.fd == -1) continue;
        if (httpd_ws_get_fd_info(self->_server, s.fd) != HTTPD_WS_CLIENT_WEBSOCKET) {
          s.fd = -1; s.fails = 0;
        }
      }

      for (size_t i = 0; i < fds; i++) {
        if (httpd_ws_get_fd_info(self->_server, clients[i]) != HTTPD_WS_CLIENT_WEBSOCKET) continue;
        uint8_t* fails = self->trackPingFd(clients[i]);
        esp_err_t err = httpd_ws_send_frame_async(self->_server, clients[i], &ping);
        if (err != ESP_OK) {
          if (fails && ++(*fails) >= PING_MAX_FAILS) {
            Serial.printf("[WS   ] /joystick ping fail x%u, closing fd=%d\n",
                          (unsigned)*fails, clients[i]);
            httpd_sess_trigger_close(self->_server, clients[i]);
            self->clearPingFd(clients[i]);
          }
        } else if (fails) {
          *fails = 0;
        }
      }
    }

    io::GamepadService& _gs;
    httpd_handle_t      _server = nullptr;
    TimerHandle_t       _pingTimer = nullptr;
    PingState           _pingState[PING_TRACK_SLOTS] = {};
  };

}
#endif // ESP32
