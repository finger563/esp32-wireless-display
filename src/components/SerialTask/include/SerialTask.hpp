#ifndef __SerialTask__INCLUDE_GUARD
#define __SerialTask__INCLUDE_GUARD

#include <cstdint>

// Task Includes
#include "driver/gpio.h" // needed for printf
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "soc/uart_struct.h"

// Needed to be able to update each task's changeState
#include "DisplayTask.hpp"


// Generated state functions and members for the task
namespace SerialTask {

  // Task Forward Declarations
  extern bool changeState;

  // Generated task function
  void  taskFunction ( void *pvParameter );

  // Generated state functions
  void  state_State_2_execute      ( void );
  void  state_State_2_setState     ( void );
  void  state_State_2_transition   ( void );
  void  state_State_2_finalization ( void );
  void  state_State_2_State_execute      ( void );
  void  state_State_2_State_setState     ( void );
  void  state_State_2_State_transition   ( void );
  void  state_State_2_State_finalization ( void );
  void  state_State_1_execute      ( void );
  void  state_State_1_setState     ( void );
  void  state_State_1_transition   ( void );
  void  state_State_1_finalization ( void );

};

#endif // __SerialTask__INCLUDE_GUARD
