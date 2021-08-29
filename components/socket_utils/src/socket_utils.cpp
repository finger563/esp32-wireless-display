#include "socket_utils.hpp"
using namespace phoenix;

static const char *TAG = "SocketUtils";

bool SocketUtils::validate_address_string(const std::string& address) {
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, address.c_str(), &(sa.sin_addr));
  return result != 0;
}

bool SocketUtils::enable_reuse(int socket) {
  #if CONFIG_LWIP_SO_REUSE
  int err = 0;
  int enable_reuse = 1;
  // allow reuse (so other servers can quickly re-use this port after
  // we shut down)
  err = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &enable_reuse, sizeof(int));
  if (err < 0) {
    ESP_LOGE(TAG, "Unable to set SO_REUSEADDR: %d - %s", errno, strerror(errno));
    return false;
  }
  // Note: this option is unavailable on ESP32 LWIP currently.
  /*
  err = setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &enable_reuse, sizeof(int));
  if (err < 0) {
    ESP_LOGE(TAG, "Unable to set SO_REUSEPORT: %d - %s", errno, strerror(errno));
    return false;
  }
  */
  #else
  ESP_LOGE(TAG, "LWIP_SO_REUSE not enabled in sdkconfig, returning dummy 'true' value!");
  #endif
  return true;
}

bool SocketUtils::make_multicast_socket(int socket, uint8_t time_to_live, bool loopback_enabled) {
  uint8_t ttl = time_to_live;
  uint8_t loopback_val = loopback_enabled;
  int err = 0;

  // Assign multicast TTL (set separately from normal interface TTL)
  err = setsockopt(socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(uint8_t));
  if (err < 0) {
    ESP_LOGE(TAG, "Failed to set IP_MULTICAST_TTL. Error %d - %s", errno, strerror(errno));
    return false;
  }

  // select whether multicast traffic should be received by this device, too
  // (if setsockopt() is not called, the default is no)
  err = setsockopt(socket, IPPROTO_IP, IP_MULTICAST_LOOP,
                   &loopback_val, sizeof(uint8_t));
  if (err < 0) {
    ESP_LOGE(TAG, "Failed to set IP_MULTICAST_LOOP. Error %d - %s", errno, strerror(errno));
    return false;
  }

  return true;
}

bool SocketUtils::add_multicast_group(int socket, std::string_view multicast_group, bool assign_source_if) {
  std::string multicast_group_string{multicast_group};
  struct ip_mreq imreq;
  struct in_addr iaddr;
  int err = 0;
  // Configure source interface
  imreq.imr_interface.s_addr = IPADDR_ANY;
  // Configure multicast address to listen to
  err = inet_aton(multicast_group_string.c_str(), &imreq.imr_multiaddr.s_addr);
  if (err != 1) {
    ESP_LOGE(TAG, "Configured IPV4 multicast address '%s' is invalid.", multicast_group_string.c_str());
    return false;
  }
  ESP_LOGI(TAG, "Configured IPV4 Multicast address %s", inet_ntoa(imreq.imr_multiaddr.s_addr));
  if (!IP_MULTICAST(ntohl(imreq.imr_multiaddr.s_addr))) {
    ESP_LOGW(TAG, "Configured IPV4 multicast address '%s' is not a valid multicast address. This will probably not work.", multicast_group_string.c_str());
  }

  if (assign_source_if) {
    // Assign the IPv4 multicast source interface, via its IP
    // (only necessary if this socket is IPV4 only)
    err = setsockopt(socket, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,
                     sizeof(struct in_addr));
    if (err < 0) {
      ESP_LOGE(TAG, "Failed to set IP_MULTICAST_IF. Error %d - %s", errno, strerror(errno));
      return false;
    }
  }

  err = setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &imreq, sizeof(struct ip_mreq));
  if (err < 0) {
    ESP_LOGE(TAG, "Failed to set IP_ADD_MEMBERSHIP. Error %d - %s", errno, strerror(errno));
    return false;
  }

  return true;
}

std::string SocketUtils::sender_address_to_string(struct sockaddr_in6& source_address) {
  char address_string[INET6_ADDRSTRLEN] = "";
  if (source_address.sin6_family == PF_INET) {
    inet_ntoa_r(((struct sockaddr_in *)&source_address)->sin_addr.s_addr, address_string, sizeof(address_string) - 1);
  } else if (source_address.sin6_family == PF_INET6) {
    inet6_ntoa_r(source_address.sin6_addr, address_string, sizeof(address_string) - 1);
  }
  return std::string{address_string} + ":" + std::to_string(source_address.sin6_port);
}

SocketUtils::SenderInfo SocketUtils::sender_address_to_info(struct sockaddr_in6& source_address) {
  char address_string[INET6_ADDRSTRLEN];
  if (source_address.sin6_family == PF_INET) {
    inet_ntoa_r(((struct sockaddr_in *)&source_address)->sin_addr.s_addr, address_string, sizeof(address_string) - 1);
  } else if (source_address.sin6_family == PF_INET6) {
    inet6_ntoa_r(source_address.sin6_addr, address_string, sizeof(address_string) - 1);
  }
  return {.address=std::string{address_string},
          .port=source_address.sin6_port};
}
