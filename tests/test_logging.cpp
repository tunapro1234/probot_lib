#include "test_harness.hpp"

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

#include <probot/logging/logger.hpp>
#include <Arduino.h>

namespace {

struct ParsedFrame {
  uint16_t sequence{};
  probot::logging::Priority priority{};
  probot::logging::ValueType value_type{};
  uint32_t timestamp{};
  uint16_t drop_count{};
  std::string type_name;
  std::string instance_name;
  std::string field;
  int32_t int_value{};
  float float_value{};
  bool bool_value{};
  std::string text_value;
};

bool parseFrame(const std::vector<uint8_t>& buffer, size_t offset, ParsedFrame& out){
  if (offset + 5 > buffer.size()) return false;
  if (buffer[offset] != 0xAA || buffer[offset + 1] != 0x55) return false;
  uint16_t payload_len = static_cast<uint16_t>((buffer[offset + 2] << 8) | buffer[offset + 3]);
  size_t frame_size = static_cast<size_t>(payload_len) + 5; // sync(2) + len(2) + crc(1)
  if (offset + frame_size > buffer.size()) return false;

  size_t idx = offset + 4;

  out.sequence = static_cast<uint16_t>((buffer[idx] << 8) | buffer[idx + 1]); idx += 2;
  out.priority = static_cast<probot::logging::Priority>(buffer[idx++]);
  out.value_type = static_cast<probot::logging::ValueType>(buffer[idx++]);

  out.timestamp = (static_cast<uint32_t>(buffer[idx]) << 24) |
                  (static_cast<uint32_t>(buffer[idx + 1]) << 16) |
                  (static_cast<uint32_t>(buffer[idx + 2]) << 8) |
                  static_cast<uint32_t>(buffer[idx + 3]);
  idx += 4;

  out.drop_count = static_cast<uint16_t>((buffer[idx] << 8) | buffer[idx + 1]);
  idx += 2;

  auto readText = [&](std::string& dest)->bool{
    if (idx >= offset + frame_size - 1) return false;
    uint8_t len = buffer[idx++];
    if (idx + len > offset + frame_size - 1) return false;
    dest.assign(reinterpret_cast<const char*>(&buffer[idx]), len);
    idx += len;
    return true;
  };

  if (!readText(out.type_name)) return false;
  if (!readText(out.instance_name)) return false;
  if (!readText(out.field)) return false;

  switch (out.value_type){
    case probot::logging::ValueType::kInt: {
      if (idx + 4 > offset + frame_size - 1) return false;
      out.int_value = (static_cast<int32_t>(buffer[idx]) << 24) |
                      (static_cast<int32_t>(buffer[idx + 1]) << 16) |
                      (static_cast<int32_t>(buffer[idx + 2]) << 8) |
                      static_cast<int32_t>(buffer[idx + 3]);
      idx += 4;
      break;
    }
    case probot::logging::ValueType::kFloat: {
      if (idx + 4 > offset + frame_size - 1) return false;
      uint32_t raw = (static_cast<uint32_t>(buffer[idx]) << 24) |
                     (static_cast<uint32_t>(buffer[idx + 1]) << 16) |
                     (static_cast<uint32_t>(buffer[idx + 2]) << 8) |
                     static_cast<uint32_t>(buffer[idx + 3]);
      idx += 4;
      std::memcpy(&out.float_value, &raw, sizeof(float));
      break;
    }
    case probot::logging::ValueType::kBool: {
      if (idx + 1 > offset + frame_size - 1) return false;
      out.bool_value = buffer[idx++] != 0;
      break;
    }
    case probot::logging::ValueType::kString: {
      if (!readText(out.text_value)) return false;
      break;
    }
  }

  uint8_t crc_expected = 0;
  for (size_t i = 2; i < frame_size - 1; ++i) {
    crc_expected ^= buffer[offset + i];
  }
  uint8_t crc_read = buffer[offset + frame_size - 1];
  if (crc_expected != crc_read) return false;

  if (idx != offset + frame_size - 1) return false;
  return true;
}

struct DummySource {};

} // namespace

TEST_CASE(logging_serial_frame_basic){
  __serialStubClear();
  __wifiStubClear();
  auto& mgr = probot::logging::manager();
  probot::logging::enableSerialLogging(true);
  probot::logging::setSerialBandwidthMode(probot::logging::BandwidthMode::kFull);
  probot::logging::enableWifiLogging(false);
  probot::logging::setWifiSendCallback(nullptr);

  DummySource src;
  probot::logging::SourceRegistration reg{};
  reg.type_name = "test";
  reg.instance_name = "unit";
  reg.default_priority = probot::logging::Priority::kUserMarked;
  reg.allow_background_on_wifi = false;
  reg.supports_static_dump = false;
  reg.dynamic_collector = nullptr;
  reg.static_collector = nullptr;
  const auto* stored = probot::logging::registerSource(&src, reg);

  mgr.enqueueValue(*stored,
                   &src,
                   "value",
                   probot::logging::ValueType::kInt,
                   probot::logging::Priority::kUseDefault,
                   123,
                   0.0f,
                   false,
                   nullptr,
                   1234);
  mgr.update(1234, 20);

  const auto& buffer = __serialStubBuffer();
  EXPECT_TRUE(!buffer.empty());

  ParsedFrame frame{};
  EXPECT_TRUE(parseFrame(buffer, 0, frame));
  EXPECT_TRUE(frame.priority == probot::logging::Priority::kUserMarked);
  EXPECT_TRUE(frame.value_type == probot::logging::ValueType::kInt);
  EXPECT_TRUE(frame.timestamp == 1234);
  EXPECT_TRUE(frame.drop_count == 0);
  EXPECT_TRUE(frame.int_value == 123);
  EXPECT_TRUE(frame.type_name == "test");
  EXPECT_TRUE(frame.instance_name == "unit");
  EXPECT_TRUE(frame.field == "value");

  probot::logging::unregisterSource(&src);
  __serialStubClear();
}

TEST_CASE(logging_serial_reports_drops){
  __serialStubClear();
  __wifiStubClear();
  auto& mgr = probot::logging::manager();
  probot::logging::enableSerialLogging(true);
  probot::logging::setSerialBandwidthMode(probot::logging::BandwidthMode::kFull);
  probot::logging::enableWifiLogging(false);
  probot::logging::setWifiSendCallback(nullptr);

  DummySource src;
  probot::logging::SourceRegistration reg{};
  reg.type_name = "queue";
  reg.instance_name = "overrun";
  reg.default_priority = probot::logging::Priority::kBackground;
  reg.allow_background_on_wifi = false;
  reg.supports_static_dump = false;
  reg.dynamic_collector = nullptr;
  reg.static_collector = nullptr;
  const auto* stored = probot::logging::registerSource(&src, reg);

  for (int i = 0; i < 128; ++i){
    mgr.enqueueValue(*stored,
                     &src,
                     "load",
                     probot::logging::ValueType::kInt,
                     probot::logging::Priority::kUseDefault,
                     i,
                     0.0f,
                     false,
                     nullptr,
                     2000 + static_cast<uint32_t>(i));
  }

  mgr.update(3000, 20);
  const auto& buffer = __serialStubBuffer();
  EXPECT_TRUE(!buffer.empty());

  ParsedFrame frame{};
  EXPECT_TRUE(parseFrame(buffer, 0, frame));
  EXPECT_TRUE(frame.drop_count > 0);
  EXPECT_TRUE(frame.priority == probot::logging::Priority::kBackground);

  probot::logging::unregisterSource(&src);
  __serialStubClear();
}

TEST_CASE(logging_wifi_frame_basic){
  __wifiStubClear();
  __serialStubClear();
  auto& mgr = probot::logging::manager();
  probot::logging::enableWifiLogging(true);
  probot::logging::setWifiBandwidthMode(probot::logging::BandwidthMode::kFull);
  probot::logging::setWifiSendCallback(__wifiStubSend);
  probot::logging::setWifiEndpoint(0xFFFFFFFFu, 49160);

  DummySource src;
  probot::logging::SourceRegistration reg{};
  reg.type_name = "wifi";
  reg.instance_name = "unit";
  reg.default_priority = probot::logging::Priority::kUserMarked;
  reg.allow_background_on_wifi = false;
  reg.supports_static_dump = false;
  const auto* stored = probot::logging::registerSource(&src, reg);

  mgr.enqueueValue(*stored,
                   &src,
                   "value",
                   probot::logging::ValueType::kFloat,
                   probot::logging::Priority::kUseDefault,
                   0,
                   42.5f,
                   false,
                   nullptr,
                   3210);
  mgr.update(3210, 20);

  const auto& buffer = __wifiStubBuffer();
  EXPECT_TRUE(!buffer.empty());

  ParsedFrame frame{};
  EXPECT_TRUE(parseFrame(buffer, 0, frame));
  EXPECT_TRUE(frame.priority == probot::logging::Priority::kUserMarked);
  EXPECT_TRUE(frame.value_type == probot::logging::ValueType::kFloat);
  EXPECT_NEAR(frame.float_value, 42.5f, 1e-3f);
  EXPECT_TRUE(frame.type_name == "wifi");
  EXPECT_TRUE(frame.instance_name == "unit");

  probot::logging::unregisterSource(&src);
  __wifiStubClear();
  probot::logging::enableWifiLogging(false);
  probot::logging::setWifiSendCallback(nullptr);
}

TEST_CASE(logging_wifi_reports_drops){
  __wifiStubClear();
  __serialStubClear();
  auto& mgr = probot::logging::manager();
  probot::logging::enableWifiLogging(true);
  probot::logging::setWifiBandwidthMode(probot::logging::BandwidthMode::kFull);
  probot::logging::setWifiSendCallback(__wifiStubSendFail);
  probot::logging::setWifiEndpoint(0xFFFFFFFFu, 49160);

  DummySource src;
  probot::logging::SourceRegistration reg{};
  reg.type_name = "wifi";
  reg.instance_name = "drops";
  reg.default_priority = probot::logging::Priority::kUserMarked;
  reg.allow_background_on_wifi = false;
  reg.supports_static_dump = false;
  const auto* stored = probot::logging::registerSource(&src, reg);

  mgr.enqueueValue(*stored,
                   &src,
                   "value",
                   probot::logging::ValueType::kInt,
                   probot::logging::Priority::kUseDefault,
                   99,
                   0.0f,
                   false,
                   nullptr,
                   4000);
  mgr.update(4000, 20);

  // First send should fail and not populate buffer
  EXPECT_TRUE(__wifiStubBuffer().empty());

  probot::logging::setWifiSendCallback(__wifiStubSend);

  mgr.enqueueValue(*stored,
                   &src,
                   "value",
                   probot::logging::ValueType::kInt,
                   probot::logging::Priority::kUseDefault,
                   55,
                   0.0f,
                   false,
                   nullptr,
                   4050);
  mgr.update(4050, 20);

  const auto& buffer = __wifiStubBuffer();
  EXPECT_TRUE(!buffer.empty());
  ParsedFrame frame{};
  EXPECT_TRUE(parseFrame(buffer, 0, frame));
  EXPECT_TRUE(frame.drop_count > 0);
  EXPECT_TRUE(frame.int_value == 55);

  probot::logging::unregisterSource(&src);
  probot::logging::enableWifiLogging(false);
  probot::logging::setWifiSendCallback(nullptr);
  __wifiStubClear();
}
