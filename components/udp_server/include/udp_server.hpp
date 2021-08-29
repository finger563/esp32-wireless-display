#pragma once
#include <string.h>
#include <functional>
#include <optional>

#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "socket_utils.hpp"
#include "task.hpp"

namespace phoenix {

class UdpServer {
public:
  typedef std::function<std::optional<std::string>(const std::string&, const SocketUtils::SenderInfo&)> receive_callback_fn;

  struct Config {
    size_t port;
    size_t receive_buffer_size = 128;
    bool is_multicast_endpoint = false;
    std::string multicast_group = "";
    receive_callback_fn on_receive_callback;

    bool is_valid() const;
  };

  static std::unique_ptr<UdpServer> make_unique() {
    return std::unique_ptr<UdpServer>(new UdpServer());
  }

  ~UdpServer();

  bool start(Task::Config& task_config, const Config& server_config);

private:
  int socket_;
  bool running_;
  std::unique_ptr<Task> task_;

  // private so we can only call make_unique
  UdpServer();
  // deleted so task structures cannot be copied
  UdpServer(const UdpServer&) = delete;
  // deleted so task structures cannot be copied
  UdpServer& operator=(const UdpServer&) = delete;

  void close_socket();
  void task_function(const Config& config);
};

}
