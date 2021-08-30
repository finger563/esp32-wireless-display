#pragma once
#include <cstring>
#include <mutex>
#include <string>
#define FMT_HEADER_ONLY
#include "fmt/color.h"
#include "fmt/format.h"

namespace phoenix {

class ConsoleLogger {
  std::size_t buffer_max_size_;
  std::string buffer_{""};
  std::mutex mutex_;

  template <class... Args>
  void out_(const fmt::color &color, std::string_view format_string,
            Args &&... args) {
    fmt::format_to(std::back_inserter(buffer_), format_string,
                   std::forward<Args>(args)...);
    buffer_ += "\n";
    const auto buffer_size = buffer_.size();
    const auto is_overflow = buffer_size > buffer_max_size_;
    if (is_overflow) {
      fmt::print(fg(color) | fmt::emphasis::bold, "{}", buffer_);
      buffer_ = "";
    }
  }

public:
  enum class LogLevel { info, warn, error };

private:
  LogLevel log_level_;

  bool is_log_level_enabled(const LogLevel &level) {
    return static_cast<std::size_t>(log_level_) >=
           static_cast<std::size_t>(level);
  }

public:
  struct Config {
    std::size_t max_buffer_size_bytes;
    LogLevel log_level;
  };
  ConsoleLogger(const Config &config = {.max_buffer_size_bytes = 10,
                                        .log_level = LogLevel::info});

  ConsoleLogger(const ConsoleLogger &l);

  ~ConsoleLogger();

  template <class... Args>
  void info(std::string_view format_string, Args &&... args) {
    if (!is_log_level_enabled(LogLevel::info)) {
      return;
    }
    std::unique_lock<std::mutex> lock{mutex_};
    out_(fmt::color::white, format_string, std::forward<Args>(args)...);
  }

  template <class... Args>
  void warn(std::string_view format_string, Args &&... args) {
    if (!is_log_level_enabled(LogLevel::warn)) {
      return;
    }
    std::unique_lock<std::mutex> lock{mutex_};
    out_(fmt::color::yellow, format_string, std::forward<Args>(args)...);
  }

  template <class... Args>
  void error(std::string_view format_string, Args &&... args) {
    if (!is_log_level_enabled(LogLevel::error)) {
      return;
    }
    std::unique_lock<std::mutex> lock{mutex_};
    out_(fmt::color::red, format_string, std::forward<Args>(args)...);
  }
};

} // namespace phoenix
