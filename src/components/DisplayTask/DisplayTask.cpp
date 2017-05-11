#include "DisplayTask.hpp"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MS_TO_TICKS( xTimeInMs ) (uint32_t)( ( ( TickType_t ) xTimeInMs * configTICK_RATE_HZ ) / ( TickType_t ) 1000 )

namespace DisplayTask {

  // User definitions for the task
  bool    changeState = false;

  uint8_t color = 0xFF;

  int graphHeight = DISPLAY_HEIGHT * 2 / 3;
  int debugHeight = DISPLAY_HEIGHT - graphHeight;

  GraphDisplay graphDisplay( 0, DISPLAY_WIDTH, 0, graphHeight );
  TextDisplay  debugDisplay( 0, DISPLAY_WIDTH, graphHeight, DISPLAY_HEIGHT );

  void GraphDisplay::Plot::shift( void ) {
    if (type == DataType::Integer) {
      for (int i=0; i<MAX_PLOT_DATA_LEN-1; i++)
        data_i[i] = data_i[i + 1];
    }
    else if (type == DataType::Float) {
      for (int i=0; i<MAX_PLOT_DATA_LEN-1; i++)
        data_f[i] = data_f[i + 1];
    }
  }

  void GraphDisplay::drawPlot( GraphDisplay::Plot* plot ) {
    if (plot->type == Plot::DataType::Integer) {
      for (int i=1; i<MAX_PLOT_DATA_LEN; i++) {
        draw_line(
          { (uint16_t) (left+((i-1)*right)/MAX_PLOT_DATA_LEN),
           (uint16_t) (bottom-(plot->data_i[i-1]*(bottom-top))/plot->range_i) },
          { (uint16_t) (left+(i*right)/MAX_PLOT_DATA_LEN),
           (uint16_t) (bottom-(plot->data_i[i]*(bottom-top))/plot->range_i) },
          plot->color);
      }
    }
    else if (plot->type == Plot::DataType::Float) {
      for (int i=1; i<MAX_PLOT_DATA_LEN; i++) {
        draw_line( 
          { (uint16_t) (left+((i-1)*(right-left))/MAX_PLOT_DATA_LEN),
           (uint16_t) (top +(plot->data_f[i-1]*(bottom-top))/plot->range_f) },
          { (uint16_t) (left+(i*(right-left))/MAX_PLOT_DATA_LEN),
           (uint16_t) (top +(plot->data_f[i]*(bottom-top))/plot->range_f) },
          plot->color);
      }
    }
  }

  void GraphDisplay::shiftPlots( void ) {
    for (int i=0; i<_numPlots; i++) {
      _plots[i].shift();
    }
  }

  void GraphDisplay::drawPlots( void ) {
    for (int i=0; i<_numPlots; i++) {
      drawPlot(&_plots[i]);
    }
  }

  void GraphDisplay::addIntData( const char *plotName, int newData ) {
  }

  void GraphDisplay::addFloatData( const char *plotName, float newData ) {
  }

  bool GraphDisplay::createPlot( const char *plotName, bool overWrite ) {
    int index = getPlotIndex( plotName );
    if (index > -1) {
      if ( overWrite ) {
        // will overwrite plot that has the same name with empty plot
        return true;
      }
    }
    else {
      if ( _numPlots < MAX_PLOTS ) {
        return true;
      }
      else if ( overWrite ) {
        // will delete the oldest plot by creation time ( _plots[0] )
        return true;
      }
    }
    return false;
  }

  void GraphDisplay::removePlot( const char *plotName ) {
    int index = getPlotIndex( plotName );
    if (index > -1) {
      for (int i=index; i<_numPlots; i++)
        _plots[i] = _plots[i+1];
    }
  }

  GraphDisplay::Plot* GraphDisplay::getPlot( const char *plotName ) {
    int index = getPlotIndex( plotName );
    if (index > -1)
      return &_plots[index];
    else
      return nullptr;
  }

  int GraphDisplay::getPlotIndex( const char *plotName ) {
    for (int i=0; i<_numPlots; i++) {
      if (!strcmp( _plots[i].name, plotName ))
        return i;
    }
    return -1;
  }

  bool GraphDisplay::hasPlot( const char *plotName ) {
    return getPlotIndex( plotName ) > -1;
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
    state_Draw_Square_setState();
    // execute the init transition for the initial state and task
    ili9341_init();
    clear_vram();
    display_vram();

    // now loop running the state code
    while (true) {
      // reset __change_state__ to false
      __change_state__ = false;
      // run the proper state function
      state_Write_Text_execute();
      state_Draw_Circle_execute();
      state_Clear_Screen_execute();
      state_Draw_Square_execute();
      state_Draw_Line_execute();
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
  const uint8_t state_Write_Text = 0;

  void state_Write_Text_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_Write_Text)
      return;

    state_Write_Text_transition();

    // execute all substates

    if (!__change_state__) {
      static char *text = "Hello World";

      uint8_t color = rand() % 256;
      uint16_t x = rand() % (DISPLAY_WIDTH);
      uint16_t y = rand() % (DISPLAY_HEIGHT);

      Draw_8x12_string( text, strlen(text), x, y, color);
    }
  }

  void state_Write_Text_setState( void ) {
    stateLevel_0 = state_Write_Text;
  }

  void state_Write_Text_transition( void ) {
    if (__change_state__)
      return;
    else if ( changeState ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Write_Text_finalization();
      // set the current state to the state we are transitioning to
      state_Clear_Screen_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function
      changeState = false;
          clear_vram();

    }
  }

  void state_Write_Text_finalization( void ) {

  }

  const uint8_t state_Draw_Circle = 1;

  void state_Draw_Circle_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_Draw_Circle)
      return;

    state_Draw_Circle_transition();

    // execute all substates

    if (!__change_state__) {
      uint8_t size = rand() % 10;
      uint8_t color = rand() % 256;
      uint16_t x = rand() % (DISPLAY_WIDTH);
      uint16_t y = rand() % (DISPLAY_HEIGHT);
      draw_circle({x, y}, size, color & 0b10101010, color);
    }
  }

  void state_Draw_Circle_setState( void ) {
    stateLevel_0 = state_Draw_Circle;
  }

  void state_Draw_Circle_transition( void ) {
    if (__change_state__)
      return;
    else if ( changeState ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Draw_Circle_finalization();
      // set the current state to the state we are transitioning to
      state_Draw_Line_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function
      changeState = false;

    }
  }

  void state_Draw_Circle_finalization( void ) {

  }

  const uint8_t state_Clear_Screen = 2;

  void state_Clear_Screen_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_Clear_Screen)
      return;

    state_Clear_Screen_transition();

    // execute all substates

    if (!__change_state__) {
      static char *text = "Hello World";

      uint8_t color = rand() % 256;
      uint16_t x = rand() % (DISPLAY_WIDTH);
      uint16_t y = rand() % (DISPLAY_HEIGHT);

      Draw_8x12_string( text, strlen(text), x, y, color);
    }
  }

  void state_Clear_Screen_setState( void ) {
    stateLevel_0 = state_Clear_Screen;
  }

  void state_Clear_Screen_transition( void ) {
    if (__change_state__)
      return;
    else if ( true ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Clear_Screen_finalization();
      // set the current state to the state we are transitioning to
      state_Draw_Square_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function

    }
  }

  void state_Clear_Screen_finalization( void ) {

  }

  const uint8_t state_Draw_Square = 3;

  void state_Draw_Square_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_Draw_Square)
      return;

    state_Draw_Square_transition();

    // execute all substates

    if (!__change_state__) {
      uint8_t size = rand() % 20;
      uint8_t color = rand() % 256;
      uint16_t x = rand() % (DISPLAY_WIDTH);
      uint16_t y = rand() % (DISPLAY_HEIGHT);
      draw_rectangle({x, y}, size, size, color & 0b10101010, color);
    }
  }

  void state_Draw_Square_setState( void ) {
    stateLevel_0 = state_Draw_Square;
  }

  void state_Draw_Square_transition( void ) {
    if (__change_state__)
      return;
    else if ( changeState ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Draw_Square_finalization();
      // set the current state to the state we are transitioning to
      state_Draw_Circle_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function
      changeState = false;

    }
  }

  void state_Draw_Square_finalization( void ) {

  }

  const uint8_t state_Draw_Line = 4;

  void state_Draw_Line_execute( void ) {
    if (__change_state__ || stateLevel_0 != state_Draw_Line)
      return;

    state_Draw_Line_transition();

    // execute all substates

    if (!__change_state__) {
      uint8_t color = rand() % 256;
      uint16_t x1 = rand() % (DISPLAY_WIDTH);
      uint16_t y1 = rand() % (DISPLAY_HEIGHT);
      uint16_t x2 = rand() % (DISPLAY_WIDTH);
      uint16_t y2 = rand() % (DISPLAY_HEIGHT);
      draw_line({x1, y1}, {x2, y2}, color);
    }
  }

  void state_Draw_Line_setState( void ) {
    stateLevel_0 = state_Draw_Line;
  }

  void state_Draw_Line_transition( void ) {
    if (__change_state__)
      return;
    else if ( changeState ) {
      __change_state__ = true;
      // run the current state's finalization function
      state_Draw_Line_finalization();
      // set the current state to the state we are transitioning to
      state_Write_Text_setState();
      // start state timer (@ next states period)
      __state_delay__ = 100;
      // execute the transition function
      changeState = false;

    }
  }

  void state_Draw_Line_finalization( void ) {

  }

 
};
