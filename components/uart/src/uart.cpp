#include "uart.hpp"
using namespace phoenix;

Uart::Uart() :
  task_(Task::make_unique()) {
}

Uart::~Uart() {
  running_ = false;
  close();
}

void Uart::close() {
  logger_.info("{}: closing UART {}", tag_, uart_);
  if (uart_ != -1) {
    // TODO: close UART
    logger_.info("{}: UART closed", tag_);
  }
}

void Uart::task_function(const Config& config) {
  while (running_) {
    if (on_receive_callback) {
      auto optional_response = on_receive_callback(message);
      if (optional_response.has_value()) {
        // relay it back to the sender
      }
    }
  }
}

bool Uart::Config::is_valid() const {
  // TODO: add chekcing here
  return true;
}

bool Uart::start(Task::Config& task_config, const Uart::Config& uart_config) {
  if (!uart_config.is_valid()) {
    logger_.error("{}: could not start UART, config is invalid!", tag_);
    return false;
  }
  // set the callback function for the config to be our member function
  task_config.on_task_callback = std::bind(&Uart::task_function, this, uart_config);
  // make sure the uart keeps running
  running_ = true;
  // now start the task
  return task_->start(task_config);
}
