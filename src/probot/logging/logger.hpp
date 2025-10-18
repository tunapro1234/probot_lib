#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <probot/core/scheduler.hpp>

namespace probot::logging {

  enum class Priority : uint8_t {
    kSystemCritical = 0,
    kUserMarked     = 1,
    kBackground     = 2,
    kUseDefault     = 255
  };

  enum class BandwidthMode : uint8_t {
    kNever  = 0,
    kLow    = 1,
    kNormal = 2,
    kHigh   = 3,
    kFull   = 4
  };

  enum class ValueType : uint8_t {
    kInt,
    kFloat,
    kBool,
    kString
  };

  class TelemetryCollector;
  class LoggingManager;

  using WifiSendFn = bool (*)(const uint8_t* data, size_t len);

  using DynamicCollectorFn = void (*)(const void* object,
                                      TelemetryCollector& collector,
                                      uint32_t now_ms,
                                      uint32_t dt_ms);

  using StaticCollectorFn = void (*)(const void* object,
                                     TelemetryCollector& collector);

  struct SourceRegistration {
    const char*        type_name;
    const char*        instance_name;      // optional, can be nullptr
    Priority           default_priority;
    bool               allow_background_on_wifi;
    bool               supports_static_dump;
    DynamicCollectorFn dynamic_collector;
    StaticCollectorFn  static_collector;
  };

  const SourceRegistration* registerSource(const void* object,
                                           const SourceRegistration& registration);
  void unregisterSource(const void* object);
  const SourceRegistration* findSource(const void* object);

  LoggingManager& manager();
  void enableSerialLogging(bool enabled);
  void enableWifiLogging(bool enabled);
  void setSerialBandwidthMode(BandwidthMode mode);
  void setWifiBandwidthMode(BandwidthMode mode);
  void setWifiSendCallback(WifiSendFn fn);
  void setWifiEndpoint(uint32_t ipv4_be, uint16_t port);
  void emitStaticSnapshot(const void* source);
  uint32_t wifiEndpointIp();
  uint16_t wifiEndpointPort();

  class TelemetryCollector {
  public:
    TelemetryCollector(LoggingManager& manager,
                       const SourceRegistration& registration,
                       const void* source,
                       uint32_t timestamp_ms);

    void addInt(const char* field,
                int32_t value,
                Priority priority_override = Priority::kUseDefault);
    void addFloat(const char* field,
                  float value,
                  Priority priority_override = Priority::kUseDefault);
    void addBool(const char* field,
                 bool value,
                 Priority priority_override = Priority::kUseDefault);
    void addString(const char* field,
                   const char* value,
                   Priority priority_override = Priority::kUseDefault);

    Priority basePriority() const;
    const SourceRegistration& registration() const;
    const void* source() const;
    uint32_t timestamp() const;

  private:
    LoggingManager&           manager_;
    const SourceRegistration& registration_;
    const void*               source_;
    Priority                  base_priority_;
    uint32_t                  timestamp_ms_;
  };

  class LoggingManager : public ::control::IUpdatable {
  public:
    static LoggingManager& instance();

    void update(uint32_t now_ms, uint32_t dt_ms) override;

    void enableSerial(bool enabled);
    void setSerialBandwidth(BandwidthMode mode);
    void enableWifi(bool enabled);
    void setWifiBandwidth(BandwidthMode mode);

    void enqueueValue(const SourceRegistration& reg,
                      const void* source,
                      const char* field,
                      ValueType type,
                      Priority priority,
                      int32_t int_value,
                      float float_value,
                      bool bool_value,
                      const char* str_value,
                      uint32_t timestamp_ms);

    void publishSystemEvent(const char* origin,
                            const char* message,
                            uint32_t timestamp_ms);

    ::control::IUpdatable* wrap(::control::IUpdatable* object);
    ::control::IUpdatable* lookupWrapper(const ::control::IUpdatable* original) const;
    void releaseWrapper(const ::control::IUpdatable* original);

    void onSourceRegistered(const void* object,
                            const SourceRegistration& reg);
    void onSourceUnregistered(const void* object);
    void emitStatic(const void* source);
    void recordDrop(bool serial = true, bool wifi = true);
    void setWifiSendCallbackInternal(WifiSendFn fn);
    void setWifiEndpointInternal(uint32_t ipv4_be, uint16_t port);
    uint32_t wifiEndpointIpInternal() const;
    uint16_t wifiEndpointPortInternal() const;

  private:
    LoggingManager();
    ~LoggingManager();
    LoggingManager(const LoggingManager&) = delete;
    LoggingManager& operator=(const LoggingManager&) = delete;

    struct Impl;
    Impl* impl_;
  };

  ::control::IUpdatable* wrapForScheduler(::control::IUpdatable* object);
  ::control::IUpdatable* resolveForDetach(const ::control::IUpdatable* original);
  void releaseForDetach(const ::control::IUpdatable* original);

  void notifySchedulerOverrun(uint32_t now_ms, uint32_t overrun_ms);

  void configureDefaults();

} // namespace probot::logging
