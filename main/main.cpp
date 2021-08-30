#include <chrono>
#include <functional>
#include <thread>

#include "wifi.hpp"
#include "uart.hpp"
#include "udp_server.hpp"

using namespace std::chrono_literals;

extern "C" void app_main(void) {
  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  /**
   * UDP Server Receive Data
   */
  auto on_udp_server_receive_data =
    [](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) ->
    std::optional<std::string>
    {
     // we received a message, let the display manager know and handle
     // it
     // DisplayTask::pushData( message );
     // return a string if we want to respond to the sender, else return {}
     return {};
    };

  auto server_task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=0,
                                                  .is_multicast_endpoint=false,
                                                  .on_receive_callback=on_udp_server_receive_data};
  auto server = phoenix::UdpServer::make_unique();
  bool server_started = server->start(server_task_config, server_config);

  /**
   * UART Receive Data
   */
  auto on_uart_receive_data =
    [](const std::string& message) -> std::optional<std::string>
    {
     // we received a message, let the display manager know and handle
     // it
     // DisplayTask::pushData( message );
     // return a string if we want to respond to the sender, else return {}
     return {};
    };

  auto uart_task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto uart_config = phoenix::Uart::Config{.uart=UART_NUM_1,
                                           .pin_config={
                                                        },
                                           .port_config={
                                                         },
                                           .pattern_config={
                                                            },
                                           .on_receive_callback=on_uart_receive_data,
                                           .timeout_ms=1000,
                                           .receive_buffer_size=128};
  auto uart = phoenix::Uart::make_unique();
  bool uart_started = uart->start(uart_task_config, uart_config);

  while (true) {
    std::this_thread::sleep_for(1s);
  }
}
