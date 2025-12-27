#pragma once
#include <stdint.h>
#include <probot/core/scheduler.hpp>

namespace probot::command {

struct ISubsystem : public probot::control::IUpdatable {
  virtual const char* name() const { return "Subsystem"; }
  virtual void periodic(uint32_t now_ms, uint32_t dt_ms) = 0;
  void update(uint32_t now_ms, uint32_t dt_ms) override { periodic(now_ms, dt_ms); }
  virtual ~ISubsystem() {}
};

class SubsystemBase : public ISubsystem {
public:
  explicit SubsystemBase(const char* name = "Subsystem") : name_(name) {}
  const char* name() const override { return name_; }

private:
  const char* name_;
};

} // namespace probot::command
