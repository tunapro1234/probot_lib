#pragma once
#include <stdint.h>

namespace probot::command {

// Forward declaration
struct ICommand;

struct ISubsystem {
  virtual const char* name() const { return "Subsystem"; }
  virtual void periodic(uint32_t now_ms, uint32_t dt_ms) = 0;

  // Default command - scheduler bosta kalinca calistirir
  virtual ICommand* getDefaultCommand() const { return nullptr; }
  virtual void setDefaultCommand(ICommand*) {}

  // Current command - scheduler tarafindan set edilir
  virtual ICommand* getCurrentCommand() const { return nullptr; }
  virtual void setCurrentCommand(ICommand*) {}

  virtual ~ISubsystem() = default;
};

class SubsystemBase : public ISubsystem {
public:
  explicit SubsystemBase(const char* name = "Subsystem") : name_(name) {}

  const char* name() const override { return name_; }

  ICommand* getDefaultCommand() const override { return default_command_; }
  void setDefaultCommand(ICommand* cmd) override { default_command_ = cmd; }

  ICommand* getCurrentCommand() const override { return current_command_; }
  void setCurrentCommand(ICommand* cmd) override { current_command_ = cmd; }

private:
  const char* name_;
  ICommand* default_command_ = nullptr;
  ICommand* current_command_ = nullptr;
};

} // namespace probot::command
