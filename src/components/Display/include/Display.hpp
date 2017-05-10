#ifndef DISPLAY_INCLUDE_GUARD_
#define DISPLAY_INCLUDE_GUARD_
#include <stdint.h>

//*****************************************************************************
//
// Make sure all of the definitions in this header have a C binding.
//
//*****************************************************************************

#define CONFIG_WROVER_KIT_V2        1
#define CONFIG_LCD_USE_FAST_PINS    0

#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 320

typedef struct s_point_s {
  uint16_t x;
  uint16_t y;
} point_s;

extern uint8_t  vram[];
extern uint16_t myPalette[];

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
#endif //DISPLAY_INCLUDE_GUARD_