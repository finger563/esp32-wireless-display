// required for TEST_CASE & TEST_ASSERT* etc.
#include "unity.h"
// required for unity_wait_for_signal etc.
#include "test_utils.hpp"
// header for code under test
#include "wifi.hpp"

TEST_CASE("Wifi connection test", "[WIFI]")
{
  // Initialize NVS, then connect to SSID
  phoenix::WifiConnectionManager::initialize({
                                                 .ssid = CONFIG_ESP_WIFI_SSID,
                                                 .password = CONFIG_ESP_WIFI_PASSWORD
    });

  TEST_ASSERT(phoenix::WifiConnectionManager::is_connected());

  phoenix::WifiConnectionManager::deinitialize();
};

TEST_CASE("Wifi re-connection test", "[WIFI]")
{
  {
    // Initialize NVS, then connect to SSID
    phoenix::WifiConnectionManager::initialize({
                                                   .ssid = CONFIG_ESP_WIFI_SSID,
                                                   .password = CONFIG_ESP_WIFI_PASSWORD
      });

    TEST_ASSERT(phoenix::WifiConnectionManager::is_connected());

    phoenix::WifiConnectionManager::deinitialize();
  }
  // when the first manager goes out of scope, it will be destructed
  // and will disconnect, the second manager will re-init and
  // re-connect
  {
    // Initialize NVS, then connect to SSID
    phoenix::WifiConnectionManager::initialize({
                                                   .ssid = CONFIG_ESP_WIFI_SSID,
                                                   .password = CONFIG_ESP_WIFI_PASSWORD
      });

    TEST_ASSERT(phoenix::WifiConnectionManager::is_connected());

    phoenix::WifiConnectionManager::deinitialize();
  }
};

TEST_CASE("Wifi multi connection test", "[WIFI]")
{
  for (size_t i=0; i<5; i++) {
    // Initialize NVS, then connect to SSID
    phoenix::WifiConnectionManager::initialize({
                                                   .ssid = CONFIG_ESP_WIFI_SSID,
                                                   .password = CONFIG_ESP_WIFI_PASSWORD
      });

    TEST_ASSERT(phoenix::WifiConnectionManager::is_connected());

    phoenix::WifiConnectionManager::deinitialize();
  }
};
