#pragma once

#include <memory>
#include <mutex>
#include <stdint.h>

#include "sdkconfig.h"
#include "screen_driver.h"

#include "console_logger.hpp"

typedef struct s_point_s {
  uint16_t x;
  uint16_t y;
} point_s;

namespace display {
  class Display {

  private:
    // Private so only make_unique can be used
    Display(size_t width, size_t height);
    // deleted so task structures cannot be copied
    Display(const Display&) = delete;

  public:

    // create a new task structure and return unique pointer to it
    static std::unique_ptr<Display> make_unique(size_t width, size_t height) {
      return std::unique_ptr<Display>(new Display(width, height));
    }

    ~Display();

    void init();

    void draw_8x12_char(
                        char* _char_matrix,
                        int x_start,
                        int y_start,
                        unsigned char clr);
    void draw_8x12_string(
                          char* str,
                          unsigned char len,
                          int x_start,
                          int y_start,
                          unsigned char clr);
    void draw_5x8_char(
                       char* _char_matrix,
                       int x_start,
                       int y_start,
                       unsigned char clr);
    void draw_5x8_string(
                         char* str,
                         unsigned char len,
                         int x_start,
                         int y_start,
                         unsigned char clr);

    void draw_rectangle(const point_s& pos, const uint16_t width, const uint16_t height, const uint8_t outline, const uint8_t fill);
    void draw_circle(const point_s& pos, const uint16_t radius, const uint8_t outline, const uint8_t fill);
    void draw_line(const point_s& start, const point_s& end, const uint8_t color);

    void clear();
    void clear(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height);

    void update();

  protected:
    size_t width_=0;
    size_t height_=0;
    scr_controller_config_t screen_config_;
    scr_interface_driver_t* screen_interface_driver_;
    scr_driver_t screen_driver_;
    spi_bus_handle_t spi_bus_;
    uint8_t *vram;

    phoenix::ConsoleLogger logger_;
    const std::string tag_{"Display"};

    // helper functions for drawing circles
    void plot_4_points(int cx, int cy, int x, int y, unsigned char clroutline,unsigned char clrfill);
    void plot_8_points(int cx, int cy, int x, int y, unsigned char clroutline,unsigned char clrfill);
    void circle(int cx, int cy, int radius, unsigned char clroutline, unsigned char clrfill);

    size_t vram_size() { return width_ * height_ * sizeof(uint8_t); }
  };
}
