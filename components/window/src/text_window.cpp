#include "text_window.hpp"

using namespace display;

void TextWindow::init( void ) {
  clear_logs();
}

void TextWindow::clear_logs( void ) {
  _logs = std::deque<std::string>();
  _log_colors = std::deque<uint8_t>();
  // add in empty so that we have blank space & layout things properly
  for (int i=0; i<max_logs; i++) {
    _logs.push_back( " " );
    _log_colors.push_back( 0xFF );
  }
}

void TextWindow::add_log( const std::string& newLog, const uint8_t logColor ) {
  // remove the old and shift to make space
  _logs.pop_front();
  _log_colors.pop_front();
  _logs.push_back( newLog );
  _log_colors.push_back( logColor );
}

void TextWindow::draw_logs( void ) {
  // print the newest logs at the bottom of the window
  int x = left, y = bottom;
  // iterate from the back (newest) as we print up (bottom->top) of
  // the window, with each log starting on the left of the window
  for (int i=_logs.size() - 1; i>=0; i--) {
    // if we're out of log space, stop writing logs! (these would be the oldest)
    if (y <= top) {
      break;
    }
    y -= log_height;
    // use one of our two fonts depending on the log height specified
    if (log_height < 12) {
      Draw_5x8_string( (char *)_logs[i].c_str(), _logs[i].length(), x, y, _log_colors[i]);
    } else {
      Draw_8x12_string( (char *)_logs[i].c_str(), _logs[i].length(), x, y, _log_colors[i]);
    }
  }
}

