#pragma once

#include "esp_system.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "task.hpp"
#include "console_logger.hpp"

namespace phoenix {
  class Uart {
  public:
    typedef std::function<std::optional<std::string>(const std::string&)> receive_callback_fn;

    struct Config {
      size_t uart = UART_NUM_1;
      size_t baud_rate = 115200;
      size_t data_bits = UART_DATA_8_BITS;
      bool   parity = UART_PARITY_DISABLE;
      size_t stop_bits = UART_STOP_BITS_1;
      bool   flow_control = UART_HW_FLOWCTRL_DISABLE;
      size_t source_clk = UART_SCLK_APB;
      receive_callback_fn on_receive_callback;

      bool is_valid() const;
    };

    static std::unique_ptr<Uart> make_unique() {
      return std::unique_ptr<Uart>(new Uart());
    }

    ~Uart();

    bool start(Config& uart_config);

  private:
    int uart_ = -1;
    bool running_ = false;
    std::uniqe_ptr<Task> task_;

    ConsoleLogger logger_;
    const std::string tag_{"Uart"};

    // private so we can only call make_unique
    Uart();
    // deleted so task structures cannot be copied
    Uart(const Uart&) = delete;
    // deleted so task structures cannot be copied
    Uart& operator=(const Uart&) = delete;

    void close();
    void task_function(const Config& config);
  };
}
