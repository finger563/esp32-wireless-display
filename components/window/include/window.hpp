#pragma once

#include <functional>
#include "display.hpp"

namespace display {

  class Window {
  public:
    Window( std::reference_wrapper<Display> display, int l, int r, int t, int b ) : display_(display), left(l), right(r), top(t), bottom(b) {}

    int  width  ( void ) { return (right - left + 1); }
    int  height ( void ) { return (bottom - top + 1); }

    void clear  ( void ) { display_.get().clear( left, top, width(), height() ); }

  protected:
    std::reference_wrapper<Display> display_;

    int left   = 0;
    int right  = 0;
    int top    = 0;
    int bottom = 0;
  };

}
