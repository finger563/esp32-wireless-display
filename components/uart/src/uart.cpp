#include "uart.hpp"
using namespace phoenix;

// TODO: May want to look into the use of the VFS for more POSIX like
// UART control; see example here:
// https://github.com/espressif/esp-idf/blob/b63ec47238fd6aa6eaa59f7ad3942cbdff5fcc1f/examples/peripherals/uart/uart_select/main/uart_select_example_main.c

Uart::Uart() :
  task_(Task::make_unique()) {
}

Uart::~Uart() {
  running_ = false;
  close();
}

void Uart::open(const Config& config) {
  const uart_config_t uart_config = {
                                     .baud_rate = (int)config.port_config.baud_rate,
                                     .data_bits = config.port_config.data_bits,
                                     .parity = config.port_config.parity,
                                     .stop_bits = config.port_config.stop_bits,
                                     .flow_ctrl = config.port_config.flow_control,
                                     .rx_flow_ctrl_thresh = 0,
                                     .source_clk = config.port_config.source_clk,
  };
  uart_driver_install(config.uart,
                      config.receive_buffer_size * 2,
                      config.send_buffer_size * 2,
                      0, NULL, 0);
  uart_param_config(config.uart, &uart_config);
  uart_set_pin(config.uart,
               config.pin_config.tx,
               config.pin_config.rx,
               config.pin_config.rts,
               config.pin_config.cts);

  // set pattern detect function
  if (config.pattern_config.count > 0) {
    uart_enable_pattern_det_baud_intr(config.uart,
                                      config.pattern_config.character,
                                      config.pattern_config.count,
                                      config.pattern_config.timeout,
                                      config.pattern_config.post_idle,
                                      config.pattern_config.pre_idle);
    if (config.pattern_config.max_positions > 0) {
      // reset the pattern queue length to record at most X pattern positions
      uart_pattern_queue_reset(config.uart, config.pattern_config.max_positions);
    }
  }
}

void Uart::close() {
  logger_.info("{}: closing UART {}", tag_, uart_);
  if (uart_ != -1) {
    // TODO: close UART
    logger_.info("{}: UART closed", tag_);
  }
}

void Uart::task_function(const Config& config) {
  // data for receiving raw bytes
  char receive_buffer[config.receive_buffer_size + 1];
  auto on_receive_callback = config.on_receive_callback;

  // now open the uart port
  open(config);

  while (running_) {
    int len = uart_read_bytes(config.uart, (uint8_t*)receive_buffer, config.receive_buffer_size,
                              config.timeout_ms / portTICK_RATE_MS);
    if (len > 0) {
      // null-terminate the string
      receive_buffer[len] = 0;
      logger_.info("{}: received {} bytes", tag_, len);
      // execute the callback since we've received a message
      std::string message{receive_buffer};
      if (on_receive_callback) {
        auto optional_response = on_receive_callback(message);
        if (optional_response.has_value()) {
          // the callback has returned a response to send
          // back to the client
          auto response = optional_response.value();
          // relay it back to the sender
          logger_.info("{}: sending response '{}' back to client", tag_, response.c_str());
          int sent_len = uart_write_bytes(config.uart, response.c_str(), response.size());
          if (sent_len < 0) {
            logger_.error("{}: error sending response", tag_);
          }
        }
      }
    } else {
      // timeout
      logger_.info("{}: timeout waiting for bytes", tag_);
    }
  }
}

bool Uart::Config::is_valid() const {
  // TODO: check pin config?
  // TODO: warn about default pins (-1 / UART_PIN_NO_CHANGE)?
  // TODO: check port config?
  // TODO: Check baudrate
  if (receive_buffer_size == 0) {
    // it's a size_t so it can't be < 0, but 0 receive buffer is
    // invalid
    return false;
  }
  // TODO: check callback?
  // TODO: check pattern config?
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
