#pragma once

#include <string>

#include "sdkconfig.h"

#include <cstring>

#include "window.hpp"

namespace display {
  class GraphWindow : public Window {
    public:
    GraphWindow( int l, int r, int t, int b ) : Window(l, r, t, b) {}

    struct Plot {
      static const size_t max_data_length = CONFIG_WINDOW_PLOT_MAX_DATA_LENGTH;

      std::string name{""};
      char        color{(char)(rand()%256)};
      int         range{1};
      int         min{0};
      int         max{0};
      int         data [ max_data_length ] = {0};

      Plot() {}
      Plot(const std::string& plot_name): name(plot_name) {}

      void update ( void );
      void shift  ( int newData );
      void shift  ( void );
    };

    void shift_plots   ( void ); // left shifts each plot by 1 element
    void clear_plots   ( void );
    void draw_plots    ( void );
    void draw_plot     ( const Plot* plot );
    void add_data      ( const std::string& plotName, int newData );
    int  create_plot   ( const std::string& plotName );
    void remove_plot   ( const std::string& plotName );
    void remove_plot   ( int index );

    protected:
    int      get_plot_index  ( const std::string& plotName );
    Plot*    get_plot        ( const std::string& plotName );
    bool     has_plot        ( const std::string& plotName );

    private:
    Plot _plots[ CONFIG_WINDOW_MAX_PLOTS ];
    int  _num_plots = 0;
   };
}
