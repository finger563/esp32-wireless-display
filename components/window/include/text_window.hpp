#pragma once

#include <deque>
#include <string>

#include "sdkconfig.h"
#include "window.hpp"

namespace display {
  class TextWindow : public Window {
    public:
    TextWindow( int l, int r, int t, int b ) : Window(l, r, t, b) {}

    static const int max_logs = CONFIG_WINDOW_MAX_TEXT_LOGS;
    static const int log_height = CONFIG_WINDOW_TEXT_LOG_HEIGHT;

    void init      ( void );
    void clear_logs( void );
    void add_log   ( const std::string& newLog, const uint8_t logColor = 0xFF );
    void draw_logs ( void );

    private:
    std::deque<std::string> _logs;
    std::deque<uint8_t>     _log_colors;
  };
}
