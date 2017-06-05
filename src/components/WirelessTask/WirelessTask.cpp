#include "WirelessTask.hpp"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MS_TO_TICKS( xTimeInMs ) (uint32_t)( ( ( TickType_t ) xTimeInMs * configTICK_RATE_HZ ) / ( TickType_t ) 1000 )

namespace WirelessTask {

  // User definitions for the task
  extern "C" {
    #include <string.h>
    #include <sys/socket.h>
  }

  static int mysocket;

  static struct sockaddr_in remote_addr;
  static unsigned int socklen;

  static esp_err_t event_handler(void *ctx, system_event_t *event)
  {
    std::string ipStr;
    switch(event->event_id) {
      case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(udp_event_group, WIFI_CONNECTED_BIT);
        break;
      case SYSTEM_EVENT_STA_CONNECTED:
        break;
      case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "event_handler:SYSTEM_EVENT_STA_GOT_IP!");
        ESP_LOGI(TAG, "got ip:%s\n",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        ipStr = std::string("IP: ") + ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip);
        ipStr += ":" + std::to_string(EXAMPLE_DEFAULT_PORT);
        DisplayTask::pushData( ipStr );
        xEventGroupSetBits(udp_event_group, WIFI_CONNECTED_BIT);
        break;
      case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR" join,AID=%d\n",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        xEventGroupSetBits(udp_event_group, WIFI_CONNECTED_BIT);
        break;
      case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR"leave,AID=%d\n",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        xEventGroupClearBits(udp_event_group, WIFI_CONNECTED_BIT);
        break;
      default:
        break;
    }
    return ESP_OK;
  }

  // Generated state variables
  bool     __change_state__ = false;
  uint32_t __state_delay__ = 0;
  uint8_t  stateLevel_0;

  // Generated task function
  void taskFunction ( void *pvParameter ) {
    // initialize here
    __change_state__ = false;
    __state_delay__ = 10;
    state_State_1_setState();
    // execute the init transition for the initial state and task
    #if 1
    udp_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
      {
        "Max Mobility 2.4 GHz",
        "1qaz2wsx3edc4rfv",
        false
      }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    ESP_LOGI(TAG, "task udp_conn start.");
    /*wating for connecting to AP*/
    xEventGroupWaitBits(udp_event_group, WIFI_CONNECTED_BIT,false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "sta has connected to ap.");

    #else
    wifi_init_softap();
    #endif

    /*create udp socket*/
    int socket_ret;

    ESP_LOGI(TAG, "create udp server after 3s...");
    vTaskDelay(3000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "create_udp_server.");
    socket_ret=create_udp_server();

    if(socket_ret == ESP_FAIL) {
      ESP_LOGI(TAG, "create udp socket error,stop.");
      vTaskDelete(NULL);
    }

    /*create a task to tx/rx data*/
    //TaskHandle_t tx_rx_task;
    //xTaskCreate(&send_recv_data, "send_recv_data", 4096, NULL, 4, &tx_rx_task);

    /*waiting udp connected success*/
    //xEventGroupWaitBits(udp_event_group, UDP_CONNCETED_SUCCESS,false, true, portMAX_DELAY);
    //ESP_LOGI(TAG, "udp connected");

    int len;
    char databuff[EXAMPLE_DEFAULT_PKTSIZE];

    /*send&receive first packet*/
    socklen = sizeof(remote_addr);

    ESP_LOGI(TAG, "first recvfrom:");
    len = recvfrom(mysocket, databuff, EXAMPLE_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, &socklen);

    if (len > 0) {
      ESP_LOGI(TAG, "transfer data with %s:%u\n",
               inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
      xEventGroupSetBits(udp_event_group, UDP_CONNCETED_SUCCESS);
    } else {
      show_socket_error_reason(mysocket);
      close(mysocket);
      vTaskDelete(NULL);
    } /*if (len > 0)*/

    // now loop running the state code
    while (true) {
      // reset __change_state__ to false
      __change_state__ = false;
      // run the proper state function
      state_State_1_execute();
      // now wait if we haven't changed state
      if (!__change_state__) {
        vTaskDelay( MS_TO_TICKS(__state_delay__) );
      }
      else {
        vTaskDelay( MS_TO_TICKS(1) );
      }
    }
  }

  // Generated state functions
  const uint8_t state_State_1 = 0;

  void state_State_1_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_State_1)
      return;

    state_State_1_transition();

    // execute all substates

    if (!__change_state__) {
      int len;
      char databuff[EXAMPLE_DEFAULT_PKTSIZE];
      memset( databuff, 0, EXAMPLE_DEFAULT_PKTSIZE );
      len = recvfrom(mysocket, databuff, EXAMPLE_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, &socklen);
      if (len > 0) {
        ESP_LOGI(TAG, "transfer data with %s:%u\n",
                 inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
        ESP_LOGI(TAG, " received: %s", databuff);
        std::string newData( (const char *)databuff );
        DisplayTask::pushData( newData );
      } else {
        if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) {
          show_socket_error_reason(mysocket);
        }
      } /*if (len > 0)*/
    }
  }

  void state_State_1_setState( void ) {
    stateLevel_0 = state_State_1;
  }

  void state_State_1_transition( void ) {
    if (__change_state__)
      return;
  }

  void state_State_1_finalization( void ) {

  }

 
};
