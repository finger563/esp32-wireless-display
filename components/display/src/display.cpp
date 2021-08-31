#include "display.hpp"

extern "C" {
#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "rom/ets_sys.h"
#include "rom/gpio.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"
#include "soc/gpio_struct.h"
#include "soc/io_mux_reg.h"
#include "soc/spi_reg.h"
}

#include "fonts.hpp"

#define U16x2toU32(m,l) ((((uint32_t)(l>>8|(l&0xFF)<<8))<<16)|(m>>8|(m&0xFF)<<8))

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define LCD_SEL_CMD()   GPIO.out_w1tc = (1 << CONFIG_DISPLAY_DC_PIN) // Low to send command
#define LCD_SEL_DATA()  GPIO.out_w1ts = (1 << CONFIG_DISPLAY_DC_PIN) // High to send data
#define LCD_RST_SET()   GPIO.out_w1ts = (1 << CONFIG_DISPLAY_RST_PIN)
#define LCD_RST_CLR()   GPIO.out_w1tc = (1 << CONFIG_DISPLAY_RST_PIN)

#define LCD_BKG_OFF()    GPIO.out_w1ts = (1 << CONFIG_DISPLAY_BACKLIGHT_PIN) // Backlight OFF
#define LCD_BKG_ON()   GPIO.out_w1tc = (1 << CONFIG_DISPLAY_BACKLIGHT_PIN) // Backlight ON

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
  //blit_vram(xLeft, yTop, xRight - xLeft, yBottom - yTop);
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
// forward declare main low level func for displaying to screen:
void ili9341_write_frame(
                         const uint16_t x,
                         const uint16_t y,
                         const uint16_t width,
                         const uint16_t height,
                         const uint8_t *data);

void clear_vram() {
  memset(vram, 0, CONFIG_DISPLAY_WIDTH * CONFIG_DISPLAY_HEIGHT);
}

void clear_vram(
                const uint16_t x,
                const uint16_t y,
                const uint16_t width,
                const uint16_t height) {
  int xs = MAX(0, x),
    xe = MIN(CONFIG_DISPLAY_WIDTH, xs + width),
    ys = MAX(0, y),
    ye = MIN(CONFIG_DISPLAY_HEIGHT, ys + height),
    len = ye - ys;
  for (int i=xs; i<xe; i++) {
    memset( &vram[ ys + i * CONFIG_DISPLAY_HEIGHT ], 0, len );
  }
}

void display_vram() {
  ili9341_write_frame(0, 0, CONFIG_DISPLAY_WIDTH, CONFIG_DISPLAY_HEIGHT, (const uint8_t *)vram);
}

void blit_vram(const uint16_t xs, const uint16_t ys, const uint16_t width, const uint16_t height) {
  int x, y;
  int i;
  uint16_t x1, y1;
  uint32_t xv, yv, dc;
  uint32_t temp[16];
  dc = (1 << CONFIG_DISPLAY_DC_PIN);

  for (y=0; y<height; y++) {
    //start line
    x1 = xs+(width-1);
    y1 = ys+y+(height-1);
    xv = U16x2toU32(xs,x1);
    yv = U16x2toU32((ys+y),y1);

    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1tc = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 7, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), 0x2A);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1ts = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 31, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), xv);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1tc = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 7, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), 0x2B);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1ts = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 31, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), yv);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1tc = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 7, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), 0x2C);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);

    x = 0;
    GPIO.out_w1ts = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 511, SPI_USR_MOSI_DBITLEN_S);
    while (x<width) {
      for (i=0; i<16; i++) {
        x1 = myPalette[(unsigned char)(vram[y + x * CONFIG_DISPLAY_HEIGHT])]; x++;
        y1 = myPalette[(unsigned char)(vram[y + x * CONFIG_DISPLAY_HEIGHT])]; x++;
        temp[i] = U16x2toU32(x1,y1);
      }
      while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
      for (i=0; i<16; i++) {
        WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS) + (i << 2)), temp[i]);
      }
      SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    }
  }
  while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
}

// LOCAL ONLY FUNCTIONS

static void spi_write_byte(const uint8_t data){
  SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 0x7, SPI_USR_MOSI_DBITLEN_S);
  WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), data);
  SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
  while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
}

static void LCD_WriteCommand(const uint8_t cmd)
{
  LCD_SEL_CMD();
  spi_write_byte(cmd);
}

static void LCD_WriteData(const uint8_t data)
{
  LCD_SEL_DATA();
  spi_write_byte(data);
}

static void  ILI9341_INITIAL ()
{
  LCD_BKG_ON();
  //------------------------------------Reset Sequence-----------------------------------------//

  LCD_RST_SET();
  ets_delay_us(100000);

  LCD_RST_CLR();
  ets_delay_us(200000);

  LCD_RST_SET();
  ets_delay_us(200000);

  //************* Start Initial Sequence **********//
  LCD_WriteCommand(0xCF);
  LCD_WriteData(0x00);
  LCD_WriteData(0x83);
  LCD_WriteData(0X30);

  LCD_WriteCommand(0xED);
  LCD_WriteData(0x64);
  LCD_WriteData(0x03);
  LCD_WriteData(0X12);
  LCD_WriteData(0X81);

  LCD_WriteCommand(0xE8);
  LCD_WriteData(0x85);
  LCD_WriteData(0x01); //i
  LCD_WriteData(0x79); //i

  LCD_WriteCommand(0xCB);
  LCD_WriteData(0x39);
  LCD_WriteData(0x2C);
  LCD_WriteData(0x00);
  LCD_WriteData(0x34);
  LCD_WriteData(0x02);

  LCD_WriteCommand(0xF7);
  LCD_WriteData(0x20);

  LCD_WriteCommand(0xEA);
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);

  LCD_WriteCommand(0xC0);    //Power control
  LCD_WriteData(0x26); //i  //VRH[5:0]

  LCD_WriteCommand(0xC1);    //Power control
  LCD_WriteData(0x11);   //i //SAP[2:0];BT[3:0]

  LCD_WriteCommand(0xC5);    //VCM control
  LCD_WriteData(0x35); //i
  LCD_WriteData(0x3E); //i

  LCD_WriteCommand(0xC7);    //VCM control2
  LCD_WriteData(0xBE); //i   //»òÕß B1h

  LCD_WriteCommand(0x36);    // Memory Access Control
  LCD_WriteData(0b01101000); // MY MX MV ML BGR MH 0 0, default 00
  //LCD_WriteData(0x28); //i //was 0x48

  LCD_WriteCommand(0x3A);
  LCD_WriteData(0x55);

  LCD_WriteCommand(0xB1);
  LCD_WriteData(0x00);
  LCD_WriteData(0x1B); //18

  LCD_WriteCommand(0xF2);    // 3Gamma Function Disable
  LCD_WriteData(0x08);

  LCD_WriteCommand(0x26);    //Gamma curve selected
  LCD_WriteData(0x01);

  LCD_WriteCommand(0xE0);    //Set Gamma
  LCD_WriteData(0x1F);
  LCD_WriteData(0x1A);
  LCD_WriteData(0x18);
  LCD_WriteData(0x0A);
  LCD_WriteData(0x0F);
  LCD_WriteData(0x06);
  LCD_WriteData(0x45);
  LCD_WriteData(0X87);
  LCD_WriteData(0x32);
  LCD_WriteData(0x0A);
  LCD_WriteData(0x07);
  LCD_WriteData(0x02);
  LCD_WriteData(0x07);
  LCD_WriteData(0x05);
  LCD_WriteData(0x00);

  LCD_WriteCommand(0XE1);    //Set Gamma
  LCD_WriteData(0x00);
  LCD_WriteData(0x25);
  LCD_WriteData(0x27);
  LCD_WriteData(0x05);
  LCD_WriteData(0x10);
  LCD_WriteData(0x09);
  LCD_WriteData(0x3A);
  LCD_WriteData(0x78);
  LCD_WriteData(0x4D);
  LCD_WriteData(0x05);
  LCD_WriteData(0x18);
  LCD_WriteData(0x0D);
  LCD_WriteData(0x38);
  LCD_WriteData(0x3A);
  LCD_WriteData(0x1F);

  LCD_WriteCommand(0x2A);
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);
  LCD_WriteData(0xEF);

  LCD_WriteCommand(0x2B);
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);
  LCD_WriteData(0x01);
  LCD_WriteData(0x3f);
  LCD_WriteCommand(0x2C);

  LCD_WriteCommand(0xB7);
  LCD_WriteData(0x07);

  LCD_WriteCommand(0xB6);    // Display Function Control
  LCD_WriteData(0x0A); //8 82 27
  LCD_WriteData(0x82);
  LCD_WriteData(0x27);
  LCD_WriteData(0x00);

  //LCD_WriteCommand(0xF6); //not there
  //LCD_WriteData(0x01);
  //LCD_WriteData(0x30);

  LCD_WriteCommand(0x11);    //Exit Sleep
  ets_delay_us(100000);
  LCD_WriteCommand(0x29);    //Display on
  ets_delay_us(100000);
}
//.............LCD API END----------

static void ili_gpio_init()
{
#if CONFIG_LCD_USE_FAST_PINS
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO21_U,2);   //DC PIN
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO22_U,2);   //RESET PIN
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO25_U,2);   //BKL PIN
  WRITE_PERI_REG(GPIO_ENABLE_W1TS_REG, BIT21|BIT22|BIT25);
#else
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO21_U,2);   //DC PIN
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO18_U,2);   //RESET PIN
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,2);    //BKL PIN
  WRITE_PERI_REG(GPIO_ENABLE_W1TS_REG, BIT21|BIT18|BIT5);
#endif
}

static void spi_master_init()
{
  ets_printf("lcd spi pin mux init ...\r\n");
#if CONFIG_LCD_USE_FAST_PINS
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO19_U,1);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO23_U,1);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO18_U,1);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,1);
#else
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO19_U,2);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO23_U,2);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO22_U,2);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO25_U,2);
  WRITE_PERI_REG(GPIO_ENABLE_W1TS_REG, BIT19|BIT23|BIT22);

  ets_printf("lcd spi signal init\r\n");
  gpio_matrix_in(CONFIG_DISPLAY_SPI_CIPO_PIN, VSPIQ_IN_IDX,0);
  gpio_matrix_out(CONFIG_DISPLAY_SPI_COPI_PIN, VSPID_OUT_IDX,0,0);
  gpio_matrix_out(CONFIG_DISPLAY_SPI_CLK_PIN, VSPICLK_OUT_IDX,0,0);
  gpio_matrix_out(CONFIG_DISPLAY_SPI_CS_PIN, VSPICS0_OUT_IDX,0,0);
#endif
  ets_printf("Hspi config\r\n");

  CLEAR_PERI_REG_MASK(SPI_SLAVE_REG(CONFIG_DISPLAY_SPI_BUS), SPI_TRANS_DONE << 5);
  SET_PERI_REG_MASK(SPI_USER_REG(CONFIG_DISPLAY_SPI_BUS), SPI_CS_SETUP);
  CLEAR_PERI_REG_MASK(SPI_PIN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_CK_IDLE_EDGE);
  CLEAR_PERI_REG_MASK(SPI_USER_REG(CONFIG_DISPLAY_SPI_BUS),  SPI_CK_OUT_EDGE);
  CLEAR_PERI_REG_MASK(SPI_CTRL_REG(CONFIG_DISPLAY_SPI_BUS), SPI_WR_BIT_ORDER);
  CLEAR_PERI_REG_MASK(SPI_CTRL_REG(CONFIG_DISPLAY_SPI_BUS), SPI_RD_BIT_ORDER);
  CLEAR_PERI_REG_MASK(SPI_USER_REG(CONFIG_DISPLAY_SPI_BUS), SPI_DOUTDIN);
  WRITE_PERI_REG(SPI_USER1_REG(CONFIG_DISPLAY_SPI_BUS), 0);
  SET_PERI_REG_BITS(SPI_CTRL2_REG(CONFIG_DISPLAY_SPI_BUS), SPI_MISO_DELAY_MODE, 0, SPI_MISO_DELAY_MODE_S);
  CLEAR_PERI_REG_MASK(SPI_SLAVE_REG(CONFIG_DISPLAY_SPI_BUS), SPI_SLAVE_MODE);

  WRITE_PERI_REG(SPI_CLOCK_REG(CONFIG_DISPLAY_SPI_BUS), (1 << SPI_CLKCNT_N_S) | (1 << SPI_CLKCNT_L_S));//40MHz
  //WRITE_PERI_REG(SPI_CLOCK_REG(CONFIG_DISPLAY_SPI_BUS), SPI_CLK_EQU_SYSCLK); // 80Mhz

  SET_PERI_REG_MASK(SPI_USER_REG(CONFIG_DISPLAY_SPI_BUS), SPI_CS_SETUP | SPI_CS_HOLD | SPI_USR_MOSI);
  SET_PERI_REG_MASK(SPI_CTRL2_REG(CONFIG_DISPLAY_SPI_BUS), ((0x4 & SPI_MISO_DELAY_NUM) << SPI_MISO_DELAY_NUM_S));
  CLEAR_PERI_REG_MASK(SPI_USER_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_COMMAND);
  SET_PERI_REG_BITS(SPI_USER2_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_COMMAND_BITLEN, 0, SPI_USR_COMMAND_BITLEN_S);
  CLEAR_PERI_REG_MASK(SPI_USER_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_ADDR);
  SET_PERI_REG_BITS(SPI_USER1_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_ADDR_BITLEN, 0, SPI_USR_ADDR_BITLEN_S);
  CLEAR_PERI_REG_MASK(SPI_USER_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MISO);
  SET_PERI_REG_MASK(SPI_USER_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI);
  char i;
  for (i = 0; i < 16; ++i) {
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS) + (i << 2)), 0);
  }
}

void ili9341_write_frame(const uint16_t xs, const uint16_t ys, const uint16_t width, const uint16_t height, const uint8_t * data){
  int x, y;
  int i;
  uint16_t x1, y1;
  uint32_t xv, yv, dc;
  uint32_t temp[16];
  dc = (1 << CONFIG_DISPLAY_DC_PIN);

  for (y=0; y<height; y++) {
    //start line
    x1 = xs+(width-1);
    y1 = ys+y+(height-1);
    xv = U16x2toU32(xs,x1);
    yv = U16x2toU32((ys+y),y1);

    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1tc = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 7, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), 0x2A);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1ts = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 31, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), xv);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1tc = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 7, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), 0x2B);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1ts = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 31, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), yv);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
    GPIO.out_w1tc = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 7, SPI_USR_MOSI_DBITLEN_S);
    WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS)), 0x2C);
    SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);

    x = 0;
    GPIO.out_w1ts = dc;
    SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR_MOSI_DBITLEN, 511, SPI_USR_MOSI_DBITLEN_S);
    while (x<width) {
      for (i=0; i<16; i++) {
        if(data == NULL){
          temp[i] = 0;
          x += 2;
          continue;
        }
        x1 = myPalette[(unsigned char)(data[y + x * height])]; x++;
        y1 = myPalette[(unsigned char)(data[y + x * height])]; x++;
        temp[i] = U16x2toU32(x1,y1);
      }
      while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
      for (i=0; i<16; i++) {
        WRITE_PERI_REG((SPI_W0_REG(CONFIG_DISPLAY_SPI_BUS) + (i << 2)), temp[i]);
      }
      SET_PERI_REG_MASK(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS), SPI_USR);
    }
  }
  while (READ_PERI_REG(SPI_CMD_REG(CONFIG_DISPLAY_SPI_BUS))&SPI_USR);
}

void ili9341_init()
{
  spi_master_init();
  ili_gpio_init();
  ILI9341_INITIAL ();
}
