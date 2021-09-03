#include "text_window.hpp"

using namespace display;

void TextWindow::init( void ) {
  clear_logs();
}

void TextWindow::clear_logs( void ) {
  _logs.clear();
  _log_colors.clear();
}

void TextWindow::add_log( const std::string& newLog, const uint8_t logColor ) {
  // remove the old and shift to make space
  if (_logs.size() > max_logs) {
    _logs.pop_front();
  }
  if (_log_colors.size() > max_logs) {
    _log_colors.pop_front();
  }
  _logs.push_back( newLog );
  _log_colors.push_back( logColor );
}

void TextWindow::draw_logs( void ) {
  // print the newest logs at the bottom of the window
  int x = left, y = bottom;
  // iterate from the back (newest) as we print up (bottom->top) of
  // the window, with each log starting on the left of the window
  for (int i=_logs.size() - 1; i>=0; i--) {
    // we start drawing at the top of the character, so we need to
    // decrement the height first
    y -= log_height;
    // if we're out of log display space, stop writing logs! (these
    // would be the oldest)
    if (y <= top) {
      break;
    }
    // use one of our two fonts depending on the log height specified
    if (log_height < 12) {
      display_.get().draw_5x8_string( (char *)_logs[i].c_str(), _logs[i].length(), x, y, _log_colors[i]);
    } else {
      display_.get().draw_8x12_string( (char *)_logs[i].c_str(), _logs[i].length(), x, y, _log_colors[i]);
    }
  }
}

