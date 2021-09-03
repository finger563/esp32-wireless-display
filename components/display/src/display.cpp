#include "display.hpp"

#include <algorithm>
#include <string.h>

#include "fonts.hpp"

using namespace display;

uint16_t myPalette[256] = {
                           0,8,23,31,256,264,279,287,512,520,535,543,768,776,791,799,1248,1256,1271,1279,1504,
                           1512,1527,1535,1760,1768,1783,1791,2016,2024,2039,2047,8192,8200,8215,8223,8448,8456,
                           8471,8479,8704,8712,8727,8735,8960,8968,8983,8991,9440,9448,9463,9471,9696,9704,9719,
                           9727,9952,9960,9975,9983,10208,10216,10231,10239,16384,16392,16407,16415,16640,16648,
                           16663,16671,16896,16904,16919,16927,17152,17160,17175,17183,17632,17640,17655,17663,
                           17888,17896,17911,17919,18144,18152,18167,18175,18400,18408,18423,18431,24576,24584,
                           24599,24607,24832,24840,24855,24863,25088,25096,25111,25119,25344,25352,25367,25375,
                           25824,25832,25847,25855,26080,26088,26103,26111,26336,26344,26359,26367,26592,26600,
                           26615,26623,38912,38920,38935,38943,39168,39176,39191,39199,39424,39432,39447,39455,
                           39680,39688,39703,39711,40160,40168,40183,40191,40416,40424,40439,40447,40672,40680,
                           40695,40703,40928,40936,40951,40959,47104,47112,47127,47135,47360,47368,47383,47391,
                           47616,47624,47639,47647,47872,47880,47895,47903,48352,48360,48375,48383,48608,48616,
                           48631,48639,48864,48872,48887,48895,49120,49128,49143,49151,55296,55304,55319,55327,
                           55552,55560,55575,55583,55808,55816,55831,55839,56064,56072,56087,56095,56544,56552,
                           56567,56575,56800,56808,56823,56831,57056,57064,57079,57087,57312,57320,57335,57343,
                           63488,63496,63511,63519,63744,63752,63767,63775,64000,64008,64023,64031,64256,64264,
                           64279,64287,64736,64744,64759,64767,64992,65000,65015,65023,65248,65256,65271,65279,
                           65504,65512,65527,65535
};

Display::Display(size_t width, size_t height):
  width_(width),
  height_(height)
{
  logger_.info("{}: allocating VRAM (size = {})", tag_, vram_size());
  vram = (uint8_t*)heap_caps_malloc(vram_size(), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  if (vram == nullptr) {
    logger_.error("{}: could not allocate VRAM!", tag_);
  }
}

Display::~Display() {
  // Clean up display driver
  screen_driver_.deinit();
  // Free interface driver
  if (screen_interface_driver_) {
    scr_interface_delete(screen_interface_driver_);
  }
  // clean up the spi bus
  spi_bus_delete(&spi_bus_);
  // free the pixel memory
  heap_caps_free(vram);
}

void Display::init() {
  // create the screen interface driver
#if CONFIG_DISPLAY_USE_SPI
  spi_config_t spi_cfg = {.miso_io_num=(gpio_num_t)CONFIG_DISPLAY_SPI_CIPO_PIN,
                          .mosi_io_num=(gpio_num_t)CONFIG_DISPLAY_SPI_COPI_PIN,
                          .sclk_io_num=(gpio_num_t)CONFIG_DISPLAY_SPI_CLK_PIN,
                          .max_transfer_sz = (uint16_t)(width_ * height_)
  };
  spi_bus_ = spi_bus_create((spi_host_device_t)(CONFIG_DISPLAY_SPI_BUS-1), &spi_cfg);
  scr_interface_spi_config_t config = {.spi_bus=spi_bus_,
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
               width_, height_);
  screen_config_ = scr_controller_config_t {.interface_drv=screen_interface_driver_,
                                            .pin_num_rst=CONFIG_DISPLAY_RST_PIN,
                                            .pin_num_bckl=CONFIG_DISPLAY_BACKLIGHT_PIN,
                                            .rst_active_level=0,
                                            .bckl_active_level=0,
                                            .width=(uint16_t)width_,
                                            .height=(uint16_t)height_,
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

void Display::update() {
  // acquire the interface
  LCD_IFACE_ACQUIRE();
  esp_err_t status;
  // say where we want to draw (whole window)
  status = screen_driver_.set_window(0, 0, width_ - 1, height_ - 1);
  if (status != ESP_OK) {
    logger_.error("{}: error setting window: {}", tag_, strerror(errno));
  }
  // now draw each pixel using the write_ram command since we've
  // already set the window
  for (size_t pixel=0; pixel < (width_ * height_); pixel++) {
    uint16_t color = myPalette[vram[pixel]];
    status = screen_driver_.write_ram_data(color);
  }
  if (status != ESP_OK) {
    logger_.error("{}: error writing ram data: {}", tag_, strerror(errno));
  }
  // release the interface
  LCD_IFACE_RELEASE();
}

// TEXT FUNCTIONS:
void Display::draw_5x8_char(char* _char_matrix,int x_start,int y_start,unsigned char clr)
{
  for (int col=0;col<=4;col++) {
    for (int row=0;row<=7;row++) {
      if ((row+y_start)>=0 && (row+y_start)< height_ && (col+x_start)>=0 && (col+x_start)< width_) {
        if (((_char_matrix[row]>>(7-col))&0x01))
          vram[(row+y_start) + (col+x_start) * height_] = clr;
        else
          vram[(row+y_start) + (col+x_start) * height_] = 0x00;
      }
    }
  }
}

void Display::draw_5x8_string(char* str,unsigned char len,int x_start,int y_start,unsigned char clr)
{
  for (int i=0;i<len;i++) {
    draw_5x8_char((char *)char5x8_matrix[(int)str[i]],x_start+i*6,y_start,clr);
  }
}

void Display::draw_8x12_char(char* _char_matrix,int x_start,int y_start,unsigned char clr)
{
  for (int row=0;row<12;row++) {
    for (int col=0;col<8;col++) {
      if ((row+y_start)>=0 && (row+y_start)< height_ && (col+x_start)>=0 && (col+x_start)< width_) {
        if (((_char_matrix[row]>>(7-col))&0x01))
          vram[(row+y_start) + (col+x_start) * height_] = clr;
        else
          vram[(row+y_start) + (col+x_start) * height_] = 0x00;
      }
    }
  }
}

void Display::draw_8x12_string(char* str,unsigned char len,int x_start,int y_start,unsigned char clr)
{
  for (int i=0;i<len;i++) {
    draw_8x12_char((char *)char8x12_matrix[(int)str[i]],x_start+i*9,y_start,clr);
  }
}

// DRAWING FUNCTIONS:

void Display::draw_rectangle(
                    const point_s&  pos,
                    const uint16_t width,
                    const uint16_t height,
                    const uint8_t  outline,
                    const uint8_t  fill) {
  int xLeft = pos.x - width/2,
		  xRight = pos.x + width/2,
		  yTop = pos.y - height/2,
		  yBottom = pos.y + height/2;
  for (int row=yTop;row<=yBottom;row++) {
    for (int col=xLeft;col<=xRight;col++) {
      if (row>=0 && col>=0 && row<height_ && col<width_) {
        if ( ((col-xLeft)<2) ||
             ((xRight-col)<2) ||
             ((row-yTop)<2) ||
             ((yBottom-row)<2))
          vram[row + col * height_] = outline;
        else vram[row + col * height_] = fill;
      }
    }
  }
}

void Display::plot_4_points(int cx, int cy, int x, int y, unsigned char clroutline,unsigned char clrfill)
{
  for (int row = cy-y;row<=(cy+y);row++) {
    for (int col=cx-x;col<=(cx+x);col++) {
      if (row>=0 && row< height_ && col>=0 && col< width_) vram[row + col * height_] = clrfill;
    }
  }
  if ((cy+y)>=0 && (cy+y)< height_ && (cx+x)>=0 && (cx+x)< width_)
    vram[(cy+y) + (cx+x) * height_] = clroutline;

  if (x != 0) {
    if ((cy+y)>=0 && (cy+y)< height_ && (cx-x)>=0 && (cx-x)< width_)
      vram[(cy+y) + (cx-x) * height_] = clroutline;
  }
  if (y != 0) {
    if ((cy-y)>=0 && (cy-y)< height_ && (cx+x)>=0 && (cx+x)< width_)
      vram[(cy-y) + (cx+x) * height_] = clroutline;
  }
  if (x != 0 && y != 0) {
    if ((cy-y)>=0 && (cy-y)< height_ && (cx-x)>=0 && (cx-x)< width_)
      vram[(cy-y) + (cx-x) * height_] = clroutline;
  }
}

void Display::plot_8_points(int cx, int cy, int x, int y, unsigned char clroutline,unsigned char clrfill)
{
  plot_4_points(cx, cy, x, y,clroutline,clrfill);
  plot_4_points(cx, cy, y, x,clroutline,clrfill);
}

void Display::circle(int cx, int cy, int radius, unsigned char clroutline, unsigned char clrfill)
{
  int error = -radius;
  int x = radius;
  int y = 0;

  while (x > y)
    {
      plot_8_points(cx, cy, x, y,clroutline,clrfill);

      error += y;
      ++y;
      error += y;

      if (error >= 0)
        {
          --x;
          error -= x;
          error -= x;
        }
    }
  plot_4_points(cx, cy, x, y,clroutline,clrfill);
}

void Display::draw_circle(
                 const point_s  &pos,
                 const uint16_t radius,
                 const uint8_t  outline,
                 const uint8_t  fill) {
  int cx = pos.x,
		  cy = pos.y;
  circle(cx, cy, radius, outline, fill);
}

void Display::draw_line(
               const point_s &start,
               const point_s &end,
               const uint8_t color) {
  int _dummy;
  int xLeft = start.x,
		  xRight = end.x,
		  yTop = start.y,
		  yBottom = end.y;
  int steep = (abs(yBottom - yTop) > abs(xRight - xLeft));
  if (steep) {
    _dummy = xLeft;
    xLeft = yTop;
    yTop = _dummy;
    _dummy = xRight;
    xRight = yBottom;
    yBottom = _dummy;
  }
  if (xLeft>xRight) {
    _dummy = xLeft;
    xLeft = xRight;
    xRight = _dummy;
    _dummy = yTop;
    yTop = yBottom;
    yBottom = _dummy;
  }
  int dx = xRight - xLeft;
  int dy = abs(yBottom - yTop);
  int error = dx>>1;		// divide by 2
  int ystep;
  int row = yTop;
  if (yTop<yBottom) ystep = 1;
  else ystep = -1;

  for (int col = xLeft;col <= xRight;col++) {
    if (row>=0 && col>=0 && row< height_ && col< width_) {
      if (steep) vram[col + row * height_] = color;
      else       vram[row + col * height_] = color;
    }
    error = error - dy;
    if (error<0) {
      row = row + ystep;
      error = error + dx;
    }
  }
}


// LOW LEVEL FUNCTIONS:
void Display::clear() {
  logger_.info("{}, clearing VRAM (size = {})", tag_, vram_size());
  memset(vram, 0, vram_size());
}

void Display::clear(
                const uint16_t x,
                const uint16_t y,
                const uint16_t width,
                const uint16_t height) {
  int xs = std::max((uint16_t)0, x),
    xe = std::min((uint16_t)width_, (uint16_t)(xs + width)),
    ys = std::max((uint16_t)0, y),
    ye = std::min((uint16_t)height_, (uint16_t)(ys + height)),
    len = ye - ys;
  for (int i=xs; i<xe; i++) {
    memset( &vram[ ys + i * height_ ], 0, len );
  }
}
