#pragma once
#ifdef ESP32
#include <esp_http_server.h>
#include <probot/io/gamepad.hpp>

namespace probot::driverstation::esp32 {

  /**
   * WebSocket joystick server (ESP-IDF httpd, port 81)
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

    void begin(uint16_t port = 81) {
      httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
      cfg.server_port = port;
      cfg.ctrl_port   = port + 1;
      cfg.stack_size  = 4096;

      if (httpd_start(&_server, &cfg) != ESP_OK) {
        Serial.println("[WS   ] Failed to start WebSocket server");
        return;
      }

      httpd_uri_t ws_uri = {
        .uri      = "/joystick",
        .method   = HTTP_GET,
        .handler  = wsHandler,
        .user_ctx = this,
        .is_websocket             = true,
        .handle_ws_control_frames = true,
      };
      httpd_register_uri_handler(_server, &ws_uri);

      Serial.print("[WS   ] WebSocket server on port ");
      Serial.println(port);
    }

  private:
    static constexpr uint8_t  MAGIC    = 0x4A;
    static constexpr uint32_t MAX_AXES = 20;
    static constexpr uint32_t MAX_BTNS = 20;
    static constexpr size_t   MAX_FRAME = 4 + MAX_AXES * 2 + (MAX_BTNS + 7) / 8;

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
    }

    io::GamepadService& _gs;
    httpd_handle_t      _server = nullptr;
  };

}
#endif // ESP32
