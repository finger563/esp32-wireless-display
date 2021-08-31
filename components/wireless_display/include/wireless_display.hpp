#pragma once

#include <mutex>
#include <queue>
#include <sstream>

#include "sdkconfig.h"

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

  WirelessDisplay(int left, int right, int plot_height, int display_height):
    plot_display(left, right, 0, plot_height),
    log_display(left, right, plot_height + 1, display_height) {
    // initialize the display hardware
    ili9341_init();
    clear_vram();
    display_vram();
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
            log_display.clear_logs();
            // make sure we transition to the next state
            hasNewTextData = true;
          }
          else if (command == clearPlotsCommand) {
            plot_display.clear_plots();
            // make sure we transition to the next state
            hasNewPlotData = true;
          }
          else if ( (pos = line.find(removePlotCommand)) != std::string::npos) {
            plotName = line.substr(pos + removePlotCommand.length(), line.length());
            plot_display.remove_plot( plotName );
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
              plot_display.add_data( plotName, iValue );
              hasNewPlotData = true;
            }
            else {
              // couldn't find that, so we just have text data
              log_display.add_log( line );
              // make sure we transition to the next state
              hasNewTextData = true;
            }
          }
          else {
            // couldn't find that, so we just have text data
            log_display.add_log( line );
            // make sure we transition to the next state
            hasNewTextData = true;
          }
        }
      }
    }
    if (hasNewPlotData) {
      log_display.draw_logs();
      display_vram();
    }
    if (hasNewTextData) {
      plot_display.draw_plots();
      display_vram();
    }
    return hasNewPlotData || hasNewTextData;
  }

protected:
  display::GraphWindow plot_display;
  display::TextWindow log_display;

  std::queue<std::string> data_queue_;
  std::mutex data_queue_mutex_;
};
