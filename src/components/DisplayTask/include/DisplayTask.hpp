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
    Window( int l, int r, int t, int b ) : left(l), right(r), top(t), bottom(b) {}

    int  width  ( void ) { return (right - left); }
    int  height ( void ) { return (bottom - top); }
    
    void clear  ( void ) { clear_vram( left, top, width(), height() ); }
    
    protected:
    
    int left   = 0;
    int right  = DISPLAY_WIDTH;
    int top    = 0;
    int bottom = DISPLAY_HEIGHT;
  };

  class GraphDisplay : public Window {
    public:
    GraphDisplay( int l, int r, int t, int b ) : Window(l, r, t, b) {}
    
    #define MAX_PLOT_NAME_LEN 15
    #define MIN_X_SPACING     12
    #define MAX_PLOT_DATA_LEN (DISPLAY_WIDTH / MIN_X_SPACING)
    #define MAX_PLOTS         10

    struct Plot {
      enum class DataType { Invalid, Integer, Float };
      char     name[ MAX_PLOT_NAME_LEN ];
      char     color;
      int      range_i;
      float    range_f;
      DataType type;
      union {
        int   data_i [ MAX_PLOT_DATA_LEN ];
        float data_f [ MAX_PLOT_DATA_LEN ];
      };
      
      void shift_i ( int newData );
      void shift_f ( float newData );
      void shift   ( void );
    };
    
    void shiftPlots   ( void ); // left shifts each plot by 1 element
    void drawPlots    ( void );
    void drawPlot     ( Plot* plot );
    void addIntData   ( const char *plotName, int newData );
    void addFloatData ( const char *plotName, float newData );
    bool createPlot   ( const char *plotName, bool overWrite = false );
    void removePlot   ( const char *plotName );
    
    protected:
    int      getPlotIndex  ( const char *plotName );
    Plot*    getPlot       ( const char *plotName );
    bool     hasPlot       ( const char *plotName );
    
    private:
    Plot _plots[ MAX_PLOTS ];
    int  _numPlots = 0;
   };

  class TextDisplay : public Window {
    public:
    TextDisplay( int l, int r, int t, int b ) : Window(l, r, t, b) {}
    
    #define MAX_NUM_LOGS  3
    #define MAX_LOG_LEN   10
    
    private:
    char _logData[ MAX_NUM_LOGS ][ MAX_LOG_LEN ];
  };

  extern GraphDisplay graphDisplay;
  extern TextDisplay  debugDisplay;


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
