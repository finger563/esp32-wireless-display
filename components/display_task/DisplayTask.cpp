#include "DisplayTask.hpp"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MS_TO_TICKS( xTimeInMs ) (uint32_t)( ( ( TickType_t ) xTimeInMs * configTICK_RATE_HZ ) / ( TickType_t ) 1000 )

namespace DisplayTask {

  // User definitions for the task
  bool updateDone      = false;
  bool hasNewPlotData  = false;
  bool hasNewTextData  = false;
  // for sending data to the display
  std::queue<std::string> qData;
  SemaphoreHandle_t       qDataMutex = NULL;

  void pushData ( std::string data ) {
    if ( xSemaphoreTake( qDataMutex, ( TickType_t ) 100 ) == pdTRUE ) {
      qData.push(data);
      xSemaphoreGive( qDataMutex );
    }
  }

  std::string popData  ( void ) {
    std::string retData = "";
    if ( xSemaphoreTake( qDataMutex, ( TickType_t ) 100 ) == pdTRUE ) {
      if (!qData.empty()) {
        retData = qData.front();
        qData.pop();
      }
      xSemaphoreGive( qDataMutex );
    }
    return retData;
  }

  int graphHeight = DISPLAY_HEIGHT * 2 / 3;
  int debugHeight = DISPLAY_HEIGHT - graphHeight;

  GraphDisplay graphDisplay( 0, DISPLAY_WIDTH, 0, graphHeight );
  TextDisplay  debugDisplay( 0, DISPLAY_WIDTH, graphHeight + 1, DISPLAY_HEIGHT );

  // Graph Display

  // TextDisplay

  // Generated state variables
  bool     __change_state__ = false;
  uint32_t __state_delay__ = 0;
  uint8_t  stateLevel_0;

  // Generated task function
  void taskFunction ( void *pvParameter ) {
    // initialize here
    __change_state__ = false;
    __state_delay__ = 100;
    state_Wait_For_Data_setState();
    // execute the init transition for the initial state and task
    qDataMutex = xSemaphoreCreateMutex();

    debugDisplay.init();

    ili9341_init();
    clear_vram();
    display_vram();

    // now loop running the state code
    while (true) {
      // reset __change_state__ to false
      __change_state__ = false;
      // run the proper state function
      state_Update_Text_execute();
      state_Update_Graph_execute();
      state_Wait_For_Data_execute();
      // now wait if we haven't changed state
      if (!__change_state__) {
        vTaskDelay( MS_TO_TICKS(__state_delay__) );
      }
      else {
        vTaskDelay( MS_TO_TICKS(1) );
      }
    }
  }

  // Generated state functions
  const uint8_t state_Update_Text = 0;

  void state_Update_Text_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_Update_Text)
      return;

    state_Update_Text_transition();

    // execute all substates

    if (!__change_state__) {
      debugDisplay.drawLogs();
      display_vram();
      updateDone = true;
    }
  }

  void state_Update_Text_setState( void ) {
    stateLevel_0 = state_Update_Text;
  }

  void state_Update_Text_transition( void ) {
    if (__change_state__)
      return;
    else if ( updateDone ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Update_Text_finalization();
      // set the current state to the state we are transitioning to
      state_Wait_For_Data_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function
      updateDone = false;

    }
  }

  void state_Update_Text_finalization( void ) {

  }

  const uint8_t state_Update_Graph = 1;

  void state_Update_Graph_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_Update_Graph)
      return;

    state_Update_Graph_transition();

    // execute all substates

    if (!__change_state__) {
      graphDisplay.drawPlots();
      display_vram();
      updateDone = true;
    }
  }

  void state_Update_Graph_setState( void ) {
    stateLevel_0 = state_Update_Graph;
  }

  void state_Update_Graph_transition( void ) {
    if (__change_state__)
      return;
    else if ( updateDone ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Update_Graph_finalization();
      // set the current state to the state we are transitioning to
      state_Wait_For_Data_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function
      updateDone = false;

    }
  }

  void state_Update_Graph_finalization( void ) {

  }

  const uint8_t state_Wait_For_Data = 2;

  void state_Wait_For_Data_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_Wait_For_Data)
      return;

    state_Wait_For_Data_transition();

    // execute all substates

    if (!__change_state__) {
      static const std::string dataDelim = "::";
      static const std::string commandDelim = "+++"; // should start with this for command
      static const std::string shiftPlotCommand = "SHIFT PLOT:"; // followed by log name
      static const std::string shiftPlotsCommand = "SHIFT PLOTS";
      static const std::string removePlotCommand = "REMOVE PLOT:"; // followed by log name
      static const std::string clearPlotsCommand = "CLEAR PLOTS";
      static const std::string clearLogsCommand = "CLEAR LOGS";

      std::string newData = popData();
      int len = newData.length();
      if(len > 0) {
        // have data, parse here
        std::stringstream ss(newData);
        std::string line;
        while (std::getline(ss, line, '\n')) {
          size_t pos = 0;
          // parse for commands
          if ( (pos = line.find(commandDelim)) != std::string::npos) {
            std::string command;
            std::string plotName;
            command = line.substr(pos + commandDelim.length(), line.length());
            if (command == clearLogsCommand) {
              debugDisplay.clearLogs();
              // make sure we transition to the next state
              hasNewTextData = true;
            }
            else if (command == clearPlotsCommand) {
              graphDisplay.clearPlots();
              // make sure we transition to the next state
              hasNewPlotData = true;
            }
            else if ( (pos = line.find(removePlotCommand)) != std::string::npos) {
              plotName = line.substr(pos + removePlotCommand.length(), line.length());
              graphDisplay.removePlot( plotName );
              // make sure we transition to the next state
              hasNewPlotData = true;
            }
          }
          else {
            // parse for data
            if ( (pos = line.find(dataDelim)) != std::string::npos) {
              // found "::" so we have a plot data
              std::string plotName;
              std::string value;
              int         iValue;
              plotName = line.substr(0, pos);
              pos = pos + dataDelim.length();
              if ( pos < line.length() ) {
                value = line.substr(pos, line.length());
                iValue = std::stoi(value);
                // make sure we transition to the next state
                graphDisplay.addData( plotName, iValue );
                hasNewPlotData = true;
              }
              else {
                // couldn't find that, so we just have text data
                debugDisplay.addLog( line );
                // make sure we transition to the next state
                hasNewTextData = true;
              }
            }
            else {
              // couldn't find that, so we just have text data
              debugDisplay.addLog( line );
              // make sure we transition to the next state
              hasNewTextData = true;
            }
          }
        }
      }
    }
  }

  void state_Wait_For_Data_setState( void ) {
    stateLevel_0 = state_Wait_For_Data;
  }

  void state_Wait_For_Data_transition( void ) {
    if (__change_state__)
      return;
    else if ( hasNewTextData ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Wait_For_Data_finalization();
      // set the current state to the state we are transitioning to
      state_Update_Text_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function
      hasNewTextData = false;
          debugDisplay.clear();

    }
    else if ( hasNewPlotData ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Wait_For_Data_finalization();
      // set the current state to the state we are transitioning to
      state_Update_Graph_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function
      hasNewPlotData = false;
          graphDisplay.clear();

    }
  }

  void state_Wait_For_Data_finalization( void ) {

  }

 
};
