#pragma once
#include <stddef.h>
#include <stdint.h>
#include <probot/command/subsystem.hpp>

namespace probot::command {

// Command interrupt davranisi
enum class InterruptBehavior : uint8_t {
  kCancelSelf,      // Yeni command gelince kendini cancel et (default)
  kCancelIncoming   // Yeni command'i reddet
};

struct ICommand {
  virtual const char* name() const { return "Command"; }
  virtual void initialize() {}
  virtual void execute(uint32_t now_ms, uint32_t dt_ms) = 0;
  virtual void end(bool interrupted) { (void)interrupted; }
  virtual bool isFinished() const = 0;
  virtual void reset() {}  // Reset command state for re-scheduling

  // Convenience wrapper for manual/legacy usage - NOT called by scheduler
  // Scheduler uses state machine (initialize/execute/end/isFinished)
  virtual void periodic(uint32_t, uint32_t) {}

  // Subsystem requirements
  virtual bool addRequirement(ISubsystem* subsystem) { (void)subsystem; return false; }
  virtual size_t requirementCount() const { return 0; }
  virtual ISubsystem* requirementAt(size_t idx) const { (void)idx; return nullptr; }

  // WPILib-style ozellikler
  virtual bool runsWhenDisabled() const { return false; }
  virtual InterruptBehavior interruptBehavior() const { return InterruptBehavior::kCancelSelf; }

  virtual ~ICommand() = default;
};

class CommandBase : public ICommand {
public:
  explicit CommandBase(const char* name = "Command") : name_(name) {}

  const char* name() const override { return name_; }

  void periodic(uint32_t now_ms, uint32_t dt_ms) override {
    if (finished_) return;
    if (!initialized_){
      initialize();
      initialized_ = true;
    }
    execute(now_ms, dt_ms);
    if (isFinished()){
      end(false);
      finished_ = true;
    }
  }

  void reset() override {
    initialized_ = false;
    finished_ = false;
  }

  void cancel(){
    if (!finished_){
      end(true);
      finished_ = true;
    }
  }

  bool isInitialized() const { return initialized_; }
  bool isCompleted() const { return finished_; }

  // Requirement yonetimi
  bool addRequirement(ISubsystem* subsystem) override {
    if (!subsystem) return false;
    for (size_t i = 0; i < requirement_count_; ++i){
      if (requirements_[i] == subsystem) return true;
    }
    if (requirement_count_ >= kMaxRequirements) return false;
    requirements_[requirement_count_++] = subsystem;
    return true;
  }

  size_t requirementCount() const override { return requirement_count_; }

  ISubsystem* requirementAt(size_t idx) const override {
    if (idx >= requirement_count_) return nullptr;
    return requirements_[idx];
  }

  // WPILib-style yapilandirma
  bool runsWhenDisabled() const override { return runs_when_disabled_; }
  void setRunsWhenDisabled(bool value) { runs_when_disabled_ = value; }

  InterruptBehavior interruptBehavior() const override { return interrupt_behavior_; }
  void setInterruptBehavior(InterruptBehavior behavior) { interrupt_behavior_ = behavior; }

protected:
  // Alt siniflar icin kolaylik metodlari
  CommandBase& withRequirement(ISubsystem* sub) {
    addRequirement(sub);
    return *this;
  }

private:
  static constexpr size_t kMaxRequirements = 16;
  const char* name_;
  bool initialized_ = false;
  bool finished_ = false;
  bool runs_when_disabled_ = false;
  InterruptBehavior interrupt_behavior_ = InterruptBehavior::kCancelSelf;
  ISubsystem* requirements_[kMaxRequirements] = {};
  size_t requirement_count_ = 0;
};

} // namespace probot::command
