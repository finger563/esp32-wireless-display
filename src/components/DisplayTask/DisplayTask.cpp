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

  uint8_t data[BUF_SIZE];

  int graphHeight = DISPLAY_HEIGHT * 2 / 3;
  int debugHeight = DISPLAY_HEIGHT - graphHeight;

  GraphDisplay graphDisplay( 0, DISPLAY_WIDTH, 0, graphHeight );
  TextDisplay  debugDisplay( 0, DISPLAY_WIDTH, graphHeight + 1, DISPLAY_HEIGHT );

  // Graph Display

  void GraphDisplay::Plot::init( const std::string& newName ) {
    name = newName;
    color = rand() % 256;
    range = 1;
    min = 0;
    max = 0;
    memset( data, 0, MAX_PLOT_DATA_LEN );
  }

  void GraphDisplay::Plot::update( void ) {
    for (int i=0; i<MAX_PLOT_DATA_LEN; i++) {
      if (data[i] > max)
        max = data[i];
      else if (data[i] < min)
        min = data[i];
    }
    range = max - min;
    if (range == 0) range = 1;
  }

  void GraphDisplay::Plot::shift( int newData ) {
    shift();
    data[MAX_PLOT_DATA_LEN - 1] = newData;
    update();
  }

  void GraphDisplay::Plot::shift( void ) {
    for (int i=0; i<MAX_PLOT_DATA_LEN-1; i++)
      data[i] = data[i + 1];
  }

  void GraphDisplay::drawPlot( GraphDisplay::Plot* plot ) {
    for (int i=1; i<MAX_PLOT_DATA_LEN; i++) {
      draw_line(
        { (uint16_t) (left+((i-1)*right)/MAX_PLOT_DATA_LEN),
         (uint16_t) (bottom-((plot->data[i-1] - plot->min)*(bottom-top))/plot->range) },
        { (uint16_t) (left+(i*right)/MAX_PLOT_DATA_LEN),
         (uint16_t) (bottom-((plot->data[i] - plot->min)*(bottom-top))/plot->range) },
        plot->color);
    }
  }

  void GraphDisplay::shiftPlots( void ) {
    for (int i=0; i<_numPlots; i++) {
      _plots[i].shift( 0 );
    }
  }

  void GraphDisplay::clearPlots( void ) {
    _numPlots = 0;
  }

  void GraphDisplay::drawPlots( void ) {
    for (int i=0; i<_numPlots; i++) {
      drawPlot(&_plots[i]);
    }
  }

  void GraphDisplay::addData( std::string& plotName, int newData ) {
    int pi = getPlotIndex( plotName );
    if ( pi == -1 )
      pi = createPlot( plotName, true );
    _plots[pi].shift( newData );
  }

  int GraphDisplay::createPlot( std::string& plotName, bool overWrite ) {
    int index = getPlotIndex( plotName );
    if (index > -1) {
      if ( overWrite ) {
        // will overwrite plot that has the same name with empty plot
        _plots[index].init( plotName );
      }
      return index;
    }
    else {
      if ( _numPlots < MAX_PLOTS ) {
        index = _numPlots++;
        _plots[index].init( plotName );
        return index;
      }
      else if ( overWrite ) {
        index = 0;
        _plots[index].init( plotName );
        return index;
      }
    }
    return -1;
  }

  void GraphDisplay::removePlot( std::string& plotName ) {
    int index = getPlotIndex( plotName );
    removePlot( index );
  }

  void GraphDisplay::removePlot( int index ) {
    if (index > -1) {
      for (int i=index; i<_numPlots; i++)
        _plots[i] = _plots[i+1];
      _numPlots--;
    }
  }

  GraphDisplay::Plot* GraphDisplay::getPlot( std::string& plotName ) {
    int index = getPlotIndex( plotName );
    if (index > -1)
      return &_plots[index];
    else
      return nullptr;
  }

  int GraphDisplay::getPlotIndex( std::string& plotName ) {
    for (int i=0; i<_numPlots; i++) {
      if (_plots[i].name == plotName )
        return i;
    }
    return -1;
  }

  bool GraphDisplay::hasPlot( std::string& plotName ) {
    return getPlotIndex( plotName ) > -1;
  }

  // TextDisplay

  void TextDisplay::init( void ) {
    clearLogs();
  }

  void TextDisplay::clearLogs( void ) {
    _logs = std::deque<std::string>();
    for (int i=0; i<maxLogs; i++)
      _logs.push_front( " " );
  }

  void TextDisplay::addLog( const std::string& newLog ) {
    _logs.pop_front();
    _logs.push_back( newLog );
  }

  void TextDisplay::drawLogs( void ) {
    int x = left, y = top;
    for (int i=0; i<maxLogs; i++) {
      y += logHeight;
      Draw_8x12_string( (char *)_logs[i].c_str(), _logs[i].length(), x, y, 0xFF);
    }
  }

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

      /*
      graphDisplay.clear();
      graphDisplay.shiftPlots();
      graphDisplay.drawPlots();
      display_vram();
      */

      memset(data, 0, BUF_SIZE);
      int len = uart_read_bytes(EX_UART_NUM, data, BUF_SIZE, 0);//100 / portTICK_RATE_MS);
      if(len > 0) {
        // have data, parse here
        std::stringstream ss((char *)data);
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
