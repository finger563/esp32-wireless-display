#include "console_logger.hpp"

namespace phoenix {

ConsoleLogger::ConsoleLogger(const ConsoleLogger::Config &config)
    : buffer_max_size_(config.max_buffer_size_bytes),
      log_level_(config.log_level) {}

ConsoleLogger::ConsoleLogger(const ConsoleLogger &l)
    : buffer_max_size_(l.buffer_max_size_), buffer_(l.buffer_),
      log_level_(l.log_level_) {}

ConsoleLogger::~ConsoleLogger() {
  fmt::print("{}", buffer_);
  buffer_ = "";
}

} // namespace phoenix
