#ifndef UDPSERVER_INCLUDE_GUARD_
#define UDPSERVER_INCLUDE_GUARD_
/* udp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifdef __cplusplus
extern "C" {
#endif

  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/event_groups.h"
  #include "esp_wifi.h"
  #include "esp_event_loop.h"
  #include "esp_log.h"

  /*test options*/
  #define EXAMPLE_ESP_WIFI_MODE_AP     1 //TRUE:AP FALSE:STA
  #define EXAMPLE_ESP_UDP_MODE_SERVER  1 //TRUE:server FALSE:client
  #define EXAMPLE_ESP_UDP_PERF_TX      0 //TRUE:send FALSE:receive

  /*AP info and tcp_server info*/
  #define EXAMPLE_DEFAULT_SSID         ("Max Mobility 2.4 GHz")
  #define EXAMPLE_DEFAULT_PWD          ("1qaz2wsx3edc4rfv")
  #define EXAMPLE_DEFAULT_PORT         5555
  #define EXAMPLE_DEFAULT_PKTSIZE      100
  #define EXAMPLE_MAX_STA_CONN         1 //how many sta can be connected(AP mode)

  #ifdef CONFIG_UDP_PERF_SERVER_IP
  #define EXAMPLE_DEFAULT_SERVER_IP    CONFIG_UDP_PERF_SERVER_IP
  #else
  #define EXAMPLE_DEFAULT_SERVER_IP    "192.168.4.1"
  #endif /*CONFIG_UDP_PERF_SERVER_IP*/

  #define TAG "udp_perf:"

  /* FreeRTOS event group to signal when we are connected to WiFi and ready to start UDP test*/
  extern EventGroupHandle_t udp_event_group;
  #define WIFI_CONNECTED_BIT      BIT0
  #define UDP_CONNCETED_SUCCESS   BIT1

  extern int total_data;
  extern int success_pack;

  //using esp as station
  void wifi_init_sta();
  //using esp as softap
  void wifi_init_softap();

  //create a udp server socket. return ESP_OK:success ESP_FAIL:error
  esp_err_t create_udp_server();
  //create a udp client socket. return ESP_OK:success ESP_FAIL:error
  esp_err_t create_udp_client();

  //send or recv data task
  void send_recv_data(void *pvParameters);

  //get socket error code. return: error code
  int get_socket_error_code(int socket);

  //show socket error code. return: error code
  int show_socket_error_reason(int socket);

  //check connected socket. return: error code
  int check_connected_socket();

  //close all socket
  void close_socket();

#ifdef __cplusplus
}
#endif

#endif //UDPSERVER_INCLUDE_GUARD_