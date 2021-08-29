#pragma once
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <string>
#include <string_view>

namespace phoenix {

class SocketUtils {

public:
  struct SenderInfo {
    std::string address;
    size_t port;
  };

  // check strings to see if they are of the form X.X.X.X, where 0 <= X <= 255
  static bool validate_address_string(const std::string& address);

  // allow reuse (so other servers can quickly re-use this port after
  // we shut down)
  static bool enable_reuse(int socket);

  // Assigns multicast TTL
  // Sets the socket option to ensure that multicast traffic should be received by this device
  static bool make_multicast_socket(int socket, uint8_t time_to_live=1, bool loopback_enabled = true);

  // Adds the input socket to a multicast group
  static bool add_multicast_group(int socket, std::string_view multicast_group, bool assign_source_if = true);

  // simple wrapper for converting sender address structure to string
  static std::string sender_address_to_string(struct sockaddr_in6& source_address);

  // simple wrapper for converting sender address structure to string
  static SenderInfo sender_address_to_info(struct sockaddr_in6& source_address);
};

}
