#pragma once
#include <stdint.h>
#include <stddef.h>
#include <probot/command/command.hpp>

namespace probot::command {

// ============================================================================
// SequentialCommandGroup - Komutlari sirayla calistirir
// ============================================================================

class SequentialCommandGroup : public CommandBase {
public:
  explicit SequentialCommandGroup(const char* name = "SequentialGroup")
    : CommandBase(name) {}

  // Fluent API ile command ekleme
  SequentialCommandGroup& addCommand(ICommand* cmd) {
    if (cmd && command_count_ < kMaxCommands) {
      commands_[command_count_++] = cmd;
      // Merge requirements
      for (size_t r = 0; r < cmd->requirementCount(); ++r) {
        addRequirement(cmd->requirementAt(r));
      }
    }
    return *this;
  }

  // Birden fazla command ekleme
  template<typename... Commands>
  SequentialCommandGroup& addCommands(Commands*... cmds) {
    (addCommand(cmds), ...);
    return *this;
  }

  void initialize() override {
    current_index_ = 0;
    for (size_t i = 0; i < command_count_; ++i) {
      if (commands_[i]) {
        commands_[i]->reset();
      }
    }
    if (command_count_ > 0 && commands_[0]) {
      commands_[0]->initialize();
    }
  }

  void execute(uint32_t now_ms, uint32_t dt_ms) override {
    if (current_index_ >= command_count_) return;

    ICommand* current = commands_[current_index_];
    if (!current) {
      ++current_index_;
      return;
    }

    current->execute(now_ms, dt_ms);

    if (current->isFinished()) {
      current->end(false);
      ++current_index_;

      // Initialize next command
      if (current_index_ < command_count_ && commands_[current_index_]) {
        commands_[current_index_]->initialize();
      }
    }
  }

  void end(bool interrupted) override {
    // End current command if interrupted
    if (interrupted && current_index_ < command_count_) {
      ICommand* current = commands_[current_index_];
      if (current) {
        current->end(true);
      }
    }
  }

  bool isFinished() const override {
    return current_index_ >= command_count_;
  }

  size_t commandCount() const { return command_count_; }
  size_t currentIndex() const { return current_index_; }

private:
  static constexpr size_t kMaxCommands = 16;
  ICommand* commands_[kMaxCommands] = {};
  size_t command_count_ = 0;
  size_t current_index_ = 0;
};

// ============================================================================
// ParallelCommandGroup - Komutlari paralel calistirir
// ============================================================================

class ParallelCommandGroup : public CommandBase {
public:
  enum class EndCondition : uint8_t {
    kAll,   // Tumu bitince grup biter
    kAny,   // Herhangi biri bitince grup biter
    kRace   // Ilk biten kazanir, digerler cancel edilir
  };

  explicit ParallelCommandGroup(const char* name = "ParallelGroup")
    : CommandBase(name) {}

  // Fluent API ile command ekleme
  ParallelCommandGroup& addCommand(ICommand* cmd) {
    if (cmd && command_count_ < kMaxCommands) {
      commands_[command_count_] = cmd;
      finished_[command_count_] = false;
      ++command_count_;
      // Merge requirements
      for (size_t r = 0; r < cmd->requirementCount(); ++r) {
        addRequirement(cmd->requirementAt(r));
      }
    }
    return *this;
  }

  // Birden fazla command ekleme
  template<typename... Commands>
  ParallelCommandGroup& addCommands(Commands*... cmds) {
    (addCommand(cmds), ...);
    return *this;
  }

  ParallelCommandGroup& withEndCondition(EndCondition cond) {
    end_condition_ = cond;
    return *this;
  }

  void initialize() override {
    for (size_t i = 0; i < command_count_; ++i) {
      finished_[i] = false;
      if (commands_[i]) {
        commands_[i]->reset();
        commands_[i]->initialize();
      }
    }
  }

  void execute(uint32_t now_ms, uint32_t dt_ms) override {
    for (size_t i = 0; i < command_count_; ++i) {
      if (finished_[i] || !commands_[i]) continue;

      commands_[i]->execute(now_ms, dt_ms);

      if (commands_[i]->isFinished()) {
        commands_[i]->end(false);
        finished_[i] = true;

        // Race condition: cancel others
        if (end_condition_ == EndCondition::kRace) {
          for (size_t j = 0; j < command_count_; ++j) {
            if (!finished_[j] && commands_[j]) {
              commands_[j]->end(true);
              finished_[j] = true;
            }
          }
        }
      }
    }
  }

  void end(bool interrupted) override {
    if (interrupted) {
      for (size_t i = 0; i < command_count_; ++i) {
        if (!finished_[i] && commands_[i]) {
          commands_[i]->end(true);
          finished_[i] = true;
        }
      }
    }
  }

  bool isFinished() const override {
    switch (end_condition_) {
      case EndCondition::kAll:
        // Tumu bitmis olmali
        for (size_t i = 0; i < command_count_; ++i) {
          if (!finished_[i]) return false;
        }
        return command_count_ > 0;

      case EndCondition::kAny:
      case EndCondition::kRace:
        // Herhangi biri bitmis olmali
        for (size_t i = 0; i < command_count_; ++i) {
          if (finished_[i]) return true;
        }
        return command_count_ == 0;
    }
    return true;
  }

  size_t commandCount() const { return command_count_; }
  EndCondition endCondition() const { return end_condition_; }

private:
  static constexpr size_t kMaxCommands = 8;
  ICommand* commands_[kMaxCommands] = {};
  bool finished_[kMaxCommands] = {};
  size_t command_count_ = 0;
  EndCondition end_condition_ = EndCondition::kAll;
};

// ============================================================================
// WaitCommand - Belirli sure bekler
// ============================================================================

class WaitCommand : public CommandBase {
public:
  explicit WaitCommand(uint32_t duration_ms, const char* name = "Wait")
    : CommandBase(name), duration_ms_(duration_ms) {}

  void initialize() override {
    start_time_ = 0;
  }

  void execute(uint32_t now_ms, uint32_t) override {
    if (start_time_ == 0) {
      start_time_ = now_ms;
    }
  }

  bool isFinished() const override {
    if (start_time_ == 0) return false;
    return (millis() - start_time_) >= duration_ms_;
  }

  void setDuration(uint32_t ms) { duration_ms_ = ms; }
  uint32_t duration() const { return duration_ms_; }

private:
  uint32_t duration_ms_;
  uint32_t start_time_ = 0;
};

// ============================================================================
// InstantCommand - Tek seferlik islem yapar
// ============================================================================

class InstantCommand : public CommandBase {
public:
  using Action = void(*)();

  explicit InstantCommand(Action action, const char* name = "Instant")
    : CommandBase(name), action_(action) {}

  void initialize() override {
    if (action_) {
      action_();
    }
  }

  void execute(uint32_t, uint32_t) override {}

  bool isFinished() const override {
    return true;
  }

  void setAction(Action action) { action_ = action; }

private:
  Action action_ = nullptr;
};

// ============================================================================
// RunCommand - Surekli bir fonksiyon calistirir (hic bitmez)
// ============================================================================

class RunCommand : public CommandBase {
public:
  using Action = void(*)(uint32_t now_ms, uint32_t dt_ms);

  explicit RunCommand(Action action, const char* name = "Run")
    : CommandBase(name), action_(action) {}

  void execute(uint32_t now_ms, uint32_t dt_ms) override {
    if (action_) {
      action_(now_ms, dt_ms);
    }
  }

  bool isFinished() const override {
    return false; // Never finishes
  }

  void setAction(Action action) { action_ = action; }

private:
  Action action_ = nullptr;
};

// ============================================================================
// FunctionalCommand - Lambda/function pointer ile olusturulan command
// ============================================================================

class FunctionalCommand : public CommandBase {
public:
  using InitFunc = void(*)();
  using ExecuteFunc = void(*)(uint32_t now_ms, uint32_t dt_ms);
  using EndFunc = void(*)(bool interrupted);
  using IsFinishedFunc = bool(*)();

  FunctionalCommand(
    InitFunc init_func,
    ExecuteFunc execute_func,
    EndFunc end_func,
    IsFinishedFunc is_finished_func,
    const char* name = "Functional"
  ) : CommandBase(name),
      init_func_(init_func),
      execute_func_(execute_func),
      end_func_(end_func),
      is_finished_func_(is_finished_func) {}

  void initialize() override {
    if (init_func_) init_func_();
  }

  void execute(uint32_t now_ms, uint32_t dt_ms) override {
    if (execute_func_) execute_func_(now_ms, dt_ms);
  }

  void end(bool interrupted) override {
    if (end_func_) end_func_(interrupted);
  }

  bool isFinished() const override {
    if (is_finished_func_) return is_finished_func_();
    return false;
  }

private:
  InitFunc init_func_ = nullptr;
  ExecuteFunc execute_func_ = nullptr;
  EndFunc end_func_ = nullptr;
  IsFinishedFunc is_finished_func_ = nullptr;
};

// ============================================================================
// ConditionalCommand - Kosul saglaninca biter
// ============================================================================

class ConditionalCommand : public CommandBase {
public:
  using Condition = bool(*)();

  explicit ConditionalCommand(Condition condition, const char* name = "Conditional")
    : CommandBase(name), condition_(condition) {}

  void execute(uint32_t, uint32_t) override {}

  bool isFinished() const override {
    if (condition_) return condition_();
    return true;
  }

  void setCondition(Condition cond) { condition_ = cond; }

private:
  Condition condition_ = nullptr;
};

} // namespace probot::command
