#ifndef __DisplayTask__INCLUDE_GUARD
#define __DisplayTask__INCLUDE_GUARD

#include <cstdint>

// Task Includes
#define _GLIBCXX_USE_C99 1    // needed for std::stoi

#include "Display.hpp"
#include <string.h>
#include <string>
#include <sstream>
#include <queue>
#include <deque>
#include <mutex>

extern "C" {  
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/event_groups.h"
  #include "freertos/semphr.h"
}

// Generated state functions and members for the task
namespace DisplayTask {

  // Task Forward Declarations
  extern bool       updateDone;
  extern bool       hasNewPlotData;
  extern bool       hasNewTextData;
  // for interacting sending data to the display
  extern std::queue<std::string> qData;
  extern SemaphoreHandle_t       qDataMutex;

  void        pushData ( std::string data );
  std::string popData  ( void );

  class Window {
    public:
    Window( int l, int r, int t, int b ) : left(l), right(r), top(t), bottom(b) {}

    int  width  ( void ) { return (right - left + 1); }
    int  height ( void ) { return (bottom - top + 1); }
    
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
    
    #define MAX_PLOT_NAME_LEN 100
    #define MIN_X_SPACING     12
    #define MAX_PLOT_DATA_LEN (DISPLAY_WIDTH / MIN_X_SPACING)
    #define MAX_PLOTS         10

    struct Plot {
      std::string name;
      char        color;
      int         range;
      int         min;
      int         max;
      int         data [ MAX_PLOT_DATA_LEN ];
      
      void init   ( const std::string& newName = "" );
      void update ( void );
      void shift  ( int newData );
      void shift  ( void );
    };
    
    void shiftPlots   ( void ); // left shifts each plot by 1 element
    void clearPlots   ( void );
    void drawPlots    ( void );
    void drawPlot     ( Plot* plot );
    void addData      ( std::string& plotName, int newData );
    int  createPlot   ( std::string& plotName, bool overWrite = false );
    void removePlot   ( std::string& plotName );
    void removePlot   ( int index );
    
    protected:
    int      getPlotIndex  ( std::string& plotName );
    Plot*    getPlot       ( std::string& plotName );
    bool     hasPlot       ( std::string& plotName );
    
    private:
    Plot _plots[ MAX_PLOTS ];
    int  _numPlots = 0;
   };

  class TextDisplay : public Window {
    public:
    TextDisplay( int l, int r, int t, int b ) : Window(l, r, t, b) {}
    
    static const int maxLogs = 7;
    static const int logHeight = 12;
    
    void init     ( void );
    void clearLogs( void );
    void addLog   ( const std::string& newLog );
    void drawLogs ( void );
    
    private:
    std::deque<std::string> _logs;
    int                     _numLogs = 0;
  };

  extern GraphDisplay graphDisplay;
  extern TextDisplay  debugDisplay;


  // Generated task function
  void  taskFunction ( void *pvParameter );

  // Generated state functions
  void  state_Update_Text_execute      ( void );
  void  state_Update_Text_setState     ( void );
  void  state_Update_Text_transition   ( void );
  void  state_Update_Text_finalization ( void );
  void  state_Update_Graph_execute      ( void );
  void  state_Update_Graph_setState     ( void );
  void  state_Update_Graph_transition   ( void );
  void  state_Update_Graph_finalization ( void );
  void  state_Wait_For_Data_execute      ( void );
  void  state_Wait_For_Data_setState     ( void );
  void  state_Wait_For_Data_transition   ( void );
  void  state_Wait_For_Data_finalization ( void );

};

#endif // __DisplayTask__INCLUDE_GUARD
