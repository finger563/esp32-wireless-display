#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <sstream>

#include "sdkconfig.h"

#include "display.hpp"
#include "text_window.hpp"
#include "graph_window.hpp"

class WirelessDisplay {
public:
  const std::string dataDelim = "::";
  const std::string commandDelim = "+++"; // should start with this for command
  const std::string shiftPlotCommand = "SHIFT PLOT:"; // followed by log name
  const std::string shiftPlotsCommand = "SHIFT PLOTS";
  const std::string removePlotCommand = "REMOVE PLOT:"; // followed by log name
  const std::string clearPlotsCommand = "CLEAR PLOTS";
  const std::string clearLogsCommand = "CLEAR LOGS";

  WirelessDisplay(size_t display_width, int display_height, int plot_height):
    display_(display::Display::make_unique(display_width, display_height)),
    plot_window_(*display_, 0, display_width, 0, plot_height),
    log_window_(*display_, 0, display_width, plot_height + 1, display_height) {
    display_->init();
    display_->clear();
  }

  void push_data(const std::string& data) {
    std::unique_lock<std::mutex> lock{data_queue_mutex_};
    data_queue_.push(data);
  }

  std::string pop_data() {
    std::string data{""};
    std::unique_lock<std::mutex> lock{data_queue_mutex_};
    if (!data_queue_.empty()) {
      data = data_queue_.front();
      data_queue_.pop();
    }
    return data;
  }

  bool update() {
    bool hasNewPlotData  = false;
    bool hasNewTextData  = false;

    std::string newData = pop_data();
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
            log_window_.clear_logs();
            // make sure we transition to the next state
            hasNewTextData = true;
          }
          else if (command == clearPlotsCommand) {
            plot_window_.clear_plots();
            // make sure we transition to the next state
            hasNewPlotData = true;
          }
          else if ( (pos = line.find(removePlotCommand)) != std::string::npos) {
            plotName = line.substr(pos + removePlotCommand.length(), line.length());
            plot_window_.remove_plot( plotName );
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
              plot_window_.add_data( plotName, iValue );
              hasNewPlotData = true;
            }
            else {
              // couldn't find that, so we just have text data
              log_window_.add_log( line );
              // make sure we transition to the next state
              hasNewTextData = true;
            }
          }
          else {
            // couldn't find that, so we just have text data
            log_window_.add_log( line );
            // make sure we transition to the next state
            hasNewTextData = true;
          }
        }
      }
    }
    if (hasNewPlotData) {
      log_window_.clear();
      log_window_.draw_logs();
    }
    if (hasNewTextData) {
      plot_window_.clear();
      plot_window_.draw_plots();
    }
    if (hasNewPlotData || hasNewTextData) {
      display_->update();
    }
    return hasNewPlotData || hasNewTextData;
  }

protected:
  std::unique_ptr<display::Display> display_;

  display::GraphWindow plot_window_;
  display::TextWindow log_window_;

  std::queue<std::string> data_queue_;
  std::mutex data_queue_mutex_;
};
