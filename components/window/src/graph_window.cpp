#include "graph_window.hpp"

using namespace display;

void GraphWindow::Plot::update( void ) {
  for (int i=0; i<max_data_length; i++) {
    if (data[i] > max)
      max = data[i];
    else if (data[i] < min)
      min = data[i];
  }
  range = max - min;
  if (range == 0) range = 1;
}

void GraphWindow::Plot::shift( int newData ) {
  shift();
  data[max_data_length - 1] = newData;
  update();
}

void GraphWindow::Plot::shift( void ) {
  for (int i=0; i<max_data_length-1; i++)
    data[i] = data[i + 1];
}

void GraphWindow::draw_plot( const Plot* plot ) {
  // draw line from (i-1) -> (i) for all points in the plot, so start
  // at second index (1)
  for (int i=1; i<Plot::max_data_length; i++) {
    display_.get().draw_line(
              { (uint16_t) (left+((i-1)*right)/Plot::max_data_length),
                (uint16_t) (bottom-((plot->data[i-1] - plot->min)*(bottom-top))/plot->range) },
              { (uint16_t) (left+(i*right)/Plot::max_data_length),
                (uint16_t) (bottom-((plot->data[i] - plot->min)*(bottom-top))/plot->range) },
              plot->color);
  }
}

void GraphWindow::shift_plots( void ) {
  for (int i=0; i<_num_plots; i++) {
    _plots[i].shift( 0 );
  }
}

void GraphWindow::clear_plots( void ) {
  _num_plots = 0;
}

void GraphWindow::draw_plots( void ) {
  for (int i=0; i<_num_plots; i++) {
    draw_plot(&_plots[i]);
  }
}

void GraphWindow::add_data( const std::string& plotName, int newData ) {
  int pi = get_plot_index( plotName );
  // couldn't find the plot so create a new one
  if ( pi == -1 )
    pi = create_plot( plotName );
  // we successfully found or created a plot
  if ( pi >= 0 )
    _plots[pi].shift( newData );
}

int GraphWindow::create_plot( const std::string& plotName ) {
  // we we have to create a new plot
  if ( _num_plots >= CONFIG_WINDOW_MAX_PLOTS) {
    // we're out of space, need to throw away the "oldest" plot (by
    // plot creation, not last updated). This will update _num_plots
    // so we can create a new one
    remove_plot(0);
  }
  // update the num_plots and create the new plot
  int index = _num_plots++;
  _plots[index] = Plot( plotName );
  return index;
}

void GraphWindow::remove_plot( const std::string& plotName ) {
  int index = get_plot_index( plotName );
  remove_plot( index );
}

void GraphWindow::remove_plot( int index ) {
  if (index > -1 && index < _num_plots) {
    for (int i=index; i<_num_plots; i++)
      _plots[i] = _plots[i+1];
    _num_plots--;
  }
}

GraphWindow::Plot* GraphWindow::get_plot( const std::string& plotName ) {
  int index = get_plot_index( plotName );
  if (index > -1)
    return &_plots[index];
  else
    return nullptr;
}

int GraphWindow::get_plot_index( const std::string& plotName ) {
  for (int i=0; i<_num_plots; i++) {
    if (_plots[i].name == plotName )
      return i;
  }
  return -1;
}

bool GraphWindow::has_plot( const std::string& plotName ) {
  return get_plot_index( plotName ) > -1;
}


