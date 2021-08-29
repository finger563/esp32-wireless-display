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
