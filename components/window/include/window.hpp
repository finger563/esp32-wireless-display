#pragma once

#include "display.hpp"

namespace display {

  class Window {
  public:
    Window( int l, int r, int t, int b ) : left(l), right(r), top(t), bottom(b) {}

    int  width  ( void ) { return (right - left + 1); }
    int  height ( void ) { return (bottom - top + 1); }

    void clear  ( void ) { clear_vram( left, top, width(), height() ); }

  protected:

    int left   = 0;
    int right  = CONFIG_DISPLAY_WIDTH;
    int top    = 0;
    int bottom = CONFIG_DISPLAY_HEIGHT;
  };

}
