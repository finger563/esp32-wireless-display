#pragma once
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#include <chrono>
#include <string>
#include <string_view>
#include <thread>

namespace phoenix {

class WifiConnectionManager {
  static std::string ssid_;
  static std::string password_;

  // global state
  static bool nvs_is_initialized_;
  static bool netif_is_initialized_;

  // Connection parameters
  static size_t maximum_wait_time_on_connected_event_ms_;
  static size_t sleep_time_before_connected_retry_ms_;
  static bool is_connected_;

  // Scan parameters
  static size_t scan_result_size_;

  // FreeRTOS event group to signal when we are connected
  static EventGroupHandle_t event_group_;
  static bool event_handlers_registered_;

  // The event group allows multiple bits for each event, but we only care about
  // two events:
  // - we are connected to the AP with an IP
  // - we failed to connect after the maximum amount of retries
  static EventBits_t WIFI_CONNECTED_BIT;
  static EventBits_t WIFI_FAIL_BIT;

  // Callback for various Wifi events
  static void event_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);

  // wifi helper functions
  static void display_scan_result(size_t scan_result_size);
  static const char *wifi_error_to_string(uint8_t errCode);

  // Initialize NVS
  static void initialize_nvs();

  // Initialize & Connect
  static bool scan_and_connect();

  // Connect
  static bool connect();

public:
  struct Config {
    std::string_view ssid;
    std::string_view password;

    // When calling esp_wifi_connect(), how long to wait
    // for WIFI_CONNECTED_BIT to be set?
    // Ideally, the WIFI_CONNECTED_BIT is set (in the connection event handler)
    // before this timeout is reached. If the timeout is reached, then
    // likely this indicates a connection failure - Another connection attempt
    // will be made
    size_t maximum_wait_time_on_connected_event_ms = 10000; // 10 seconds

    // If another connection attempt is to be made, how long to sleep
    // before making the next connection attempt?
    // Before the connection attempt is made, wifi is disconnected, stopped,
    // and then restarted
    size_t sleep_time_before_connected_retry_ms = 2000; // 2 seconds
  };

private:
  // Initialize NVS, Scan and Connect to AP
  static void initialize(const Config &config);

  // Deinitialize & disconnect
  static void deinitialize();

public:
  // Construct a wifi connection manager object
  // Initialize NVS, scan and connect to AP
  WifiConnectionManager(const Config &config);

  // Disconnect from AP and stop wifi
  ~WifiConnectionManager();

  // Connection result
  static std::string ip_address;
  static std::string netmask;
  static std::string gateway;

  // Quick status check
  static bool is_connected();
};
} // namespace phoenix
