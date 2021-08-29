#pragma once

#include "window.hpp"

namespace display {
  class GraphWindow : public Window {
    public:
    GraphWindow( int l, int r, int t, int b ) : Window(l, r, t, b) {}

    struct Plot {
      static const size_t max_data_length = CONFIG_DISPLAY_WIDTH / CONFIG_WINDOW_PLOT_X_SPACING;

      std::string name;
      char        color;
      int         range;
      int         min;
      int         max;
      int         data [ max_data_length ];

      void init   ( const std::string& newName = "" );
      void update ( void );
      void shift  ( int newData );
      void shift  ( void );
    };

    void shift_plots   ( void ); // left shifts each plot by 1 element
    void clear_plots   ( void );
    void draw_plots    ( void );
    void draw_plot     ( Plot* plot );
    void add_data      ( std::string& plotName, int newData );
    int  create_plot   ( std::string& plotName, bool overWrite = false );
    void remove_plot   ( std::string& plotName );
    void remove_plot   ( int index );

    protected:
    int      get_plot_index  ( std::string& plotName );
    Plot*    get_plot        ( std::string& plotName );
    bool     has_plot        ( std::string& plotName );

    private:
    Plot _plots[ CONFIG_WINDOW_MAX_PLOTS ];
    int  _num_plots = 0;
   };
}
