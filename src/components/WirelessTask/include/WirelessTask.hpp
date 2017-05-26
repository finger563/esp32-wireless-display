#ifndef __WirelessTask__INCLUDE_GUARD
#define __WirelessTask__INCLUDE_GUARD

#include <cstdint>

// Task Includes
#include "DisplayTask.hpp"

extern "C" {
  #include "UDPServer.h"
}

// Generated state functions and members for the task
namespace WirelessTask {

  // Task Forward Declarations


  // Generated task function
  void  taskFunction ( void *pvParameter );

  // Generated state functions
  void  state_State_1_execute      ( void );
  void  state_State_1_setState     ( void );
  void  state_State_1_transition   ( void );
  void  state_State_1_finalization ( void );

};

#endif // __WirelessTask__INCLUDE_GUARD
