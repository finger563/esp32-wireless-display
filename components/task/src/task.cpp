#include "task.hpp"
using namespace phoenix;

static const char *TAG = "Task";

Task::Task() :
  name_(""),
  started_(false) {
}

Task::~Task() {
  if (started_) {
    stop();
  }
}

bool Task::start(Task::Config &config) {
  if (started_) {
    ESP_LOGE(TAG, "Task '%s' already started", name_.c_str());
    return false;
  }
  name_ = config.name;
  ESP_LOGI(TAG, "Starting task: '%s'", name_.c_str());

#if defined(ESP_PLATFORM)
  // set the stack size / priority for the ESP32 scheduler
  auto thread_config = esp_pthread_get_default_config();
  thread_config.thread_name = config.name.c_str();
  if (config.core_id > 0)
    thread_config.pin_to_core = config.core_id;
  thread_config.stack_size = config.stack_size_bytes;
  thread_config.prio = config.priority;
  esp_pthread_set_cfg(&thread_config);
#endif

  // make sure the thread knows to start - shouldn't need a mutex here
  // because we haven't started the thread.
  started_ = true;

  // now actually start the thread
  thread_ = std::thread(&Task::task_function, this, config);

  return started_;
}

bool Task::stop() {
  if (!started_) {
    ESP_LOGE(TAG, "Task '%s' cannot be stopped - it is not running.", name_.c_str());
    return false;
  }

  // clear the started flag - this will cause the loop to break in the
  // task_function
  {
    std::unique_lock<std::mutex> lock{mutex_};
    started_ = false;
  }

  // actually stop the thread
  ESP_LOGI(TAG, "Stopping task: '%s'", name_.c_str());
  bool result{false};
  if (thread_.joinable()) {
    thread_.join();
    result = true;
  }

  return result;
}

void Task::task_function(const Task::Config& config) {
  std::string name{config.name};
  size_t stack_size_bytes = config.stack_size_bytes;
  size_t priority = config.priority;
  auto on_task_callback = config.on_task_callback;
  ESP_LOGI(TAG, "Running task function: '%s' - stack size: %d bytes, priority: %d",
           name.c_str(), stack_size_bytes, priority);
  // now actually run the task
  while (true) {
    // check whether we should stop or not
    {
      std::unique_lock<std::mutex> lock{mutex_};
      if (!started_) {
        break;
      }
    }
    // repeatedly invoke the callback. NOTE: the callback must
    // internally call std::this_thread::sleep_for(...) otherwise it
    // will consume the processor
    if (on_task_callback) {
      on_task_callback();
    } else {
      break;
    }
  }
  ESP_LOGI(TAG, "task function '%s' stopping", name.c_str());
}
