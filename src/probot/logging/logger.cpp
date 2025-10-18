#include <probot/logging/logger.hpp>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

namespace probot::logging {

namespace {
  constexpr size_t kMaxSources       = 128;
  constexpr size_t kMaxWrappers      = 128;
  constexpr size_t kQueueCapacity    = 96;
  constexpr size_t kPriorityCount    = 3;
  constexpr size_t kMaxTypeLen       = 24;
  constexpr size_t kMaxInstanceLen   = 32;
  constexpr size_t kMaxFieldLen      = 24;
  constexpr size_t kMaxTextLen       = 64;

  struct SourceRecord {
    bool                 used;
    const void*          object;
    SourceRegistration   reg;
    char                 type_buf[kMaxTypeLen];
    char                 instance_buf[kMaxInstanceLen];
  };

  struct WrapperRecord {
    bool                          used;
    const ::control::IUpdatable* original;
    ::control::IUpdatable*        wrapper;
  };

  struct LogSample {
    const SourceRegistration* reg;
    const void*               source;
    char                      field[kMaxFieldLen];
    ValueType                 type;
    Priority                  priority;
    int32_t                   int_value;
    float                     float_value;
    bool                      bool_value;
    char                      text[kMaxTextLen];
    bool                      has_text;
    uint32_t                  timestamp_ms;
  };

  template <size_t Capacity>
  struct RingBuffer {
    LogSample data[Capacity];
    size_t    head;
    size_t    tail;
    size_t    count;

    RingBuffer() : head(0), tail(0), count(0) {}

    bool full()  const { return count == Capacity; }
    bool empty() const { return count == 0; }

    bool push(const LogSample& sample){
      if (full()) return false;
      data[tail] = sample;
      tail = (tail + 1) % Capacity;
      ++count;
      return true;
    }

    bool pop(LogSample& out){
      if (empty()) return false;
      out = data[head];
      head = (head + 1) % Capacity;
      --count;
      return true;
    }

    bool dropOldest(){
      if (empty()) return false;
      head = (head + 1) % Capacity;
      --count;
      return true;
    }
  };

  struct SchedulerTag {
    int dummy;
  };

  static SourceRecord g_sources[kMaxSources];
  static WrapperRecord g_wrappers[kMaxWrappers];
  static SchedulerTag g_scheduler_tag{};
  static const SourceRegistration* g_scheduler_reg = nullptr;

  uint16_t budgetForMode(BandwidthMode mode){
    switch (mode){
      case BandwidthMode::kNever:  return 0;
      case BandwidthMode::kLow:    return 2;
      case BandwidthMode::kNormal: return 6;
      case BandwidthMode::kHigh:   return 12;
      case BandwidthMode::kFull:   return 0xFFFFu;
      default:                     return 0;
    }
  }

  Priority resolvePriority(Priority override, Priority base){
    return (override == Priority::kUseDefault) ? base : override;
  }

  void copyString(char* dest, size_t dest_size, const char* src){
    if (!dest || !dest_size) return;
    if (!src){
      dest[0] = '\0';
      return;
    }
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
  }

  SourceRecord* findSourceRecord(const void* object){
    if (!object) return nullptr;
    for (size_t i=0;i<kMaxSources;i++){
      if (g_sources[i].used && g_sources[i].object == object){
        return &g_sources[i];
      }
    }
    return nullptr;
  }

  SourceRecord* allocateSourceRecord(const void* object){
    for (size_t i=0;i<kMaxSources;i++){
      if (!g_sources[i].used){
        g_sources[i].used = true;
        g_sources[i].object = object;
        return &g_sources[i];
      }
    }
    return nullptr;
  }

  WrapperRecord* findWrapperRecord(const ::control::IUpdatable* original){
    if (!original) return nullptr;
    for (size_t i=0;i<kMaxWrappers;i++){
      if (g_wrappers[i].used && g_wrappers[i].original == original){
        return &g_wrappers[i];
      }
    }
    return nullptr;
  }

  WrapperRecord* allocateWrapperRecord(const ::control::IUpdatable* original){
    for (size_t i=0;i<kMaxWrappers;i++){
      if (!g_wrappers[i].used){
        g_wrappers[i].used = true;
        g_wrappers[i].original = original;
        g_wrappers[i].wrapper = nullptr;
        return &g_wrappers[i];
      }
    }
    return nullptr;
  }

  class LoggedUpdatable : public ::control::IUpdatable {
  public:
    LoggedUpdatable(::control::IUpdatable* inner,
                    const SourceRegistration* registration)
    : inner_(inner), registration_(registration) {}

    void update(uint32_t now_ms, uint32_t dt_ms) override {
      if (!inner_) return;
      inner_->update(now_ms, dt_ms);
      LoggingManager& mgr = LoggingManager::instance();
      if (!registration_) return;
      TelemetryCollector collector(mgr, *registration_, inner_, now_ms);
      collector.addInt("dt_ms", static_cast<int32_t>(dt_ms));
      if (registration_->dynamic_collector){
        registration_->dynamic_collector(inner_, collector, now_ms, dt_ms);
      }
    }

    ::control::IUpdatable* inner() const { return inner_; }

  private:
    ::control::IUpdatable* inner_;
    const SourceRegistration* registration_;
  };

  const SourceRegistration* ensureDefaultRegistration(const void* object){
    SourceRecord* rec = findSourceRecord(object);
    if (rec) return &rec->reg;
    SourceRegistration tmp{};
    tmp.type_name = "updatable";
    tmp.instance_name = nullptr;
    tmp.default_priority = Priority::kBackground;
    tmp.allow_background_on_wifi = true;
    tmp.supports_static_dump = false;
    tmp.dynamic_collector = nullptr;
    tmp.static_collector = nullptr;
    return registerSource(object, tmp);
  }

} // namespace

struct LoggingManager::Impl {
  template <size_t Capacity>
  struct PriorityRing : public RingBuffer<Capacity> {};

  PriorityRing<kQueueCapacity> queues[kPriorityCount];

  struct TransportState {
    bool          enabled;
    BandwidthMode mode;
    uint16_t      sequence;
    uint16_t      drop_counter;
    uint32_t      endpoint_ip_be;
    uint16_t      endpoint_port;

    TransportState()
    : enabled(false), mode(BandwidthMode::kNever), sequence(0), drop_counter(0),
      endpoint_ip_be(0), endpoint_port(0) {}
  };

  TransportState serial;
  TransportState wifi;
  WifiSendFn     wifi_writer;
};

LoggingManager& manager(){
  return LoggingManager::instance();
}

LoggingManager::LoggingManager()
: impl_(new Impl()) {
  impl_->serial.enabled = true;
  impl_->serial.mode = BandwidthMode::kNormal;
  impl_->serial.sequence = 0;
  impl_->serial.drop_counter = 0;
  impl_->serial.endpoint_ip_be = 0;
  impl_->serial.endpoint_port = 0;
  impl_->wifi.enabled = false;
  impl_->wifi.mode = BandwidthMode::kNever;
  impl_->wifi.sequence = 0;
  impl_->wifi.drop_counter = 0;
  impl_->wifi.endpoint_ip_be = 0;
  impl_->wifi.endpoint_port = 0;
  impl_->wifi_writer = nullptr;
}

LoggingManager::~LoggingManager(){
  delete impl_;
  impl_ = nullptr;
}

LoggingManager& LoggingManager::instance(){
  static LoggingManager g_instance;
  return g_instance;
}

void LoggingManager::enableSerial(bool enabled){
  impl_->serial.enabled = enabled;
}

void LoggingManager::setSerialBandwidth(BandwidthMode mode){
  impl_->serial.mode = mode;
}

void LoggingManager::enableWifi(bool enabled){
  impl_->wifi.enabled = enabled;
}

void LoggingManager::setWifiBandwidth(BandwidthMode mode){
  impl_->wifi.mode = mode;
}

void LoggingManager::setWifiSendCallbackInternal(WifiSendFn fn){
  impl_->wifi_writer = fn;
}

void LoggingManager::setWifiEndpointInternal(uint32_t ipv4_be, uint16_t port){
  impl_->wifi.endpoint_ip_be = ipv4_be;
  impl_->wifi.endpoint_port = port;
}

uint32_t LoggingManager::wifiEndpointIpInternal() const{
  return impl_->wifi.endpoint_ip_be;
}

uint16_t LoggingManager::wifiEndpointPortInternal() const{
  return impl_->wifi.endpoint_port;
}

void LoggingManager::enqueueValue(const SourceRegistration& reg,
                                  const void* source,
                                  const char* field,
                                  ValueType type,
                                  Priority priority,
                                  int32_t int_value,
                                  float float_value,
                                  bool bool_value,
                                  const char* str_value,
                                  uint32_t timestamp_ms){
  Priority final_priority = resolvePriority(priority, reg.default_priority);
  size_t idx = static_cast<size_t>(final_priority);
  if (idx >= kPriorityCount){
    final_priority = reg.default_priority;
    idx = static_cast<size_t>(final_priority);
  }

  LogSample sample{};
  sample.reg = &reg;
  sample.source = source;
  copyString(sample.field, sizeof(sample.field), field ? field : "value");
  sample.type = type;
  sample.priority = final_priority;
  sample.int_value = int_value;
  sample.float_value = float_value;
  sample.bool_value = bool_value;
  sample.timestamp_ms = timestamp_ms;
  sample.has_text = false;
  if (type == ValueType::kString && str_value){
    copyString(sample.text, sizeof(sample.text), str_value);
    sample.has_text = true;
  }

  auto& queue = impl_->queues[idx];
  if (!queue.push(sample)){
    if (final_priority == Priority::kBackground){
      recordDrop(true, true);
      return;
    }
    if (final_priority == Priority::kUserMarked){
      auto& background = impl_->queues[static_cast<size_t>(Priority::kBackground)];
      if (!background.dropOldest()){
        if (queue.dropOldest()){
          recordDrop(true, true);
        }
      } else {
        recordDrop(true, true);
      }
    } else { // system critical
      auto& background = impl_->queues[static_cast<size_t>(Priority::kBackground)];
      if (!background.dropOldest()){
        auto& user = impl_->queues[static_cast<size_t>(Priority::kUserMarked)];
        if (!user.dropOldest()){
          if (queue.dropOldest()){
            recordDrop(true, true);
          }
        } else {
          recordDrop(true, true);
        }
      } else {
        recordDrop(true, true);
      }
    }
    if (!queue.push(sample)){
      recordDrop(true, true);
    }
  }
}

void LoggingManager::publishSystemEvent(const char* origin,
                                        const char* message,
                                        uint32_t timestamp_ms){
  const SourceRegistration* reg = g_scheduler_reg;
  if (!reg){
    SourceRegistration tmp{};
    tmp.type_name = origin ? origin : "system";
    tmp.instance_name = "core";
    tmp.default_priority = Priority::kSystemCritical;
    tmp.allow_background_on_wifi = true;
    tmp.supports_static_dump = false;
    tmp.dynamic_collector = nullptr;
    tmp.static_collector = nullptr;
    g_scheduler_reg = registerSource(&g_scheduler_tag, tmp);
    reg = g_scheduler_reg;
  }
  if (!reg) return;
  enqueueValue(*reg,
               &g_scheduler_tag,
               "event",
               ValueType::kString,
               Priority::kSystemCritical,
               0,
               0.0f,
               false,
               message,
               timestamp_ms);
}

void LoggingManager::update(uint32_t now_ms, uint32_t dt_ms){
  (void)now_ms;
  (void)dt_ms;
  uint16_t serial_budget = impl_->serial.enabled ? budgetForMode(impl_->serial.mode) : 0;
  uint16_t wifi_budget   = impl_->wifi.enabled   ? budgetForMode(impl_->wifi.mode)   : 0;

  auto encodeFrame = [&](Impl::TransportState& transport,
                         const LogSample& sample,
                         uint8_t* frame,
                         size_t capacity,
                         size_t& out_len)->bool {
    size_t pos = 0;
    bool ok = true;

    auto putByte = [&](uint8_t value){
      if (!ok) return;
      if (pos >= capacity){
        ok = false;
        return;
      }
      frame[pos++] = value;
    };

    auto putBytes = [&](const uint8_t* data, size_t len){
      if (!ok) return;
      if (pos + len > capacity){
        ok = false;
        return;
      }
      memcpy(frame + pos, data, len);
      pos += len;
    };

    auto putText = [&](const char* text){
      if (!ok) return;
      size_t len = text ? strlen(text) : 0;
      if (len > 255) len = 255;
      putByte(static_cast<uint8_t>(len));
      if (len > 0 && ok){
        putBytes(reinterpret_cast<const uint8_t*>(text), len);
      }
    };

    putByte(0xAA);
    putByte(0x55);
    size_t len_pos = pos;
    putByte(0u);
    putByte(0u);

    uint16_t seq = transport.sequence++;
    putByte(static_cast<uint8_t>((seq >> 8) & 0xFF));
    putByte(static_cast<uint8_t>(seq & 0xFF));
    putByte(static_cast<uint8_t>(sample.priority));
    putByte(static_cast<uint8_t>(sample.type));

    uint32_t ts = sample.timestamp_ms;
    putByte(static_cast<uint8_t>((ts >> 24) & 0xFF));
    putByte(static_cast<uint8_t>((ts >> 16) & 0xFF));
    putByte(static_cast<uint8_t>((ts >> 8) & 0xFF));
    putByte(static_cast<uint8_t>(ts & 0xFF));

    uint16_t drops = transport.drop_counter;
    putByte(static_cast<uint8_t>((drops >> 8) & 0xFF));
    putByte(static_cast<uint8_t>(drops & 0xFF));

    const char* type_name = (sample.reg && sample.reg->type_name) ? sample.reg->type_name : "";
    const char* instance_name = (sample.reg && sample.reg->instance_name) ? sample.reg->instance_name : "";
    putText(type_name);
    putText(instance_name);
    putText(sample.field);

    if (!ok){
      return false;
    }

    switch (sample.type){
      case ValueType::kInt: {
        int32_t val = sample.int_value;
        putByte(static_cast<uint8_t>((val >> 24) & 0xFF));
        putByte(static_cast<uint8_t>((val >> 16) & 0xFF));
        putByte(static_cast<uint8_t>((val >> 8) & 0xFF));
        putByte(static_cast<uint8_t>(val & 0xFF));
        break;
      }
      case ValueType::kFloat: {
        static_assert(sizeof(float) == sizeof(uint32_t), "float must be 32 bits");
        uint32_t raw = 0;
        memcpy(&raw, &sample.float_value, sizeof(float));
        putByte(static_cast<uint8_t>((raw >> 24) & 0xFF));
        putByte(static_cast<uint8_t>((raw >> 16) & 0xFF));
        putByte(static_cast<uint8_t>((raw >> 8) & 0xFF));
        putByte(static_cast<uint8_t>(raw & 0xFF));
        break;
      }
      case ValueType::kBool: {
        putByte(sample.bool_value ? 1u : 0u);
        break;
      }
      case ValueType::kString: {
        const char* text = sample.has_text ? sample.text : "";
        putText(text);
        break;
      }
    }

    if (!ok){
      return false;
    }

    uint16_t payload_len = static_cast<uint16_t>(pos - (len_pos + 2));
    frame[len_pos] = static_cast<uint8_t>((payload_len >> 8) & 0xFF);
    frame[len_pos + 1] = static_cast<uint8_t>(payload_len & 0xFF);

    uint8_t crc = 0;
    for (size_t i = 2; i < pos; ++i){
      crc ^= frame[i];
    }
    putByte(crc);

    if (!ok){
      return false;
    }

    out_len = pos;
    return true;
  };

  auto sendSerialFrame = [&](const LogSample& sample){
    uint8_t frame[256];
    size_t len = 0;
    if (!encodeFrame(impl_->serial, sample, frame, sizeof(frame), len)){
      recordDrop(true, false);
      return;
    }
    size_t written = Serial.write(frame, len);
    if (written != len){
      recordDrop(true, false);
      return;
    }
    impl_->serial.drop_counter = 0;
  };

  auto sendWifiFrame = [&](const LogSample& sample){
    if (!impl_->wifi_writer) return;
    uint8_t frame[256];
    size_t len = 0;
    if (!encodeFrame(impl_->wifi, sample, frame, sizeof(frame), len)){
      recordDrop(false, true);
      return;
    }
    if (!impl_->wifi_writer(frame, len)){
      recordDrop(false, true);
      return;
    }
    impl_->wifi.drop_counter = 0;
  };

  while ((serial_budget > 0) || (wifi_budget > 0)){
    LogSample sample{};
    bool found = false;
    for (size_t i=0;i<kPriorityCount;i++){
      if (impl_->queues[i].pop(sample)){
        found = true;
        break;
      }
    }
    if (!found) break;

    if (serial_budget > 0){
      sendSerialFrame(sample);
      --serial_budget;
    }

    if (wifi_budget > 0){
      if (sample.priority != Priority::kBackground){
        sendWifiFrame(sample);
        --wifi_budget;
      }
    }
  }
}

void LoggingManager::emitStatic(const void* source){
  const SourceRegistration* reg = findSource(source);
  if (!reg || !reg->supports_static_dump || !reg->static_collector) return;
  TelemetryCollector collector(*this, *reg, source, millis());
  reg->static_collector(source, collector);
}

void LoggingManager::recordDrop(bool serial, bool wifi){
  if (serial && impl_->serial.drop_counter < 0xFFFFu){
    impl_->serial.drop_counter += 1;
  }
  if (wifi && impl_->wifi.drop_counter < 0xFFFFu){
    impl_->wifi.drop_counter += 1;
  }
}

::control::IUpdatable* LoggingManager::wrap(::control::IUpdatable* object){
  if (!object) return nullptr;
  WrapperRecord* existing = findWrapperRecord(object);
  if (existing){
    return existing->wrapper ? existing->wrapper : object;
  }

  const SourceRegistration* reg = findSource(object);
  if (!reg){
    reg = ensureDefaultRegistration(object);
  }
  WrapperRecord* slot = allocateWrapperRecord(object);
  if (!slot) return object;
  if (reg){
    auto* wrapper = new LoggedUpdatable(object, reg);
    slot->wrapper = wrapper;
    return wrapper;
  } else {
    slot->wrapper = nullptr;
    return object;
  }
}

::control::IUpdatable* LoggingManager::lookupWrapper(const ::control::IUpdatable* original) const {
  WrapperRecord* rec = findWrapperRecord(original);
  if (!rec) return nullptr;
  return rec->wrapper;
}

void LoggingManager::releaseWrapper(const ::control::IUpdatable* original){
  WrapperRecord* rec = findWrapperRecord(original);
  if (!rec) return;
  if (rec->wrapper){
    auto* logged = static_cast<LoggedUpdatable*>(rec->wrapper);
    delete logged;
  }
  rec->used = false;
  rec->original = nullptr;
  rec->wrapper = nullptr;
}

void LoggingManager::onSourceRegistered(const void* object,
                                        const SourceRegistration& reg){
  (void)object;
  (void)reg;
}

void LoggingManager::onSourceUnregistered(const void* object){
  WrapperRecord* rec = findWrapperRecord(static_cast<const ::control::IUpdatable*>(object));
  if (!rec) return;
  if (rec->wrapper){
    delete static_cast<LoggedUpdatable*>(rec->wrapper);
  }
  rec->used = false;
  rec->original = nullptr;
  rec->wrapper = nullptr;
}

const SourceRegistration* registerSource(const void* object,
                                         const SourceRegistration& registration){
  if (!object) return nullptr;
  SourceRecord* rec = findSourceRecord(object);
  if (!rec){
    rec = allocateSourceRecord(object);
  }
  if (!rec) return nullptr;

  copyString(rec->type_buf, sizeof(rec->type_buf),
             registration.type_name ? registration.type_name : "component");
  if (registration.instance_name){
    copyString(rec->instance_buf, sizeof(rec->instance_buf),
               registration.instance_name);
  } else {
    snprintf(rec->instance_buf,
             sizeof(rec->instance_buf),
             "%s@%p",
             rec->type_buf,
             object);
  }

  rec->reg.type_name               = rec->type_buf;
  rec->reg.instance_name           = rec->instance_buf;
  rec->reg.default_priority        = registration.default_priority;
  rec->reg.allow_background_on_wifi= registration.allow_background_on_wifi;
  rec->reg.supports_static_dump    = registration.supports_static_dump;
  rec->reg.dynamic_collector       = registration.dynamic_collector;
  rec->reg.static_collector        = registration.static_collector;

  LoggingManager::instance().onSourceRegistered(object, rec->reg);
  return &rec->reg;
}

void unregisterSource(const void* object){
  SourceRecord* rec = findSourceRecord(object);
  if (!rec) return;
  LoggingManager::instance().onSourceUnregistered(object);
  rec->used = false;
  rec->object = nullptr;
}

const SourceRegistration* findSource(const void* object){
  SourceRecord* rec = findSourceRecord(object);
  return rec ? &rec->reg : nullptr;
}

TelemetryCollector::TelemetryCollector(LoggingManager& manager,
                                       const SourceRegistration& registration,
                                       const void* source,
                                       uint32_t timestamp_ms)
: manager_(manager),
  registration_(registration),
  source_(source),
  base_priority_(registration.default_priority),
  timestamp_ms_(timestamp_ms) {}

void TelemetryCollector::addInt(const char* field,
                                int32_t value,
                                Priority priority_override){
  manager_.enqueueValue(registration_,
                        source_,
                        field,
                        ValueType::kInt,
                        resolvePriority(priority_override, base_priority_),
                        value,
                        0.0f,
                        false,
                        nullptr,
                        timestamp_ms_);
}

void TelemetryCollector::addFloat(const char* field,
                                  float value,
                                  Priority priority_override){
  manager_.enqueueValue(registration_,
                        source_,
                        field,
                        ValueType::kFloat,
                        resolvePriority(priority_override, base_priority_),
                        0,
                        value,
                        false,
                        nullptr,
                        timestamp_ms_);
}

void TelemetryCollector::addBool(const char* field,
                                 bool value,
                                 Priority priority_override){
  manager_.enqueueValue(registration_,
                        source_,
                        field,
                        ValueType::kBool,
                        resolvePriority(priority_override, base_priority_),
                        0,
                        0.0f,
                        value,
                        nullptr,
                        timestamp_ms_);
}

void TelemetryCollector::addString(const char* field,
                                   const char* value,
                                   Priority priority_override){
  manager_.enqueueValue(registration_,
                        source_,
                        field,
                        ValueType::kString,
                        resolvePriority(priority_override, base_priority_),
                        0,
                        0.0f,
                        false,
                        value,
                        timestamp_ms_);
}

Priority TelemetryCollector::basePriority() const { return base_priority_; }
const SourceRegistration& TelemetryCollector::registration() const { return registration_; }
const void* TelemetryCollector::source() const { return source_; }
uint32_t TelemetryCollector::timestamp() const { return timestamp_ms_; }

::control::IUpdatable* wrapForScheduler(::control::IUpdatable* object){
  if (!object) return nullptr;
  ensureDefaultRegistration(object);
  return LoggingManager::instance().wrap(object);
}

::control::IUpdatable* resolveForDetach(const ::control::IUpdatable* original){
  if (!original) return nullptr;
  ::control::IUpdatable* wrapper = LoggingManager::instance().lookupWrapper(original);
  return wrapper ? wrapper : const_cast<::control::IUpdatable*>(original);
}

void releaseForDetach(const ::control::IUpdatable* original){
  if (!original) return;
  LoggingManager::instance().releaseWrapper(original);
}

void notifySchedulerOverrun(uint32_t now_ms, uint32_t overrun_ms){
  if (!g_scheduler_reg){
    SourceRegistration tmp{};
    tmp.type_name = "scheduler";
    tmp.instance_name = "core";
    tmp.default_priority = Priority::kSystemCritical;
    tmp.allow_background_on_wifi = true;
    tmp.supports_static_dump = false;
    tmp.dynamic_collector = nullptr;
    tmp.static_collector = nullptr;
    g_scheduler_reg = registerSource(&g_scheduler_tag, tmp);
  }
  if (!g_scheduler_reg) return;
  LoggingManager& mgr = LoggingManager::instance();
  TelemetryCollector collector(mgr, *g_scheduler_reg, &g_scheduler_tag, now_ms);
  collector.addInt("overrun_ms", static_cast<int32_t>(overrun_ms), Priority::kSystemCritical);
}

void configureDefaults(){
  LoggingManager& mgr = LoggingManager::instance();
  mgr.enableSerial(true);
  mgr.setSerialBandwidth(BandwidthMode::kNormal);
  mgr.enableWifi(false);
  mgr.setWifiBandwidth(BandwidthMode::kLow);
#ifndef PROBOT_LOGGER_NO_SCHED_ATTACH
  control::attach(&mgr);
#endif
}

void enableSerialLogging(bool enabled){
  LoggingManager::instance().enableSerial(enabled);
}

void enableWifiLogging(bool enabled){
  LoggingManager::instance().enableWifi(enabled);
}

void setSerialBandwidthMode(BandwidthMode mode){
  LoggingManager::instance().setSerialBandwidth(mode);
}

void setWifiBandwidthMode(BandwidthMode mode){
  LoggingManager::instance().setWifiBandwidth(mode);
}

void setWifiSendCallback(WifiSendFn fn){
  LoggingManager::instance().setWifiSendCallbackInternal(fn);
}

void setWifiEndpoint(uint32_t ipv4_be, uint16_t port){
  LoggingManager::instance().setWifiEndpointInternal(ipv4_be, port);
}

void emitStaticSnapshot(const void* source){
  LoggingManager::instance().emitStatic(source);
}

uint32_t wifiEndpointIp(){
  return LoggingManager::instance().wifiEndpointIpInternal();
}

uint16_t wifiEndpointPort(){
  return LoggingManager::instance().wifiEndpointPortInternal();
}

} // namespace probot::logging
