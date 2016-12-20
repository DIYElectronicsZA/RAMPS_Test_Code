#include "gfx_lcd_ctrlr.h"


int lcd_contrast;

uint8_t draw_state = 0;

U8GLIB_ST7920_128X64_1X u8g(23, 17, 16); // SPI Com: SCK = en = 23, MOSI = rw = 17, CS = di = 16

void lcd_init()
{
	pinMode(LCD_PIN_BL, OUTPUT);	// Enable LCD backlight
	digitalWrite(LCD_PIN_BL, HIGH);

  u8g.setContrast(lcd_contrast);

	u8g.firstPage();
	do {
		u8g.setFont(u8g_font_6x10_marlin);
		u8g.setColorIndex(1);
		u8g.drawBox (0, 0, u8g.getWidth(), u8g.getHeight());
		u8g.setColorIndex(1);
	   } while( u8g.nextPage() );


	u8g.firstPage();
	do {
			// RepRap init bmp
			u8g.drawBitmapP(0,0,START_BMPBYTEWIDTH,START_BMPHEIGHT,start_bmp);
			// Welcome message
			u8g.setFont(u8g_font_6x10_marlin);
			u8g.drawStr(62,10,"RAMPS TEST");
			u8g.setFont(u8g_font_5x8);
			u8g.drawStr(62,19,"V1");
			u8g.setFont(u8g_font_6x10_marlin);
			u8g.drawStr(62,28,"DIY3D.co.za");
			u8g.drawStr(62,41,"Prusa i3");
			u8g.setFont(u8g_font_5x8);
			u8g.drawStr(62,48,"rambo.co.za");
			//u8g.setFont(u8g_font_5x8);
			//u8g.drawStr(62,55,"by STB, MM");
			//u8g.drawStr(62,61,"uses u");
			//u8g.drawStr90(92,57,"8");
			//u8g.drawStr(100,61,"glib");
	   } while( u8g.nextPage() );
}

void lcd_run_test(){
  draw_state = 0;
  while(draw_state < 5*8){
    u8g.firstPage();
    do
    {
      draw();
    } while ( u8g.nextPage() );

    draw_state++;
  }
}

void u8g_prepare(void) {
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
}

void u8g_box_frame(uint8_t a) {
  u8g.drawStr(0, 0, "drawBox");
  u8g.drawBox(5,10,20,10);
  u8g.drawBox(10+a,15,30,7);
  u8g.drawStr(0, 30, "drawFrame");
  u8g.drawFrame(5,10+30,20,10);
  u8g.drawFrame(10+a,15+30,30,7);
}

void u8g_string(uint8_t a) {
  u8g.drawStr( 30+a,31, " 0");
  u8g.drawStr90( 30,31+a, " 90");
  u8g.drawStr180( 30-a,31, " 180");
  u8g.drawStr270( 30,31-a, " 270");
}

void u8g_line(uint8_t a) {
  u8g.drawStr( 0, 0, "drawLine");
  u8g.drawLine( 7+a, 10, 40, 55);
  u8g.drawLine( 7+a*2, 10, 60, 55);
  u8g.drawLine( 7+a*3, 10, 80, 55);
  u8g.drawLine( 7+a*4, 10, 100, 55);
}

void u8g_ascii_1(void) {
  char s[2] = " ";
  uint8_t x, y;
  u8g.drawStr( 0, 0, "ASCII page 1");
  for( y = 0; y < 6; y++ ) {
    for( x = 0; x < 16; x++ ) {
      s[0] = y*16 + x + 32;
      u8g.drawStr( x*7, y*10+10, s);
    }
  }
}

void u8g_ascii_2(void) {
  char s[2] = " ";
  uint8_t x, y;
  u8g.drawStr( 0, 0, "ASCII page 2");
  for( y = 0; y < 6; y++ ) {
    for( x = 0; x < 16; x++ ) {
      s[0] = y*16 + x + 160;
      u8g.drawStr( x*7, y*10+10, s);
    }
  }
}




void draw(void) {
	  u8g_prepare();
	  switch(draw_state >> 3) {
	    case 0: u8g_box_frame(draw_state&7); break;
	    case 1: u8g_string(draw_state&7); break;
	    case 2: u8g_line(draw_state&7); break;
	    case 3: u8g_ascii_1(); break;
	    case 4: u8g_ascii_2(); break;
	  }
}
