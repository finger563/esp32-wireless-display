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
    Display() {}
    // deleted so task structures cannot be copied
    Display(const Display&) = delete;

  public:

    // create a new task structure and return unique pointer to it
    static std::unique_ptr<Display> make_unique() {
      return std::unique_ptr<Display>(new Display());
    }

    void init() {
      // create the screen interface driver
#if CONFIG_DISPLAY_USE_SPI
      spi_config_t spi_cfg = {.miso_io_num=(gpio_num_t)CONFIG_DISPLAY_SPI_CIPO_PIN,
                              .mosi_io_num=(gpio_num_t)CONFIG_DISPLAY_SPI_COPI_PIN,
                              .sclk_io_num=(gpio_num_t)CONFIG_DISPLAY_SPI_CLK_PIN,
                              .max_transfer_sz = CONFIG_DISPLAY_WIDTH * CONFIG_DISPLAY_HEIGHT
      };
      auto spi_bus = spi_bus_create((spi_host_device_t)(CONFIG_DISPLAY_SPI_BUS-1), &spi_cfg);
      scr_interface_spi_config_t config = {.spi_bus=spi_bus,
                                           .pin_num_cs=CONFIG_DISPLAY_SPI_CS_PIN,
                                           .pin_num_dc=CONFIG_DISPLAY_DC_PIN,
                                           .clk_freq=(CONFIG_DISPLAY_SPI_CLOCK_MHZ*1000000),
                                           .swap_data=true
      };
      logger_.info("{}: making SPI interface", tag_);
      logger_.info("\tbus: {}, cs: {}, dc: {}, clk: {}",
                   (CONFIG_DISPLAY_SPI_BUS-1),
                   CONFIG_DISPLAY_SPI_CS_PIN,
                   CONFIG_DISPLAY_DC_PIN,
                   (CONFIG_DISPLAY_SPI_CLOCK_MHZ*1000000));
      ESP_ERROR_CHECK(scr_interface_create(SCREEN_IFACE_SPI, &config, &screen_interface_driver_));
#else
      // RM68120 uses 8080 (parallel) interface with i2s
      i2s_lcd_config_t i2s_lcd_cfg = {
                                      .data_width  = 16,
                                      .pin_data_num = {
#ifdef CONFIG_IDF_TARGET_ESP32
                                                       19, 21, 0, 22, 23, 33, 32, 27, 25, 26, 12, 13, 14, 15, 2, 4,
#else
                                                       // 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
                                                       1, 10, 2, 11, 3, 12, 4, 13, 5, 14, 6, 15, 7, 16, 8, 17,
#endif
                                                       },
                                      .pin_num_cs = -1,
#ifdef CONFIG_IDF_TARGET_ESP32
                                      .pin_num_wr = 18,
                                      .pin_num_rs = 5,
#else
                                      .pin_num_wr = 33,
                                      .pin_num_rs = 38,
#endif
                                      .clk_freq = 20000000,
                                      .i2s_port = I2S_NUM_0,
                                      .buffer_size = 32000,
                                      .swap_data = false,
      };

      scr_interface_driver_t *iface_drv_i2s;
      logger_.info("{}: making 8080 / I2S interface", tag_);
      ESP_ERROR_CHECK(scr_interface_create(SCREEN_IFACE_8080, &i2s_lcd_cfg, &screen_interface_driver_));
#endif
      // create the screen config
      logger_.info("{}: creating screen config", tag_);
      logger_.info("\treset: {}, backlight: {}, width: {}, height: {}",
                   CONFIG_DISPLAY_RST_PIN, CONFIG_DISPLAY_BACKLIGHT_PIN,
                   CONFIG_DISPLAY_WIDTH, CONFIG_DISPLAY_HEIGHT);
      screen_config_ = scr_controller_config_t {.interface_drv=screen_interface_driver_,
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
      // find the right driver for the screen based on the controller
      // we are talking to
#if CONFIG_DISPLAY_USE_ILI9341
      logger_.info("{}: USE CONTROLLER? '{}'", tag_, CONFIG_LCD_DRIVER_SCREEN_CONTROLLER_ILI9341);
      ESP_ERROR_CHECK(scr_find_driver(SCREEN_CONTROLLER_ILI9341, &screen_driver_));
#endif
#if CONFIG_DISPLAY_USE_RM68120
      ESP_ERROR_CHECK(scr_find_driver(SCREEN_CONTROLLER_RM68120, &screen_driver_));
#endif
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
    scr_interface_driver_t* screen_interface_driver_;
    scr_driver_t screen_driver_;
    uint8_t *vram;

    phoenix::ConsoleLogger logger_;
    const std::string tag_{"Display"};

  };
}

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
