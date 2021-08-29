// required for TEST_CASE & TEST_ASSERT* etc.
#include "unity.h"
// required for unity_wait_for_signal etc.
#include "test_utils.hpp"
// header for code under test
#include "udp_server.hpp"

// needed to connect to the network
#include "wifi.hpp"
#include "udp_client.hpp"

#include <functional>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

// static so they don't possibly conflict with definitions from other
// tests when run together
static std::string multicast_address = "239.1.1.1";
static size_t server_port = 3333;

TEST_CASE("invalid server port", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  auto on_receive_callback =
    [](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      return {};
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=0,
                                                  .is_multicast_endpoint=false,
                                                  .on_receive_callback=on_receive_callback};

  auto server = phoenix::UdpServer::make_unique();
  TEST_ASSERT(!server->start(task_config, server_config));
};

TEST_CASE("invalid receive buffer size", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  auto on_receive_callback =
    [](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      return {};
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                  .receive_buffer_size=0,
                                                  .is_multicast_endpoint=false,
                                                  .on_receive_callback=on_receive_callback};

  auto server = phoenix::UdpServer::make_unique();
  TEST_ASSERT(!server->start(task_config, server_config));
};

TEST_CASE("invalid multicast config", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  auto on_receive_callback =
    [](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      return {};
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  {
    // test not providing the multicast_group
    auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                    .receive_buffer_size=100,
                                                    .is_multicast_endpoint=true,
                                                    .on_receive_callback=on_receive_callback};

    auto server = phoenix::UdpServer::make_unique();
    TEST_ASSERT(!server->start(task_config, server_config));
  }
  {
    // test providing garbage multicast group
    auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                    .receive_buffer_size=100,
                                                    .is_multicast_endpoint=true,
                                                    .multicast_group="garbage",
                                                    .on_receive_callback=on_receive_callback};

    auto server = phoenix::UdpServer::make_unique();
    TEST_ASSERT(!server->start(task_config, server_config));
  }
  {
    // test providing garbage multicast group
    auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                    .receive_buffer_size=100,
                                                    .is_multicast_endpoint=true,
                                                    .multicast_group="10.0.0.",
                                                    .on_receive_callback=on_receive_callback};

    auto server = phoenix::UdpServer::make_unique();
    TEST_ASSERT(!server->start(task_config, server_config));
  }
  {
    // test providing garbage multicast group
    auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                    .receive_buffer_size=100,
                                                    .is_multicast_endpoint=true,
                                                    .multicast_group="10.0.0.256",
                                                    .on_receive_callback=on_receive_callback};

    auto server = phoenix::UdpServer::make_unique();
    TEST_ASSERT(!server->start(task_config, server_config));
  }
  {
    // test providing garbage multicast group
    auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                    .receive_buffer_size=100,
                                                    .is_multicast_endpoint=true,
                                                    .multicast_group="1.1.1.1.1",
                                                    .on_receive_callback=on_receive_callback};

    auto server = phoenix::UdpServer::make_unique();
    TEST_ASSERT(!server->start(task_config, server_config));
  }
};

TEST_CASE("unicast receive without response test", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  bool data_received = false;
  auto on_receive_callback =
    [&data_received](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      data_received = true;
      return {};
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                  .is_multicast_endpoint=false,
                                                  .on_receive_callback=on_receive_callback};

  auto server = phoenix::UdpServer::make_unique();
  TEST_ASSERT(server->start(task_config, server_config));

  // set up the client for sending unicast data
  auto client_config = phoenix::UdpClient::Config{.ip_address=ip_address,
                                                  .port=server_port,
                                                  .is_multicast_endpoint=false};
  phoenix::UdpClient client(client_config);
  // set up the sending configuration
  auto send_config = phoenix::UdpClient::UdpSendConfig{.wait_for_response=false,
                                                       .response_size=0,
                                                       .on_response_callback=nullptr,
                                                       .response_timeout_ms=0};
  // test sending data
  std::this_thread::sleep_for(100ms);
  TEST_ASSERT(client.send("test_payload", send_config));
  std::this_thread::sleep_for(100ms);
  TEST_ASSERT(data_received);
};

TEST_CASE("unicast multi-receive without response test", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  size_t num_data_received = 0;
  auto on_receive_callback =
    [&num_data_received](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      num_data_received++;
      return {};
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                  .is_multicast_endpoint=false,
                                                  .on_receive_callback=on_receive_callback};

  auto server = phoenix::UdpServer::make_unique();
  TEST_ASSERT(server->start(task_config, server_config));
  // allow the server task time to start (give control of the
  // processor back so the task can run)
  std::this_thread::sleep_for(100ms);

  // set up the client for sending unicast data
  auto client_config = phoenix::UdpClient::Config{.ip_address=ip_address,
                                                  .port=server_port,
                                                  .is_multicast_endpoint=false};
  phoenix::UdpClient client(client_config);
  // set up the sending configuration
  auto send_config = phoenix::UdpClient::UdpSendConfig{.wait_for_response=false,
                                                       .response_size=0,
                                                       .on_response_callback=nullptr,
                                                       .response_timeout_ms=0};
  // test sending data
  for (size_t i=0; i<5; i++) {
    TEST_ASSERT(client.send("test_payload", send_config));
    std::this_thread::sleep_for(100ms);
  }
  TEST_ASSERT_EQUAL(5, num_data_received);
};

TEST_CASE("server port reuse test", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  size_t num_data_received = 0;
  auto on_receive_callback =
    [&num_data_received](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      num_data_received++;
      return {};
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                  .is_multicast_endpoint=false,
                                                  .on_receive_callback=on_receive_callback};

  // set up the client for sending unicast data
  auto client_config = phoenix::UdpClient::Config{.ip_address=ip_address,
                                                  .port=server_port,
                                                  .is_multicast_endpoint=false};
  phoenix::UdpClient client(client_config);
  // set up the sending configuration
  auto send_config = phoenix::UdpClient::UdpSendConfig{.wait_for_response=false,
                                                       .response_size=0,
                                                       .on_response_callback=nullptr,
                                                       .response_timeout_ms=0};
  // now actually try creating the server multiple times and re-using the port
  for (size_t i=0; i<5; i++) {
    auto server = phoenix::UdpServer::make_unique();
    TEST_ASSERT(server->start(task_config, server_config));
    // allow the server task time to start (give control of the
    // processor back so the task can run)
    std::this_thread::sleep_for(100ms);
    TEST_ASSERT(client.send("test_payload", send_config));
    std::this_thread::sleep_for(100ms);
  }
  TEST_ASSERT_EQUAL(5, num_data_received);
};

TEST_CASE("unicast receive with response test", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  bool data_received = false;
  auto on_receive_callback =
    [&data_received](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      data_received = true;
      return "server received response!";
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                  .is_multicast_endpoint=false,
                                                  .on_receive_callback=on_receive_callback};

  auto server = phoenix::UdpServer::make_unique();
  TEST_ASSERT(server->start(task_config, server_config));
  // allow the server task time to start (give control of the
  // processor back so the task can run)
  std::this_thread::sleep_for(100ms);

  bool response_received = false;
  auto on_response_callback =
    [&response_received](const std::string& message) {
      ESP_LOGI("on_response_callback", "received: '%s'", message.c_str());
      response_received = true;
    };

  // set up the client for sending unicast data
  auto client_config = phoenix::UdpClient::Config{.ip_address=ip_address,
                                                  .port=server_port,
                                                  .is_multicast_endpoint=false};
  phoenix::UdpClient client(client_config);
  // set up the sending configuration
  auto send_config = phoenix::UdpClient::UdpSendConfig{.wait_for_response=true,
                                                       .response_size=100,
                                                       .on_response_callback=on_response_callback,
                                                       .response_timeout_ms=0};
  // test sending data
  TEST_ASSERT(client.send("test_payload", send_config));
  TEST_ASSERT(data_received);
  TEST_ASSERT(response_received);
};

TEST_CASE("multicast receive without response test", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  bool data_received = false;
  auto on_receive_callback =
    [&data_received](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      data_received = true;
      return {};
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                  .is_multicast_endpoint=true,
                                                  .multicast_group=multicast_address,
                                                  .on_receive_callback=on_receive_callback};

  auto server = phoenix::UdpServer::make_unique();
  TEST_ASSERT(server->start(task_config, server_config));
  // allow the server task time to start (give control of the
  // processor back so the task can run)
  std::this_thread::sleep_for(100ms);

  // set up the client for sending unicast data
  auto client_config = phoenix::UdpClient::Config{.ip_address=multicast_address,
                                                  .port=server_port,
                                                  .is_multicast_endpoint=true};
  phoenix::UdpClient client(client_config);
  // set up the sending configuration
  auto send_config = phoenix::UdpClient::UdpSendConfig{.wait_for_response=false};
  // test sending data
  TEST_ASSERT(client.send("test_payload", send_config));
  std::this_thread::sleep_for(100ms);
  TEST_ASSERT(data_received);
};

TEST_CASE("multicast receive with response test", "[UDP_SERVER]")
{
  std::this_thread::sleep_for(500ms);

  // Initialize NVS, then connect to SSID
  auto wifiman = phoenix::WifiConnectionManager({
    .ssid = CONFIG_ESP_WIFI_SSID,
    .password = CONFIG_ESP_WIFI_PASSWORD
  });
  TEST_ASSERT(wifiman.is_connected());

  // Network Info
  const auto ip_address = phoenix::WifiConnectionManager::ip_address;
  const auto netmask = phoenix::WifiConnectionManager::netmask;
  const auto gateway = phoenix::WifiConnectionManager::gateway;

  bool data_received = false;
  auto on_receive_callback =
    [&data_received](const std::string& message, const phoenix::SocketUtils::SenderInfo& sender_info) -> std::optional<std::string> {
      ESP_LOGI("on_receive_callback", "received: '%s'", message.c_str());
      ESP_LOGI("on_receive_callback", "from: %s:%d",
               sender_info.address.c_str(), sender_info.port);
      data_received = true;
      return "server received response!";
    };

  // set up the server for receiving the data
  auto task_config = phoenix::Task::Config{.stack_size_bytes=4096};
  auto server_config = phoenix::UdpServer::Config{.port=server_port,
                                                  .is_multicast_endpoint=true,
                                                  .multicast_group=multicast_address,
                                                  .on_receive_callback=on_receive_callback};

  auto server = phoenix::UdpServer::make_unique();
  TEST_ASSERT(server->start(task_config, server_config));
  // allow the server task time to start (give control of the
  // processor back so the task can run)
  std::this_thread::sleep_for(100ms);

  bool response_received = false;
  auto on_response_callback =
    [&response_received](const std::string& message) {
      ESP_LOGI("on_response_callback", "received: '%s'", message.c_str());
      response_received = true;
    };

  // set up the client for sending unicast data
  auto client_config = phoenix::UdpClient::Config{.ip_address=multicast_address,
                                                  .port=server_port,
                                                  .is_multicast_endpoint=true};
  phoenix::UdpClient client(client_config);
  // set up the sending configuration
  auto send_config = phoenix::UdpClient::UdpSendConfig{.wait_for_response=true,
                                                       .response_size=100,
                                                       .on_response_callback=on_response_callback,
                                                       .response_timeout_ms=1000};
  // test sending data
  std::this_thread::sleep_for(100ms);
  TEST_ASSERT(client.send("test_payload", send_config));
  std::this_thread::sleep_for(100ms);
  TEST_ASSERT(data_received);
  TEST_ASSERT(response_received);
};
