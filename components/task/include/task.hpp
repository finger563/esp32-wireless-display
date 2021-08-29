#pragma once

#if defined(ESP_PLATFORM)
#include <esp_log.h>
#include <esp_pthread.h>
#endif

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace phoenix {

class Task {

public:

  // NOTE: the majority config is only really used on the ESP
  struct Config {
    std::string name = "default";
    size_t stack_size_bytes = 2048;
    size_t priority = 0; // priority 0 is the lowest priority
    int core_id = -1; // default is not pinned to a core
    std::function<void(void)> on_task_callback = nullptr;
  };

private:
  void task_function(const Config& config);

  std::thread thread_;
  std::mutex mutex_;
  std::string name_;
  bool started_;

  // Private so only make_unique can be used
  Task();
  // deleted so task structures cannot be copied
  Task(const Task&) = delete;

public:

  // create a new task structure and return unique pointer to it
  static std::unique_ptr<Task> make_unique() {
    return std::unique_ptr<Task>(new Task());
  }

  // Stop task if started
  ~Task();

  // Start the task
  bool start(Config& config);

  // Stop the task
  bool stop();

  bool started() { return started_; }
};

}
