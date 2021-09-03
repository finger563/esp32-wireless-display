#include "display.hpp"

#include <algorithm>
#include <string.h>

#include "fonts.hpp"

#define U16x2toU32(m,l) ((((uint32_t)(l>>8|(l&0xFF)<<8))<<16)|(m>>8|(m&0xFF)<<8))

uint8_t  *vram = new uint8_t[CONFIG_DISPLAY_WIDTH * CONFIG_DISPLAY_HEIGHT];
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

// TEXT FUNCTIONS:
void Draw_5x8_char(char* _char_matrix,int x_start,int y_start,unsigned char clr)
{
  int row, col;

  for (col=0;col<=4;col++) {
    for (row=0;row<=7;row++) {
      if ((row+y_start)>=0 && (row+y_start)< CONFIG_DISPLAY_HEIGHT && (col+x_start)>=0 && (col+x_start)< CONFIG_DISPLAY_WIDTH) {
        if (((_char_matrix[row]>>(7-col))&0x01))
          vram[(row+y_start) + (col+x_start) * CONFIG_DISPLAY_HEIGHT] = clr;
        else
          vram[(row+y_start) + (col+x_start) * CONFIG_DISPLAY_HEIGHT] = 0x00;
      }
    }
  }
}

void Draw_5x8_string(char* str,unsigned char len,int x_start,int y_start,unsigned char clr)
{
  int i = 0;
  for (i=0;i<len;i++) Draw_5x8_char((char *)char5x8_matrix[(int)str[i]],x_start+i*6,y_start,clr);
}

void Draw_8x12_char(char* _char_matrix,int x_start,int y_start,unsigned char clr)
{
  int row;
  int col;
  for (row=0;row<12;row++) {
    for (col=0;col<8;col++) {
      if ((row+y_start)>=0 && (row+y_start)< CONFIG_DISPLAY_HEIGHT && (col+x_start)>=0 && (col+x_start)< CONFIG_DISPLAY_WIDTH) {
        if (((_char_matrix[row]>>(7-col))&0x01))
          vram[(row+y_start) + (col+x_start) * CONFIG_DISPLAY_HEIGHT] = clr;
        else
          vram[(row+y_start) + (col+x_start) * CONFIG_DISPLAY_HEIGHT] = 0x00;
      }
    }
  }
}

void Draw_8x12_string(char* str,unsigned char len,int x_start,int y_start,unsigned char clr)
{
  int i = 0;
  for (i=0;i<len;i++) {
    Draw_8x12_char((char *)char8x12_matrix[(int)str[i]],x_start+i*9,y_start,clr);
  }
}

// DRAWING FUNCTIONS:

void draw_rectangle(
                    const point_s  pos,
                    const uint16_t width,
                    const uint16_t height,
                    const uint8_t  outline,
                    const uint8_t  fill) {
  int row;
  int col;
  int xLeft = pos.x - width/2,
		  xRight = pos.x + width/2,
		  yTop = pos.y - height/2,
		  yBottom = pos.y + height/2;
  for (row=yTop;row<=yBottom;row++) {
    for (col=xLeft;col<=xRight;col++) {
      if (row>=0 && col>=0 && row<CONFIG_DISPLAY_HEIGHT && col<CONFIG_DISPLAY_WIDTH) {
        if ( ((col-xLeft)<2) ||
             ((xRight-col)<2) ||
             ((row-yTop)<2) ||
             ((yBottom-row)<2))
          vram[row + col * CONFIG_DISPLAY_HEIGHT] = outline;
        else vram[row + col * CONFIG_DISPLAY_HEIGHT] = fill;
      }
    }
  }
}

void plot4points(int cx, int cy, int x, int y, unsigned char clroutline,unsigned char clrfill)
{
  int row,col;
  for (row = cy-y;row<=(cy+y);row++) {
    for (col=cx-x;col<=(cx+x);col++) {
      if (row>=0 && row< CONFIG_DISPLAY_HEIGHT && col>=0 && col< CONFIG_DISPLAY_WIDTH) vram[row + col * CONFIG_DISPLAY_HEIGHT] = clrfill;
    }
  }
  if ((cy+y)>=0 && (cy+y)< CONFIG_DISPLAY_HEIGHT && (cx+x)>=0 && (cx+x)< CONFIG_DISPLAY_WIDTH)
    vram[(cy+y) + (cx+x) * CONFIG_DISPLAY_HEIGHT] = clroutline;

  if (x != 0) {
    if ((cy+y)>=0 && (cy+y)< CONFIG_DISPLAY_HEIGHT && (cx-x)>=0 && (cx-x)< CONFIG_DISPLAY_WIDTH)
      vram[(cy+y) + (cx-x) * CONFIG_DISPLAY_HEIGHT] = clroutline;
  }
  if (y != 0) {
    if ((cy-y)>=0 && (cy-y)< CONFIG_DISPLAY_HEIGHT && (cx+x)>=0 && (cx+x)< CONFIG_DISPLAY_WIDTH)
      vram[(cy-y) + (cx+x) * CONFIG_DISPLAY_HEIGHT] = clroutline;
  }
  if (x != 0 && y != 0) {
    if ((cy-y)>=0 && (cy-y)< CONFIG_DISPLAY_HEIGHT && (cx-x)>=0 && (cx-x)< CONFIG_DISPLAY_WIDTH)
      vram[(cy-y) + (cx-x) * CONFIG_DISPLAY_HEIGHT] = clroutline;
  }
}

void plot8points(int cx, int cy, int x, int y, unsigned char clroutline,unsigned char clrfill)
{
  plot4points(cx, cy, x, y,clroutline,clrfill);
  plot4points(cx, cy, y, x,clroutline,clrfill);
}

void circle(int cx, int cy, int radius, unsigned char clroutline, unsigned char clrfill)
{
  int error = -radius;
  int x = radius;
  int y = 0;

  while (x > y)
    {
      plot8points(cx, cy, x, y,clroutline,clrfill);

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
  plot4points(cx, cy, x, y,clroutline,clrfill);
}

void draw_circle(
                 const point_s  pos,
                 const uint16_t radius,
                 const uint8_t  outline,
                 const uint8_t  fill) {
  int cx = pos.x,
		  cy = pos.y;
  circle(cx, cy, radius, outline, fill);
}

void draw_line(
               const point_s start,
               const point_s end,
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

  int col;

  for (col = xLeft;col <= xRight;col++) {
    if (row>=0 && col>=0 && row< CONFIG_DISPLAY_HEIGHT && col< CONFIG_DISPLAY_WIDTH) {
      if (steep) vram[col + row * CONFIG_DISPLAY_HEIGHT] = color;
      else       vram[row + col * CONFIG_DISPLAY_HEIGHT] = color;
    }
    error = error - dy;
    if (error<0) {
      row = row + ystep;
      error = error + dx;
    }
  }
}


// LOW LEVEL FUNCTIONS:
void clear_vram() {
  memset(vram, 0, CONFIG_DISPLAY_WIDTH * CONFIG_DISPLAY_HEIGHT);
}

void clear_vram(
                const uint16_t x,
                const uint16_t y,
                const uint16_t width,
                const uint16_t height) {
  int xs = std::max((uint16_t)0, x),
    xe = std::min((uint16_t)CONFIG_DISPLAY_WIDTH, (uint16_t)(xs + width)),
    ys = std::max((uint16_t)0, y),
    ye = std::min((uint16_t)CONFIG_DISPLAY_HEIGHT, (uint16_t)(ys + height)),
    len = ye - ys;
  for (int i=xs; i<xe; i++) {
    memset( &vram[ ys + i * CONFIG_DISPLAY_HEIGHT ], 0, len );
  }
}

void display_vram() {
  // ili9341_write_frame(0, 0, CONFIG_DISPLAY_WIDTH, CONFIG_DISPLAY_HEIGHT, (const uint8_t *)vram);
}
