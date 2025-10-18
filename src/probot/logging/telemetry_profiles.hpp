#pragma once
#include <stdint.h>
#include <probot/logging/logger.hpp>

namespace probot::logging::profiles {

  void motorControllerDynamic(const void* object,
                              TelemetryCollector& collector,
                              uint32_t now_ms,
                              uint32_t dt_ms);

  void motorControllerStatic(const void* object,
                             TelemetryCollector& collector);

  void sliderDynamic(const void* object,
                     TelemetryCollector& collector,
                     uint32_t now_ms,
                     uint32_t dt_ms);

  void armDynamic(const void* object,
                  TelemetryCollector& collector,
                  uint32_t now_ms,
                  uint32_t dt_ms);

  void elevatorDynamic(const void* object,
                       TelemetryCollector& collector,
                       uint32_t now_ms,
                       uint32_t dt_ms);

  void telescopicDynamic(const void* object,
                         TelemetryCollector& collector,
                         uint32_t now_ms,
                         uint32_t dt_ms);

  void turretDynamic(const void* object,
                     TelemetryCollector& collector,
                     uint32_t now_ms,
                     uint32_t dt_ms);

  void tankDriveDynamic(const void* object,
                        TelemetryCollector& collector,
                        uint32_t now_ms,
                        uint32_t dt_ms);

  void blinkPidDynamic(const void* object,
                       TelemetryCollector& collector,
                       uint32_t now_ms,
                       uint32_t dt_ms);

} // namespace probot::logging::profiles
