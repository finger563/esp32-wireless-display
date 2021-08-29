#include "udp_server.hpp"
using namespace phoenix;

static const char *TAG = "UdpServer";

UdpServer::UdpServer() :
  socket_(-1),
  running_(false),
  task_(Task::make_unique()) {
}

UdpServer::~UdpServer() {
  running_ = false;
  close_socket();
}

void UdpServer::close_socket() {
  ESP_LOGI(TAG, "closing socket");
  if (socket_ != -1) {
    shutdown(socket_, 0);
    close(socket_);
    socket_ = -1;
    ESP_LOGI(TAG, "socket closed");
  }
}

void UdpServer::task_function(const UdpServer::Config& config) {
    // data for the server socket
    int ip_protocol = IPPROTO_IP;
    int address_family = AF_INET;
    struct sockaddr_in dest_addr;
    struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);
    char receive_buffer[config.receive_buffer_size];

    // get the data out of the parameters
    size_t port = config.port;
    bool is_multicast_endpoint = config.is_multicast_endpoint;
    std::string multicast_group = config.multicast_group;
    auto on_receive_callback = config.on_receive_callback;

    // configure the server socket accordingly - assume IPV4
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = address_family;
    dest_addr.sin_port = htons(port);

    // Wrap the whole configuration in a while loop to automatically
    // restart the server socket if errors occur
    while (running_) {
      bool can_receive = true;

      socket_ = socket(address_family, SOCK_DGRAM, ip_protocol);
      if (socket_ < 0) {
        ESP_LOGE(TAG, "Unable to create socket - %d: %s", errno, strerror(errno));
        can_receive = false;
      }
      ESP_LOGI(TAG, "Socket created");

      if (!SocketUtils::enable_reuse(socket_)) {
        ESP_LOGE(TAG, "Warning, socket cannot be re-used!");
      }

      int err = bind(socket_, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
      if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind - %d: %s", errno, strerror(errno));
        can_receive = false;
      }
      ESP_LOGI(TAG, "Socket bound, port %d", port);

      if (is_multicast_endpoint) {
        ESP_LOGI(TAG, "Configuring multicast endpoint.");
        // make it multicast
        if (!SocketUtils::make_multicast_socket(socket_)) {
          ESP_LOGE(TAG, "Socket unable to make multicast - %d: %s", errno, strerror(errno));
          can_receive = false;
        }

        // add it to the multicast group
        if (!SocketUtils::add_multicast_group(socket_, multicast_group)) {
          ESP_LOGE(TAG, "Socket unable to add multicast group - %d: %s", errno, strerror(errno));
          can_receive = false;
        }
        ESP_LOGI(TAG, "Multicast endpoint configured.");
      }

      // now run core receive loop
      while (can_receive && running_) {
        ESP_LOGI(TAG, "Waiting for data");
        int len = recvfrom(socket_,
                           receive_buffer,
                           sizeof(receive_buffer) - 1,
                           0,
                           (struct sockaddr *)&source_addr,
                           &socklen);

        // Error occurred during receiving
        if (len < 0) {
          ESP_LOGE(TAG, "recvfrom failed - %d: %s", errno, strerror(errno));
          break;
        }
        // Data received
        else {
          // Get the sender's ip address as string
          auto sender_info = SocketUtils::sender_address_to_info(source_addr);
          receive_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
          ESP_LOGI(TAG, "Received %d bytes from %s:%d",
                   len, sender_info.address.c_str(), sender_info.port);
          // execute the callback since we've received a message
          std::string message{receive_buffer};
          if (on_receive_callback) {
            auto optional_response = on_receive_callback(message, sender_info);
            if (optional_response.has_value()) {
              // the callback has returned a response to send
              // back to the client
              auto response = optional_response.value();
              ESP_LOGI(TAG, "Sending response '%s' back to client.", response.c_str());
              int sent_len = sendto(socket_,
                                    response.c_str(),
                                    response.size(),
                                    0,
                                    (struct sockaddr *)&source_addr,
                                    socklen);
              if (sent_len < 0) {
                ESP_LOGE(TAG, "Could not respond to client - %d: %s", errno, strerror(errno));
                break;
              }
            }
          }
        }
      }
      // we've gotten here which means we cannot receive / send
      // anymore, so close the socket
      close_socket();
    }
}

bool UdpServer::Config::is_valid() const {
  if (port == 0) {
    // port is size_t so it must be >= 0, but we do not allow 0 port
    ESP_LOGE(TAG, "UdpServer::Config has port=0");
    return false;
  }
  if (receive_buffer_size == 0) {
    // we are a server, we require that we can receive data
    ESP_LOGE(TAG, "UdpServer::Config has receive_buffer_size=0");
    return false;
  }
  if (is_multicast_endpoint) {
    if (multicast_group.size() == 0) {
      // they've said that this is a multicast endpoint, but have not
      // provided a multicast group!
      ESP_LOGE(TAG, "UdpServer::Config has is_multicast_endpoint=true, "
               "but has not provided a valid multicast_group!");
      return false;
    }
    if (!SocketUtils::validate_address_string(multicast_group)) {
      // they've said that this is a multicast endpoint, but have not
      // provided a valid multicast group!
      ESP_LOGE(TAG, "UdpServer::Config has is_multicast_endpoint=true, "
               "but provided multicast_group '%s' is invalid!",
               multicast_group.c_str());
      return false;
    }
  }
  if (!on_receive_callback) {
    ESP_LOGW(TAG, "UdpServer::Config has invalid on_receive_callback"
             " - server data will not be passed to any functions!");
  }
  return true;
}

bool UdpServer::start(Task::Config& task_config, const UdpServer::Config& server_config) {
  // validate the server config
  if (!server_config.is_valid()) {
    ESP_LOGE(TAG, "Could not start UdpServer, server_config invalid!");
    return false;
  }
  // set the callback function for the config to be our member
  // function
  task_config.on_task_callback = std::bind(&UdpServer::task_function, this, server_config);
  // make sure the server keeps running
  running_ = true;
  // now start the task
  return task_->start(task_config);
}
