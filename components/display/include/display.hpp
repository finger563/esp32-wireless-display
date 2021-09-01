#pragma once

#include <memory>
#include <mutex>
#include <stdint.h>

#include "sdkconfig.h"

#include "screen_driver.h"

typedef struct s_point_s {
  uint16_t x;
  uint16_t y;
} point_s;

namespace display {
  class Display {

  private:



    // Private so only make_unique can be used
    Display() {
      // find the right driver for the screen based on the controller
      // we are talking to
#if DISPLAY_USE_ILI9341
      ESP_ERROR_CHECK(scr_find_driver(SCREEN_CONTROLLER_ILI9341, &screen_driver_));
#endif
#if DISPLAY_USE_RM68120
      ESP_ERROR_CHECK(scr_find_driver(SCREEN_CONTROLLER_RM68120, &screen_driver_));
#endif
      // create the screen interface driver
#if DISPLAY_USE_SPI
      scr_interface_spi_config_t config = {.spi_bus=DISPLAY_SPI_BUS,
                                           .pin_num_cs=DISPLAY_SPI_CS_PIN,
                                           .pin_num_dc=DISPLAY_DC_PIN,
                                           .clk_freq=DISPLAY_SPI_CLOCK_MHZ,
                                           .swap_data=false
      };
      ESP_ERROR_CHECK(scr_interface_create(SCREEN_IFACE_SPI, &config, &screen_interface_driver_));
#else
      // TODO: WE DON'T HANDLE THE 8080 INTERFACE YET
#endif
      // create the screen config
      screen_config_ = scr_controller_config_t {.interface_drv=&screen_interface_driver_,
                                                .pin_num_rst=CONFIG_DISPLAY_RST_PIN,
                                                .pin_num_bckl=CONFIG_DISPLAY_BACKLIGHT_PIN,
                                                .rst_active_level=0,
                                                .bckl_active_level=0,
                                                .width=CONFIG_DISPLAY_WIDTH,
                                                .height=CONFIG_DISPLAY_HEIGHT,
                                                .offset_hor=0,
                                                .offset_ver=0,
                                                .rotate=SCR_DIR_LRTB
      };
    }
    // deleted so task structures cannot be copied
    Display(const Display&) = delete;

  public:

    // create a new task structure and return unique pointer to it
    static std::unique_ptr<Display> make_unique() {
      return std::unique_ptr<Display>(new Display());
    }

    void init() {
      // initialize the driver for the controller on the specified
      // interface
      ESP_ERROR_CHECK(screen_driver_.init(&screen_config_));
    }

    void Draw_8x12_char(
                        char* _char_matrix,
                        int x_start,
                        int y_start,
                        unsigned char clr);
    void Draw_8x12_string(
                          char* str,
                          unsigned char len,
                          int x_start,
                          int y_start,
                          unsigned char clr);
    void Draw_5x8_char(
                       char* _char_matrix,
                       int x_start,
                       int y_start,
                       unsigned char clr);
    void Draw_5x8_string(
                         char* str,
                         unsigned char len,
                         int x_start,
                         int y_start,
                         unsigned char clr);

    void draw_rectangle(const point_s& pos, const uint16_t width, const uint16_t height, const uint8_t outline, const uint8_t fill);
    void draw_circle(const point_s& pos, const uint16_t radius, const uint8_t outline, const uint8_t fill);
    void draw_line(const point_s& start, const point_s& end, const uint8_t color);

  protected:
    scr_controller_config_t screen_config_;
    scr_interface_driver_t screen_interface_driver_;
    scr_driver_t screen_driver_;
    uint8_t *vram;
  };
}

// low level screen functions
void ili9341_init();

// VRAM functions
void clear_vram();
void clear_vram(
                const uint16_t x,
                const uint16_t y,
                const uint16_t width,
                const uint16_t height);
void display_vram();
void blit_vram(
               const uint16_t x,
               const uint16_t y,
               const uint16_t width,
               const uint16_t height);

// text functions
void Draw_8x12_char(
                    char* _char_matrix,
                    int x_start,
                    int y_start,
                    unsigned char clr);
void Draw_8x12_string(
                      char* str,
                      unsigned char len,
                      int x_start,
                      int y_start,
                      unsigned char clr);
void Draw_5x8_char(
                   char* _char_matrix,
                   int x_start,
                   int y_start,
                   unsigned char clr);
void Draw_5x8_string(
                     char* str,
                     unsigned char len,
                     int x_start,
                     int y_start,
                     unsigned char clr);

// drawing functions
void draw_rectangle(
                    const point_s  pos,
                    const uint16_t width,
                    const uint16_t height,
                    const uint8_t  outline,
                    const uint8_t  fill);
void draw_circle(
                 const point_s  pos,
                 const uint16_t radius,
                 const uint8_t  outline,
                 const uint8_t  fill);
void draw_line(
               const point_s start,
               const point_s end,
               const uint8_t color);
