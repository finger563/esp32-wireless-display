#include "wifi.hpp"
using namespace phoenix;

static const char *TAG = "WifiConnectionManager";

// Connection properties
std::string WifiConnectionManager::ssid_ = "";
std::string WifiConnectionManager::password_ = "";

// Connection parameters
bool WifiConnectionManager::nvs_is_initialized_ = false;
bool WifiConnectionManager::netif_is_initialized_ = false;

// Connection parameters
size_t WifiConnectionManager::maximum_wait_time_on_connected_event_ms_ =
    10000; // 10 seconds
size_t WifiConnectionManager::sleep_time_before_connected_retry_ms_ =
    2000; // 2 seconds
bool WifiConnectionManager::is_connected_ = false;

// Connection result
std::string WifiConnectionManager::ip_address{""};
std::string WifiConnectionManager::netmask{""};
std::string WifiConnectionManager::gateway{""};

// Scan parameters
size_t WifiConnectionManager::scan_result_size_ = 10;

// FreeRTOS event group to signal when we are connected
EventGroupHandle_t WifiConnectionManager::event_group_;
bool WifiConnectionManager::event_handlers_registered_;
EventBits_t WifiConnectionManager::WIFI_CONNECTED_BIT = BIT0;
EventBits_t WifiConnectionManager::WIFI_FAIL_BIT = BIT1;

void WifiConnectionManager::event_handler(void *arg,
                                          esp_event_base_t event_base,
                                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI(TAG, "Wifi station started");
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP) {
    ESP_LOGI(TAG, "Wifi station stopped");
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
    ESP_LOGI(TAG, "Scan done");
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    ESP_LOGI(TAG, "Connected to AP");
    is_connected_ = true;
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    is_connected_ = false;
    auto event = (wifi_event_sta_disconnected_t *)event_data;
    ESP_LOGI(TAG, "Disconnected from AP, reason: %s",
             wifi_error_to_string(event->reason));
    ESP_LOGI(TAG, "Retrying connection to the AP");

    is_connected_ = connect();
    if (is_connected_) {
      xEventGroupSetBits(event_group_, WIFI_CONNECTED_BIT);
    } else {
      xEventGroupClearBits(event_group_, WIFI_CONNECTED_BIT);
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Wi-Fi Link UP");
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    {
      char ip_address_str[40];
      sprintf(ip_address_str, IPSTR, IP2STR(&event->ip_info.ip));
      ip_address = std::string{ip_address_str};
    }

    {
      char netmask_str[40];
      sprintf(netmask_str, IPSTR, IP2STR(&event->ip_info.netmask));
      netmask = std::string{netmask_str};
    }

    {
      char gateway_str[40];
      sprintf(gateway_str, IPSTR, IP2STR(&event->ip_info.gw));
      gateway = std::string{gateway_str};
    }

    xEventGroupSetBits(event_group_, WIFI_CONNECTED_BIT);
    is_connected_ = true;
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
    ESP_LOGI(TAG, "Lost IP");
    is_connected_ = false;
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    ESP_LOGI(TAG, "Wi-Fi AP STA disconnected");
    is_connected_ = false;
  }
}

void WifiConnectionManager::initialize_nvs() {
  if (!nvs_is_initialized_) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    nvs_is_initialized_ = true;
  }
}

// these must be declared outside the function, otherwise deinit and
// reinit for reconnection / reinitialization will fail.
wifi_init_config_t wifi_init_config;
wifi_config_t wifi_config;

bool WifiConnectionManager::scan_and_connect() {
  if (!netif_is_initialized_) {
    // initialize the network interface
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // create wifi station
    esp_netif_create_default_wifi_sta();
    wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    // configure the access point connection info
    for (auto i = 0; i < ssid_.size(); i++) {
      wifi_config.sta.ssid[i] = (uint8_t)ssid_[i];
    }
    wifi_config.sta.ssid[ssid_.size()] = '\0';

    for (auto i = 0; i < password_.size(); i++) {
      wifi_config.sta.password[i] = (uint8_t)password_[i];
    }
    wifi_config.sta.password[password_.size()] = '\0';

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    netif_is_initialized_ = true;
  }

  // make sure we clear up anything that may be around if the previous
  // connection was not destroyed properly
  deinitialize();

  if (!event_handlers_registered_) {
    event_group_ = xEventGroupCreate();
    auto event_handler = WifiConnectionManager::event_handler;

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                               event_handler, NULL));
    event_handlers_registered_ = true;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_sta finished.");

  // Scan and display results - passing true blocks the calling code
  // until scan completes
  ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
  display_scan_result(scan_result_size_);
  ESP_ERROR_CHECK(esp_wifi_scan_stop());

  return connect();
}

bool WifiConnectionManager::connect() {
  bool result{false};

  while (true) {

    xEventGroupSetBits(event_group_, WIFI_CONNECTED_BIT);
    xEventGroupSetBits(event_group_, WIFI_FAIL_BIT);

    result = false;

    // now actually connect to the AP
    esp_wifi_connect();

    // Waiting until either the connection is established (WIFI_CONNECTED_BIT)
    // or connection failed for the maximum
    // number of re-tries (WIFI_FAIL_BIT).
    // The bits are set by wifi_event_handler() (see above)
    const TickType_t xTicksToWait =
        maximum_wait_time_on_connected_event_ms_ / portTICK_PERIOD_MS;

    EventBits_t bits =
        xEventGroupWaitBits(event_group_, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                            pdFALSE, pdFALSE, xTicksToWait);

    // xEventGroupWaitBits() returns the bits before the call returned,
    // hence we can test which event actually happened.
    if (bits & WIFI_CONNECTED_BIT) {
      ESP_LOGI(TAG, "Connecting to AP SSID:%s password:%s", ssid_.c_str(),
               password_.c_str());
      result = true;
    } else if (bits & WIFI_FAIL_BIT) {
      ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid_.c_str(),
               password_.c_str());
    } else {
      ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    if (result) {
      // connected to SSID w/ password
      break;
    } else {
      const std::string message =
          "Retrying Wi-Fi connect after " +
          std::to_string(sleep_time_before_connected_retry_ms_) + " ms";

      ESP_LOGI(TAG, "%s", message.c_str());
      std::this_thread::sleep_for(
          std::chrono::milliseconds(sleep_time_before_connected_retry_ms_));

      esp_wifi_disconnect();
      esp_wifi_stop();
      esp_wifi_start();
    }
  }

  return result;
}

WifiConnectionManager::WifiConnectionManager(const Config &config) {
  initialize(config);
}

void WifiConnectionManager::initialize(const Config &config) {
  ssid_ = config.ssid;
  password_ = config.password;
  maximum_wait_time_on_connected_event_ms_ =
      config.maximum_wait_time_on_connected_event_ms;
  sleep_time_before_connected_retry_ms_ =
      config.sleep_time_before_connected_retry_ms;

  initialize_nvs();
  is_connected_ = scan_and_connect();
}

WifiConnectionManager::~WifiConnectionManager() { deinitialize(); }

void WifiConnectionManager::deinitialize() {
  // Disconnect event handlers
  if (event_handlers_registered_) {
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID,
                                                 event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                 event_handler));
    vEventGroupDelete(event_group_);
    event_handlers_registered_ = false;
  }

  esp_wifi_disconnect();
  esp_wifi_stop();
  // don't fully tear down the wifi driver by calling wifi_deinit(),
  // simply set mode to NULL
  esp_wifi_set_mode(WIFI_MODE_NULL);
}

bool WifiConnectionManager::is_connected() { return is_connected_; }

// utility functions
void WifiConnectionManager::display_scan_result(size_t scan_result_size) {
  uint16_t number = scan_result_size;
  wifi_ap_record_t ap_info[scan_result_size];
  uint16_t ap_count = 0;
  memset(ap_info, 0, sizeof(ap_info));
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
  ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
  for (int i = 0; (i < scan_result_size) && (i < ap_count); i++) {
    ESP_LOGI(TAG, "SSID:\t'%s'\t%d", ap_info[i].ssid, ap_info[i].rssi);
  }
}

/**
 * @brief Convert a wifi_err_reason_t code to a string.
 * @param [in] errCode The errCode to be converted.
 * @return A string representation of the error code.
 *
 * @note: wifi_err_reason_t values as of Nov 2020 are: (1-24, 53, 200-206) and
 * are defined in ~/esp-idf/components/esp_wifi/include/esp_wifi_types.h.
 */
const char *WifiConnectionManager::wifi_error_to_string(uint8_t errCode) {
  if (errCode == ESP_OK)
    return "ESP_OK (received SYSTEM_EVENT_STA_GOT_IP event)";
  if (errCode == UINT8_MAX)
    return "Not Connected (default value)";

  switch ((wifi_err_reason_t)errCode) {
  case WIFI_REASON_UNSPECIFIED:
    return "WIFI_REASON_UNSPECIFIED";
  case WIFI_REASON_AUTH_EXPIRE:
    return "WIFI_REASON_AUTH_EXPIRE";
  case WIFI_REASON_AUTH_LEAVE:
    return "WIFI_REASON_AUTH_LEAVE";
  case WIFI_REASON_ASSOC_EXPIRE:
    return "WIFI_REASON_ASSOC_EXPIRE";
  case WIFI_REASON_ASSOC_TOOMANY:
    return "WIFI_REASON_ASSOC_TOOMANY";
  case WIFI_REASON_NOT_AUTHED:
    return "WIFI_REASON_NOT_AUTHED";
  case WIFI_REASON_NOT_ASSOCED:
    return "WIFI_REASON_NOT_ASSOCED";
  case WIFI_REASON_ASSOC_LEAVE:
    return "WIFI_REASON_ASSOC_LEAVE";
  case WIFI_REASON_ASSOC_NOT_AUTHED:
    return "WIFI_REASON_ASSOC_NOT_AUTHED";
  case WIFI_REASON_DISASSOC_PWRCAP_BAD:
    return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
  case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
    return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
  case WIFI_REASON_IE_INVALID:
    return "WIFI_REASON_IE_INVALID";
  case WIFI_REASON_MIC_FAILURE:
    return "WIFI_REASON_MIC_FAILURE";
  case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
  case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
    return "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
  case WIFI_REASON_IE_IN_4WAY_DIFFERS:
    return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
  case WIFI_REASON_GROUP_CIPHER_INVALID:
    return "WIFI_REASON_GROUP_CIPHER_INVALID";
  case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
    return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
  case WIFI_REASON_AKMP_INVALID:
    return "WIFI_REASON_AKMP_INVALID";
  case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
    return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
  case WIFI_REASON_INVALID_RSN_IE_CAP:
    return "WIFI_REASON_INVALID_RSN_IE_CAP";
  case WIFI_REASON_802_1X_AUTH_FAILED:
    return "WIFI_REASON_802_1X_AUTH_FAILED";
  case WIFI_REASON_CIPHER_SUITE_REJECTED:
    return "WIFI_REASON_CIPHER_SUITE_REJECTED";
  case WIFI_REASON_INVALID_PMKID:
    return "WIFI_REASON_INVALID_PMKID";
  case WIFI_REASON_BEACON_TIMEOUT:
    return "WIFI_REASON_BEACON_TIMEOUT";
  case WIFI_REASON_NO_AP_FOUND:
    return "WIFI_REASON_NO_AP_FOUND";
  case WIFI_REASON_AUTH_FAIL:
    return "WIFI_REASON_AUTH_FAIL";
  case WIFI_REASON_ASSOC_FAIL:
    return "WIFI_REASON_ASSOC_FAIL";
  case WIFI_REASON_HANDSHAKE_TIMEOUT:
    return "WIFI_REASON_HANDSHAKE_TIMEOUT";
  case WIFI_REASON_CONNECTION_FAIL:
    return "WIFI_REASON_CONNECTION_FAIL";
  default:
    return "Unknown ESP_ERR error";
  }
} // wifi_error_to_string
