/**
 * LoggingDemo.ino
 *
 * Quick check for the serial/Wi-Fi logging stack.  Registers a demo source,
 * pushes a heartbeat counter every 500 ms, and exposes a static snapshot. Use
 * the driver station UI (minimal stack) to see Wi-Fi log streaming, or watch
 * the binary frames on the serial monitor.
 */

#include <Arduino.h>
#include <probot.h>

namespace {

struct DemoSource {
  const probot::logging::SourceRegistration* reg{nullptr};
  uint32_t boot_ms{0};
  uint32_t counter{0};
} demo;

void demoStaticInfo(const void* object,
                    probot::logging::TelemetryCollector& collector) {
  auto src = static_cast<const DemoSource*>(object);
  collector.addInt("boot_ms", src->boot_ms, probot::logging::Priority::kSystemCritical);
  collector.addString("note", "Demo static snapshot", probot::logging::Priority::kUserMarked);
}

void publishCounter(uint32_t now_ms) {
  if (!demo.reg) return;
  demo.counter++;
  auto& mgr = probot::logging::manager();
  mgr.enqueueValue(*demo.reg,
                   &demo,
                   "counter",
                   probot::logging::ValueType::kInt,
                   probot::logging::Priority::kUserMarked,
                   static_cast<int32_t>(demo.counter),
                   0.0f,
                   false,
                   nullptr,
                   now_ms);
  mgr.enqueueValue(*demo.reg,
                   &demo,
                   "uptime_ms",
                   probot::logging::ValueType::kInt,
                   probot::logging::Priority::kBackground,
                   static_cast<int32_t>(now_ms),
                   0.0f,
                   false,
                   nullptr,
                   now_ms);
}

} // namespace

static uint32_t last_publish_ms = 0;

void robotInit(){
  Serial.begin(115200);
  delay(200);

  probot::logging::configureDefaults();
  probot::logging::enableSerialLogging(true);
  probot::logging::setSerialBandwidthMode(probot::logging::BandwidthMode::kNormal);
  probot::logging::enableWifiLogging(true);
  probot::logging::setWifiStreamingEnabled(true);

  demo.boot_ms = millis();

  probot::logging::SourceRegistration reg{
    "demo",
    "logging",
    probot::logging::Priority::kUserMarked,
    true,
    true,
    nullptr,
    demoStaticInfo
  };
  demo.reg = probot::logging::registerSource(&demo, reg);

  Serial.println();
  Serial.println(F("[LoggingDemo] Logging configured. Check the driver UI"));
}

void robotEnd(){
  if (demo.reg){
    probot::logging::unregisterSource(&demo);
    demo.reg = nullptr;
  }
}

void teleopInit(){
  last_publish_ms = millis();
}

void teleopLoop(){
  uint32_t now = millis();
  if (now - last_publish_ms >= 500){
    publishCounter(now);
    last_publish_ms = now;
  }
  vTaskDelay(pdMS_TO_TICKS(5));
}

void autonomousInit(){
  teleopInit();
}

void autonomousLoop(){
  teleopLoop();
}
