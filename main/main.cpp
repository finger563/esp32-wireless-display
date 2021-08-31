#include <chrono>
#include <functional>
#include <thread>

#include "console_logger.hpp"
#include "task.hpp"
#include "uart.hpp"
#include "wifi.hpp"
#include "udp_server.hpp"
#include "wireless_display.hpp"

using namespace std::chrono_literals;

extern "C" void app_main(void) {
  phoenix::ConsoleLogger logger;

  logger.info("Connecting to wifi");

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
                                                 .ssid = CONFIG_ESP_WIFI_SSID,
                                                 .password = CONFIG_ESP_WIFI_PASSWORD
    });

  while (!wifiman.is_connected()) {
    logger.info("Waiting for wifi to connect...");
    std::this_thread::sleep_for(500ms);
  }

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  /**
   * Display Task for managing the plotting and such. Set the
   * plot_display to be 2/3 the height of the screen, and the
   * text_display to be the remaining 1/3 of the screen.
   */
  logger.info("Configuring display");
  auto plot_height = CONFIG_DISPLAY_HEIGHT * 2 / 3;
  //auto wireless_display = WirelessDisplay(0, CONFIG_DISPLAY_WIDTH, plot_height, CONFIG_DISPLAY_HEIGHT);
  auto display_task_callback =
    [/* &wireless_display */](void) -> void
    {
      // simply call the wireless display update function
      // wireless_display.update();
      // and sleep for some amount of time
      std::this_thread::sleep_for(10ms);
    };
  auto display_task_config = phoenix::Task::Config{.stack_size_bytes=8192,
                                                   .on_task_callback=display_task_callback};
  auto display_task = phoenix::Task::make_unique();
  bool task_started = display_task->start(display_task_config);
  if (!task_started) {
    logger.error("Error: could not start display task!");
  }

  /**
   * UDP Server Receive Data
   */
  logger.info("Configuring udp_server");
  auto on_udp_server_receive_data =
    [/* &wireless_display */](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) ->
    std::optional<std::string>
    {
     // we received a message, let the display manager know and handle
     // it
     // wireless_display.push_data(message);
     // return a string if we want to respond to the sender, else return {}
     return {};
    };

  auto server_task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=CONFIG_UDP_SERVER_PORT,
                                                  .is_multicast_endpoint=false,
                                                  .on_receive_callback=on_udp_server_receive_data};
  auto server = phoenix::UdpServer::make_unique();
  bool server_started = server->start(server_task_config, server_config);
  if (!server_started) {
    logger.error("Error: could not start udp server!");
  }

  /**
   * UART Receive Data
   */
  logger.info("Configuring UART");
  auto on_uart_receive_data =
    [/* &wireless_display */](const std::string& message) -> std::optional<std::string>
    {
     // we received a message, let the display manager know and handle
     // it
     // wireless_display.push_data(message);
     // return a string if we want to respond to the sender, else return {}
     return {};
    };

  auto uart_task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto uart_config = phoenix::Uart::Config{.uart=CONFIG_UART_PORT,
                                           .pin_config={.tx=CONFIG_UART_TX,
                                                        .rx=CONFIG_UART_RX,
                                                        .rts=CONFIG_UART_RTS,
                                                        .cts=CONFIG_UART_CTS,
                                                        },
                                           .port_config={.baud_rate=CONFIG_UART_BAUDRATE,
                                                         },
                                           .pattern_config={.character='+',
                                                            .count=3
                                                            },
                                           .on_receive_callback=on_uart_receive_data,
                                           .timeout_ms=1000,
                                           .receive_buffer_size=128};
  auto uart = phoenix::Uart::make_unique();
  bool uart_started = uart->start(uart_task_config, uart_config);
  if (!uart_started) {
    logger.error("Error: could not start uart!");
  }

  while (true) {
    std::this_thread::sleep_for(1s);
  }
}
