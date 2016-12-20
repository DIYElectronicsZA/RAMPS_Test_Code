#ifndef GFXLCD_H
#define GFXLCD_H

#include "Arduino.h"
#include <U8glib.h>
#include "DOGMbitmaps.h"
#include "dogm_font_data_marlin.h"


#define BEEPER 29
// Pins for DOGM SPI LCD Support
#define DOGLCD_A0  30
#define DOGLCD_CS  17
#define LCD_PIN_BL 28  // backlight LED on PA3
// GLCD features
#define LCD_CONTRAST 1
// Uncomment screen orientation
#define LCD_SCREEN_ROT_0
  // #define LCD_SCREEN_ROT_90
  // #define LCD_SCREEN_ROT_180
  // #define LCD_SCREEN_ROT_270
//The encoder and click button
#define BTN_EN1 11
#define BTN_EN2 10
#define BTN_ENC 16  //the click switch
//not connected to a pin
#define SDCARDDETECT -1


void lcd_init();
void lcd_run_test();
void draw();


#endif //GFXLCD_H
