#ifndef __DisplayTask__INCLUDE_GUARD
#define __DisplayTask__INCLUDE_GUARD

#include <cstdint>

// Task Includes
#include "Display.hpp"
#include <string.h>

// Generated state functions and members for the task
namespace DisplayTask {

  // Task Forward Declarations
  extern bool       changeState;

  class Window {
  public:
    Window( void ) : left(0), right(DISPLAY_WIDTH), top(0), bottom(DISPLAY_HEIGHT) {}
    Window( int l, int r, int t, int b ) : left(l), right(r), top(t), bottom(b) {}
    
    int left;
    int right;
    int top;
    int bottom;
    
    int  width  ( void ) { return (right - left); }
    int  height ( void ) { return (bottom - top); }
    
    void clear  ( void ) { clear_vram( left, top, width(), height() ); }
  };

  class GraphDisplay : public Window {
    public:
   };

  class TextDisplay {
    public:
  };

  // Generated task function
  void  taskFunction ( void *pvParameter );

  // Generated state functions
  void  state_Write_Text_execute      ( void );
  void  state_Write_Text_setState     ( void );
  void  state_Write_Text_transition   ( void );
  void  state_Write_Text_finalization ( void );
  void  state_Draw_Circle_execute      ( void );
  void  state_Draw_Circle_setState     ( void );
  void  state_Draw_Circle_transition   ( void );
  void  state_Draw_Circle_finalization ( void );
  void  state_Clear_Screen_execute      ( void );
  void  state_Clear_Screen_setState     ( void );
  void  state_Clear_Screen_transition   ( void );
  void  state_Clear_Screen_finalization ( void );
  void  state_Draw_Square_execute      ( void );
  void  state_Draw_Square_setState     ( void );
  void  state_Draw_Square_transition   ( void );
  void  state_Draw_Square_finalization ( void );
  void  state_Draw_Line_execute      ( void );
  void  state_Draw_Line_setState     ( void );
  void  state_Draw_Line_transition   ( void );
  void  state_Draw_Line_finalization ( void );

};

#endif // __DisplayTask__INCLUDE_GUARD
