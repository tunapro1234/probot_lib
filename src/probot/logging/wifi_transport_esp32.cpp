#ifdef ESP32
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <probot/logging/logger.hpp>

namespace probot::logging {
namespace {

const uint16_t kDefaultWifiLoggingPort = 49160;
WiFiUDP s_udp;

bool sendUdp(const uint8_t* data, size_t len){
  if (!data || len == 0) return false;
  uint16_t port = wifiEndpointPort();
  if (port == 0) port = kDefaultWifiLoggingPort;
  uint32_t ip_be = wifiEndpointIp();
  IPAddress target;
  if (ip_be == 0){
    target = IPAddress(255, 255, 255, 255);
  } else {
    target = IPAddress((ip_be >> 24) & 0xFF,
                       (ip_be >> 16) & 0xFF,
                       (ip_be >> 8) & 0xFF,
                       ip_be & 0xFF);
  }

  // Bind local port if needed (ignore result to avoid repeated errors)
  s_udp.begin(port);

  if (!s_udp.beginPacket(target, port)){
    return false;
  }
  size_t written = s_udp.write(data, len);
  if (written != len){
    s_udp.endPacket();
    return false;
  }
  return s_udp.endPacket() == 1;
}

struct WifiLoggingInitializer {
  WifiLoggingInitializer(){
    setWifiEndpoint(0xFFFFFFFFu, kDefaultWifiLoggingPort);
    setWifiSendCallback(sendUdp);
  }
};

static WifiLoggingInitializer g_wifi_logging_initializer;

} // namespace
} // namespace probot::logging

#endif // ESP32
