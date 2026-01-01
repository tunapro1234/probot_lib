#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <probot/command/command.hpp>
#include <probot/core/scheduler_registry.hpp>
#include <probot/robot/state.hpp>

#ifdef ESP32
#include <esp_idf_version.h>
#include <esp_task_wdt.h>
#endif

namespace probot::command {

// ============================================================================
// Scheduler - WPILib-style command scheduler
// ============================================================================

class Scheduler {
public:
  // Singleton instance
  static Scheduler& instance() {
    static Scheduler inst;
    return inst;
  }

  // -------------------------------------------------------------------------
  // Command Management
  // -------------------------------------------------------------------------

  // Schedule a command for execution
  bool schedule(ICommand* cmd) {
    if (!cmd || !queue_) return false;
    Request req{RequestType::kSchedule, cmd, nullptr};
    return xQueueSend(queue_, &req, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  // Cancel a running command
  bool cancel(ICommand* cmd) {
    if (!cmd || !queue_) return false;
    Request req{RequestType::kCancel, cmd, nullptr};
    return xQueueSend(queue_, &req, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  // Cancel all running commands
  bool cancelAll() {
    if (!queue_) return false;
    Request req{RequestType::kCancelAll, nullptr, nullptr};
    return xQueueSend(queue_, &req, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  // -------------------------------------------------------------------------
  // Subsystem Management
  // -------------------------------------------------------------------------

  // Register a subsystem with the scheduler
  bool registerSubsystem(ISubsystem* sub) {
    if (!sub || !queue_) return false;
    Request req{RequestType::kRegisterSubsystem, nullptr, sub};
    return xQueueSend(queue_, &req, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  // Unregister a subsystem
  bool unregisterSubsystem(ISubsystem* sub) {
    if (!sub || !queue_) return false;
    Request req{RequestType::kUnregisterSubsystem, nullptr, sub};
    return xQueueSend(queue_, &req, pdMS_TO_TICKS(10)) == pdTRUE;
  }

  // Set default command for a subsystem
  bool setDefaultCommand(ISubsystem* sub, ICommand* cmd) {
    if (!sub) return false;
    sub->setDefaultCommand(cmd);
    return true;
  }

  // -------------------------------------------------------------------------
  // Query Methods (thread-safe reads)
  // -------------------------------------------------------------------------

  // Check if a command is scheduled
  bool isScheduled(ICommand* cmd) const {
    for (size_t i = 0; i < kMaxCommands; ++i) {
      if (commands_[i].cmd == cmd && commands_[i].state != CommandState::kNone) {
        return true;
      }
    }
    return false;
  }

  // Get the command currently using a subsystem
  ICommand* requiring(ISubsystem* sub) const {
    if (!sub) return nullptr;
    return sub->getCurrentCommand();
  }

  // Get scheduled command count
  size_t commandCount() const {
    size_t count = 0;
    for (size_t i = 0; i < kMaxCommands; ++i) {
      if (commands_[i].state != CommandState::kNone) ++count;
    }
    return count;
  }

  // Get registered subsystem count
  size_t subsystemCount() const {
    size_t count = 0;
    for (size_t i = 0; i < kMaxSubsystems; ++i) {
      if (subsystems_[i] != nullptr) ++count;
    }
    return count;
  }

  // -------------------------------------------------------------------------
  // Configuration
  // -------------------------------------------------------------------------

  void setPeriodMs(uint32_t ms) {
    if (ms > 0) period_ms_ = ms;
  }

  uint32_t periodMs() const { return period_ms_; }

  // -------------------------------------------------------------------------
  // Backward Compatibility (eski API)
  // -------------------------------------------------------------------------

  // attach() artik registerSubsystem() ve schedule() kombinasyonu
  bool attach(ISubsystem* sub) { return registerSubsystem(sub); }
  bool attach(ICommand* cmd) { return schedule(cmd); }
  bool detach(ISubsystem* sub) { return unregisterSubsystem(sub); }
  bool detach(ICommand* cmd) { return cancel(cmd); }

private:
  // -------------------------------------------------------------------------
  // Internal Types
  // -------------------------------------------------------------------------

  enum class RequestType : uint8_t {
    kSchedule,
    kCancel,
    kCancelAll,
    kRegisterSubsystem,
    kUnregisterSubsystem
  };

  struct Request {
    RequestType type;
    ICommand* cmd;
    ISubsystem* sub;
  };

  enum class CommandState : uint8_t {
    kNone = 0,
    kScheduled,   // Henuz initialize edilmedi
    kRunning,     // initialize() cagrildi, execute() calisiyor
    kEnding       // isFinished() true, end() cagrilacak
  };

  struct CommandSlot {
    ICommand* cmd = nullptr;
    CommandState state = CommandState::kNone;
    uint32_t scheduled_at = 0;
    uint32_t last_run_ms = 0;
  };

  // -------------------------------------------------------------------------
  // Constants
  // -------------------------------------------------------------------------

  static constexpr size_t kMaxCommands = 64;
  static constexpr size_t kMaxSubsystems = 32;
  static constexpr size_t kQueueSize = 16;

  // -------------------------------------------------------------------------
  // State
  // -------------------------------------------------------------------------

  QueueHandle_t queue_ = nullptr;
  uint32_t period_ms_ = 20;
  CommandSlot commands_[kMaxCommands] = {};
  ISubsystem* subsystems_[kMaxSubsystems] = {};
  probot::robot::Status last_status_ = probot::robot::Status::STOP;
  probot::robot::Phase last_phase_ = probot::robot::Phase::NOT_INIT;

  // -------------------------------------------------------------------------
  // Private Constructor (Singleton)
  // -------------------------------------------------------------------------

  Scheduler() = default;
  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;

  // -------------------------------------------------------------------------
  // Internal Methods
  // -------------------------------------------------------------------------

  void init() {
    if (!queue_) {
      queue_ = xQueueCreate(kQueueSize, sizeof(Request));
    }
  }

  // Find slot for a command
  int findCommandSlot(ICommand* cmd) const {
    for (size_t i = 0; i < kMaxCommands; ++i) {
      if (commands_[i].cmd == cmd && commands_[i].state != CommandState::kNone) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  // Find free command slot
  int findFreeCommandSlot() const {
    for (size_t i = 0; i < kMaxCommands; ++i) {
      if (commands_[i].state == CommandState::kNone) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  // Find subsystem slot
  int findSubsystemSlot(ISubsystem* sub) const {
    for (size_t i = 0; i < kMaxSubsystems; ++i) {
      if (subsystems_[i] == sub) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  // Find free subsystem slot
  int findFreeSubsystemSlot() const {
    for (size_t i = 0; i < kMaxSubsystems; ++i) {
      if (subsystems_[i] == nullptr) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  // Cancel a command internally (called from scheduler task)
  void cancelCommand(int idx, bool interrupted = true) {
    if (idx < 0 || idx >= (int)kMaxCommands) return;
    auto& slot = commands_[idx];
    if (slot.state == CommandState::kNone || !slot.cmd) return;

    // Clear subsystem current command references
    for (size_t r = 0; r < slot.cmd->requirementCount(); ++r) {
      ISubsystem* sub = slot.cmd->requirementAt(r);
      if (sub && sub->getCurrentCommand() == slot.cmd) {
        sub->setCurrentCommand(nullptr);
      }
    }

    // Call end if was running
    if (slot.state == CommandState::kRunning) {
      slot.cmd->end(interrupted);
    }

#ifndef PROBOT_SCHED_NOLOG
    Serial.printf("[SCHED] cancel: %s (interrupted=%d)\n", slot.cmd->name(), interrupted);
#endif

    slot.cmd = nullptr;
    slot.state = CommandState::kNone;
  }

  // Try to schedule a command (conflict resolution)
  void tryScheduleCommand(ICommand* cmd) {
    if (!cmd) return;

    // Already scheduled?
    if (findCommandSlot(cmd) >= 0) {
#ifndef PROBOT_SCHED_NOLOG
      Serial.printf("[SCHED] already scheduled: %s\n", cmd->name());
#endif
      return;
    }

    // Check for conflicts with requirements
    for (size_t r = 0; r < cmd->requirementCount(); ++r) {
      ISubsystem* sub = cmd->requirementAt(r);
      if (!sub) continue;

      ICommand* current = sub->getCurrentCommand();
      if (current && current != cmd) {
        // Conflict! Check interrupt behavior
        if (current->interruptBehavior() == InterruptBehavior::kCancelIncoming) {
#ifndef PROBOT_SCHED_NOLOG
          Serial.printf("[SCHED] rejected: %s (blocked by %s)\n", cmd->name(), current->name());
#endif
          return; // Reject incoming command
        }
        // Cancel the current command
        int idx = findCommandSlot(current);
        if (idx >= 0) {
          cancelCommand(idx, true);
        }
      }
    }

    // Find free slot
    int idx = findFreeCommandSlot();
    if (idx < 0) {
#ifndef PROBOT_SCHED_NOLOG
      Serial.println("[SCHED] no free slot for command");
#endif
      return;
    }

    // Schedule the command
    auto& slot = commands_[idx];
    slot.cmd = cmd;
    slot.state = CommandState::kScheduled;
    slot.scheduled_at = millis();
    slot.last_run_ms = slot.scheduled_at;

    // Set subsystem current command references
    for (size_t r = 0; r < cmd->requirementCount(); ++r) {
      ISubsystem* sub = cmd->requirementAt(r);
      if (sub) sub->setCurrentCommand(cmd);
    }

#ifndef PROBOT_SCHED_NOLOG
    Serial.printf("[SCHED] scheduled: %s (slot=%d)\n", cmd->name(), idx);
#endif
  }

  // Process incoming requests
  void processRequests() {
    Request req;
    while (xQueueReceive(queue_, &req, 0) == pdTRUE) {
      switch (req.type) {
        case RequestType::kSchedule:
          tryScheduleCommand(req.cmd);
          break;

        case RequestType::kCancel: {
          int idx = findCommandSlot(req.cmd);
          if (idx >= 0) cancelCommand(idx, true);
          break;
        }

        case RequestType::kCancelAll:
          for (size_t i = 0; i < kMaxCommands; ++i) {
            cancelCommand(static_cast<int>(i), true);
          }
          break;

        case RequestType::kRegisterSubsystem: {
          if (findSubsystemSlot(req.sub) >= 0) break; // Already registered
          int idx = findFreeSubsystemSlot();
          if (idx >= 0) {
            subsystems_[idx] = req.sub;
#ifndef PROBOT_SCHED_NOLOG
            Serial.printf("[SCHED] registered: %s\n", req.sub->name());
#endif
          }
          break;
        }

        case RequestType::kUnregisterSubsystem: {
          int idx = findSubsystemSlot(req.sub);
          if (idx >= 0) {
            // Cancel any command using this subsystem
            for (size_t i = 0; i < kMaxCommands; ++i) {
              if (commands_[i].state != CommandState::kNone && commands_[i].cmd) {
                for (size_t r = 0; r < commands_[i].cmd->requirementCount(); ++r) {
                  if (commands_[i].cmd->requirementAt(r) == req.sub) {
                    cancelCommand(static_cast<int>(i), true);
                    break;
                  }
                }
              }
            }
            req.sub->setCurrentCommand(nullptr);
            subsystems_[idx] = nullptr;
#ifndef PROBOT_SCHED_NOLOG
            Serial.printf("[SCHED] unregistered: %s\n", req.sub->name());
#endif
          }
          break;
        }
      }
    }
  }

  // Schedule default commands for idle subsystems
  void scheduleDefaultCommands() {
    for (size_t i = 0; i < kMaxSubsystems; ++i) {
      ISubsystem* sub = subsystems_[i];
      if (!sub) continue;

      // If subsystem has no current command, schedule its default
      if (sub->getCurrentCommand() == nullptr) {
        ICommand* def = sub->getDefaultCommand();
        if (def && findCommandSlot(def) < 0) {
          // Reset the command before scheduling
          def->reset();
          tryScheduleCommand(def);
        }
      }
    }
  }

  // Run all subsystem periodic methods
  void runSubsystems(uint32_t now, uint32_t dt) {
    for (size_t i = 0; i < kMaxSubsystems; ++i) {
      if (subsystems_[i]) {
        subsystems_[i]->periodic(now, dt);
      }
    }
  }

  // Run all scheduled commands
  void runCommands(uint32_t now, bool allow_updates) {
    for (size_t i = 0; i < kMaxCommands; ++i) {
      auto& slot = commands_[i];
      if (slot.state == CommandState::kNone || !slot.cmd) continue;

      // Check runsWhenDisabled
      if (!allow_updates && !slot.cmd->runsWhenDisabled()) {
        continue;
      }

      uint32_t dt = now - slot.last_run_ms;
      slot.last_run_ms = now;

      switch (slot.state) {
        case CommandState::kScheduled:
          // Initialize and start running
          slot.cmd->initialize();
          slot.state = CommandState::kRunning;
#ifndef PROBOT_SCHED_NOLOG
          Serial.printf("[SCHED] init: %s\n", slot.cmd->name());
#endif
          // Fall through to execute
          [[fallthrough]];

        case CommandState::kRunning:
          slot.cmd->execute(now, dt);
          if (slot.cmd->isFinished()) {
            slot.state = CommandState::kEnding;
          }
          break;

        case CommandState::kEnding:
          // Clear subsystem references
          for (size_t r = 0; r < slot.cmd->requirementCount(); ++r) {
            ISubsystem* sub = slot.cmd->requirementAt(r);
            if (sub && sub->getCurrentCommand() == slot.cmd) {
              sub->setCurrentCommand(nullptr);
            }
          }
          slot.cmd->end(false);
#ifndef PROBOT_SCHED_NOLOG
          Serial.printf("[SCHED] finished: %s\n", slot.cmd->name());
#endif
          slot.cmd = nullptr;
          slot.state = CommandState::kNone;
          break;

        default:
          break;
      }
    }
  }

  // Handle phase/status transitions
  void handleTransitions(probot::robot::Status status, probot::robot::Phase phase) {
    bool to_stop = (last_status_ != probot::robot::Status::STOP &&
                    status == probot::robot::Status::STOP);
    bool auto_to_teleop = (last_phase_ == probot::robot::Phase::AUTONOMOUS &&
                           phase == probot::robot::Phase::TELEOP);

    if (to_stop || auto_to_teleop) {
      // Cancel all commands on transition
      for (size_t i = 0; i < kMaxCommands; ++i) {
        if (commands_[i].state != CommandState::kNone && commands_[i].cmd) {
          commands_[i].cmd->onSchedulerStop();
          cancelCommand(static_cast<int>(i), true);
        }
      }
    }

    last_status_ = status;
    last_phase_ = phase;
  }

  // -------------------------------------------------------------------------
  // Main Task Entry Point
  // -------------------------------------------------------------------------

  static void taskEntry(void*) {
    Scheduler::instance().taskLoop();
  }

  void taskLoop() {
#ifndef PROBOT_SCHED_NOLOG
    Serial.printf("[SCHED] start on core %d (prio=%u, period=%lums)\n",
                  xPortGetCoreID(), (unsigned)uxTaskPriorityGet(NULL),
                  (unsigned long)period_ms_);
#endif

#if defined(ESP32) && ESP_IDF_VERSION_MAJOR >= 5
    esp_task_wdt_add(NULL);
#endif

    uint32_t last_run = millis();

    for (;;) {
      uint32_t now = millis();
      uint32_t dt = now - last_run;

      // Read robot state
      auto snap = probot::robot::state().read();
      bool allow_updates = (snap.status != probot::robot::Status::STOP);

      // Handle state transitions
      handleTransitions(snap.status, snap.phase);

      // Process queue requests
      processRequests();

      // Run subsystems
      runSubsystems(now, dt);

      // Run commands
      runCommands(now, allow_updates);

      // Schedule default commands for idle subsystems
      if (allow_updates) {
        scheduleDefaultCommands();
      }

      last_run = now;

#if defined(ESP32) && ESP_IDF_VERSION_MAJOR >= 5
      esp_task_wdt_reset();
#endif

      // Sleep until next period
      vTaskDelay(pdMS_TO_TICKS(period_ms_));
    }
  }

  // -------------------------------------------------------------------------
  // Static Registration (auto-register with runtime)
  // -------------------------------------------------------------------------

  static void staticInit(uint8_t) {
    Scheduler::instance().init();
  }

  static void staticTask(void* param) {
    Scheduler::taskEntry(param);
  }

  struct Registrar {
    Registrar() {
      probot::detail::g_scheduler_init = &Scheduler::staticInit;
      probot::detail::g_scheduler_task = &Scheduler::staticTask;
    }
  };

  static inline Registrar registrar_{};
};

// ============================================================================
// Convenience namespace functions (backward compatibility)
// ============================================================================

namespace scheduler {
  inline void init(uint8_t = 8) { Scheduler::instance(); }
  inline bool attach(ISubsystem* sub) { return Scheduler::instance().registerSubsystem(sub); }
  inline bool attach(ICommand* cmd) { return Scheduler::instance().schedule(cmd); }
  inline bool detach(ISubsystem* sub) { return Scheduler::instance().unregisterSubsystem(sub); }
  inline bool detach(ICommand* cmd) { return Scheduler::instance().cancel(cmd); }
  inline void setGlobalPeriodMs(uint32_t ms) { Scheduler::instance().setPeriodMs(ms); }
  inline size_t count() { return Scheduler::instance().commandCount() + Scheduler::instance().subsystemCount(); }
}

} // namespace probot::command
