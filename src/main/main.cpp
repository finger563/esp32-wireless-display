#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"

#define MS_TO_TICKS( xTimeInMs ) (uint32_t)( ( ( TickType_t ) xTimeInMs * configTICK_RATE_HZ ) / ( TickType_t ) 1000 )

// include the task components
#include "WirelessTask.hpp"
#include "SerialTask.hpp"
#include "DisplayTask.hpp"
// include the timer components

// now start the tasks that have been defined
extern "C" void app_main(void)
{
  // create the tasks
  xTaskCreate(&WirelessTask::taskFunction, // function the task runs
	      "taskFunction_0", // name of the task (should be short)
	      2048, // stack size for the task
	      NULL, // parameters to task
	      0, // priority of the task (higher -> higher priority)
	      NULL // returned task object (don't care about storing it)
	      );
  xTaskCreate(&SerialTask::taskFunction, // function the task runs
	      "taskFunction_1", // name of the task (should be short)
	      2048, // stack size for the task
	      NULL, // parameters to task
	      0, // priority of the task (higher -> higher priority)
	      NULL // returned task object (don't care about storing it)
	      );
  xTaskCreate(&DisplayTask::taskFunction, // function the task runs
	      "taskFunction_2", // name of the task (should be short)
	      2048, // stack size for the task
	      NULL, // parameters to task
	      0, // priority of the task (higher -> higher priority)
	      NULL // returned task object (don't care about storing it)
	      );
}

