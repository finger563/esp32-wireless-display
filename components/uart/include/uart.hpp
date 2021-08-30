#pragma once

// needed for portTICK_RATE_MS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include <memory>

#include "task.hpp"
#include "console_logger.hpp"

namespace phoenix {
  class Uart {
  public:
    typedef std::function<std::optional<std::string>(const std::string&)> receive_callback_fn;

    struct PinConfig {
      int    tx  = UART_PIN_NO_CHANGE;
      int    rx  = UART_PIN_NO_CHANGE;
      int    rts = UART_PIN_NO_CHANGE;
      int    cts = UART_PIN_NO_CHANGE;
    };

    struct PortConfig {
      size_t baud_rate = 115200;
      uart_word_length_t data_bits = UART_DATA_8_BITS;
      uart_parity_t parity = UART_PARITY_DISABLE;
      uart_stop_bits_t stop_bits = UART_STOP_BITS_1;
      uart_hw_flowcontrol_t flow_control = UART_HW_FLOWCTRL_DISABLE;
      uart_sclk_t source_clk = UART_SCLK_APB;
    };

    struct PatternConfig {
      char   character;
      size_t count = 0;
      size_t timeout = 10;
      size_t post_idle = 0;
      size_t pre_idle = 0;
      size_t max_positions = 0;
    };

    struct Config {
      size_t uart = UART_NUM_1;
      PinConfig pin_config;
      PortConfig port_config;
      PatternConfig pattern_config;
      receive_callback_fn on_receive_callback;
      size_t timeout_ms = 1000;
      size_t receive_buffer_size = 128;
      size_t send_buffer_size = 0;

      bool is_valid() const;
    };

    static std::unique_ptr<Uart> make_unique() {
      return std::unique_ptr<Uart>(new Uart());
    }

    ~Uart();

    bool start(Task::Config& task_config, const Config& uart_config);

  private:
    int uart_ = -1;
    bool running_ = false;
    std::unique_ptr<Task> task_;

    ConsoleLogger logger_;
    const std::string tag_{"Uart"};

    // private so we can only call make_unique
    Uart();
    // deleted so task structures cannot be copied
    Uart(const Uart&) = delete;
    // deleted so task structures cannot be copied
    Uart& operator=(const Uart&) = delete;

    void open(const Config& config);
    void close();
    void task_function(const Config& config);
  };
}
