
#include "lcd_5110_lib.h"
#include <math.h>


#define PI 3.14

int lcd_5110_pixels[84][6] ;//change 84 to 83 (and 6 to 5) and see what happens?

//copying fonttable
//static const unsigned char LCDBitmapFont[] = {
const unsigned char LCDBitmapFont[] = {
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
	0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
	0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
	0x18, 0x3C, 0x7E, 0x3C, 0x18,
	0x1C, 0x57, 0x7D, 0x57, 0x1C,
	0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
	0x00, 0x18, 0x3C, 0x18, 0x00,
	0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
	0x00, 0x18, 0x24, 0x18, 0x00,
	0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
	0x30, 0x48, 0x3A, 0x06, 0x0E,
	0x26, 0x29, 0x79, 0x29, 0x26,
	0x40, 0x7F, 0x05, 0x05, 0x07,
	0x40, 0x7F, 0x05, 0x25, 0x3F,
	0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
	0x7F, 0x3E, 0x1C, 0x1C, 0x08,
	0x08, 0x1C, 0x1C, 0x3E, 0x7F,
	0x14, 0x22, 0x7F, 0x22, 0x14,
	0x5F, 0x5F, 0x00, 0x5F, 0x5F,
	0x06, 0x09, 0x7F, 0x01, 0x7F,
	0x00, 0x66, 0x89, 0x95, 0x6A,
	0x60, 0x60, 0x60, 0x60, 0x60,
	0x94, 0xA2, 0xFF, 0xA2, 0x94,
	0x08, 0x04, 0x7E, 0x04, 0x08,
	0x10, 0x20, 0x7E, 0x20, 0x10,
	0x08, 0x08, 0x2A, 0x1C, 0x08,
	0x08, 0x1C, 0x2A, 0x08, 0x08,
	0x1E, 0x10, 0x10, 0x10, 0x10,
	0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
	0x30, 0x38, 0x3E, 0x38, 0x30,
	0x06, 0x0E, 0x3E, 0x0E, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x5F, 0x00, 0x00,
	0x00, 0x07, 0x00, 0x07, 0x00,
	0x14, 0x7F, 0x14, 0x7F, 0x14,
	0x24, 0x2A, 0x7F, 0x2A, 0x12,
	0x23, 0x13, 0x08, 0x64, 0x62,
	0x36, 0x49, 0x56, 0x20, 0x50,
	0x00, 0x08, 0x07, 0x03, 0x00,
	0x00, 0x1C, 0x22, 0x41, 0x00,
	0x00, 0x41, 0x22, 0x1C, 0x00,
	0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
	0x08, 0x08, 0x3E, 0x08, 0x08,
	0x00, 0x80, 0x70, 0x30, 0x00,
	0x08, 0x08, 0x08, 0x08, 0x08,
	0x00, 0x00, 0x60, 0x60, 0x00,
	0x20, 0x10, 0x08, 0x04, 0x02,
	0x3E, 0x51, 0x49, 0x45, 0x3E,
	0x00, 0x42, 0x7F, 0x40, 0x00,
	0x72, 0x49, 0x49, 0x49, 0x46,
	0x21, 0x41, 0x49, 0x4D, 0x33,
	0x18, 0x14, 0x12, 0x7F, 0x10,
	0x27, 0x45, 0x45, 0x45, 0x39,
	0x3C, 0x4A, 0x49, 0x49, 0x31,
	0x41, 0x21, 0x11, 0x09, 0x07,
	0x36, 0x49, 0x49, 0x49, 0x36,
	0x46, 0x49, 0x49, 0x29, 0x1E,
	0x00, 0x00, 0x14, 0x00, 0x00,
	0x00, 0x40, 0x34, 0x00, 0x00,
	0x00, 0x08, 0x14, 0x22, 0x41,
	0x14, 0x14, 0x14, 0x14, 0x14,
	0x00, 0x41, 0x22, 0x14, 0x08,
	0x02, 0x01, 0x59, 0x09, 0x06,
	0x3E, 0x41, 0x5D, 0x59, 0x4E,
	0x7C, 0x12, 0x11, 0x12, 0x7C,
	0x7F, 0x49, 0x49, 0x49, 0x36,
	0x3E, 0x41, 0x41, 0x41, 0x22,
	0x7F, 0x41, 0x41, 0x41, 0x3E,
	0x7F, 0x49, 0x49, 0x49, 0x41, //letter E
	0x7F, 0x09, 0x09, 0x09, 0x01,
	0x3E, 0x41, 0x41, 0x51, 0x73,
	0x7F, 0x08, 0x08, 0x08, 0x7F,
	0x00, 0x41, 0x7F, 0x41, 0x00,
	0x20, 0x40, 0x41, 0x3F, 0x01,
	0x7F, 0x08, 0x14, 0x22, 0x41,
	0x7F, 0x40, 0x40, 0x40, 0x40,
	0x7F, 0x02, 0x1C, 0x02, 0x7F,
	0x7F, 0x04, 0x08, 0x10, 0x7F,
	0x3E, 0x41, 0x41, 0x41, 0x3E,
	0x7F, 0x09, 0x09, 0x09, 0x06,
	0x3E, 0x41, 0x51, 0x21, 0x5E,
	0x7F, 0x09, 0x19, 0x29, 0x46,
	0x26, 0x49, 0x49, 0x49, 0x32,
	0x03, 0x01, 0x7F, 0x01, 0x03,
	0x3F, 0x40, 0x40, 0x40, 0x3F,
	0x1F, 0x20, 0x40, 0x20, 0x1F,
	0x3F, 0x40, 0x38, 0x40, 0x3F,
	0x63, 0x14, 0x08, 0x14, 0x63,
	0x03, 0x04, 0x78, 0x04, 0x03,
	0x61, 0x59, 0x49, 0x4D, 0x43,
	0x00, 0x7F, 0x41, 0x41, 0x41,
	0x02, 0x04, 0x08, 0x10, 0x20,
	0x00, 0x41, 0x41, 0x41, 0x7F,
	0x04, 0x02, 0x01, 0x02, 0x04,
	0x40, 0x40, 0x40, 0x40, 0x40,
	0x00, 0x03, 0x07, 0x08, 0x00,
	0x20, 0x54, 0x54, 0x78, 0x40,
	0x7F, 0x28, 0x44, 0x44, 0x38,
	0x38, 0x44, 0x44, 0x44, 0x28,
	0x38, 0x44, 0x44, 0x28, 0x7F,
	0x38, 0x54, 0x54, 0x54, 0x18,
	0x00, 0x08, 0x7E, 0x09, 0x02,
	0x18, 0xA4, 0xA4, 0x9C, 0x78,
	0x7F, 0x08, 0x04, 0x04, 0x78,
	0x00, 0x44, 0x7D, 0x40, 0x00,
	0x20, 0x40, 0x40, 0x3D, 0x00,
	0x7F, 0x10, 0x28, 0x44, 0x00,
	0x00, 0x41, 0x7F, 0x40, 0x00,
	0x7C, 0x04, 0x78, 0x04, 0x78,
	0x7C, 0x08, 0x04, 0x04, 0x78,
	0x38, 0x44, 0x44, 0x44, 0x38,
	0xFC, 0x18, 0x24, 0x24, 0x18,
	0x18, 0x24, 0x24, 0x18, 0xFC,
	0x7C, 0x08, 0x04, 0x04, 0x08,
	0x48, 0x54, 0x54, 0x54, 0x24,
	0x04, 0x04, 0x3F, 0x44, 0x24,
	0x3C, 0x40, 0x40, 0x20, 0x7C,
	0x1C, 0x20, 0x40, 0x20, 0x1C,
	0x3C, 0x40, 0x30, 0x40, 0x3C,
	0x44, 0x28, 0x10, 0x28, 0x44,
	0x4C, 0x90, 0x90, 0x90, 0x7C,
	0x44, 0x64, 0x54, 0x4C, 0x44,
	0x00, 0x08, 0x36, 0x41, 0x00,
	0x00, 0x00, 0x77, 0x00, 0x00,
	0x00, 0x41, 0x36, 0x08, 0x00,
	0x02, 0x01, 0x02, 0x04, 0x02,
	0x3C, 0x26, 0x23, 0x26, 0x3C,
	0x1E, 0xA1, 0xA1, 0x61, 0x12,
	0x3A, 0x40, 0x40, 0x20, 0x7A,
	0x38, 0x54, 0x54, 0x55, 0x59,
	0x21, 0x55, 0x55, 0x79, 0x41,
	0x22, 0x54, 0x54, 0x78, 0x42, // a-umlaut
	0x21, 0x55, 0x54, 0x78, 0x40,
	0x20, 0x54, 0x55, 0x79, 0x40,
	0x0C, 0x1E, 0x52, 0x72, 0x12,
	0x39, 0x55, 0x55, 0x55, 0x59,
	0x39, 0x54, 0x54, 0x54, 0x59,
	0x39, 0x55, 0x54, 0x54, 0x58,
	0x00, 0x00, 0x45, 0x7C, 0x41,
	0x00, 0x02, 0x45, 0x7D, 0x42,
	0x00, 0x01, 0x45, 0x7C, 0x40,
	0x7D, 0x12, 0x11, 0x12, 0x7D, // A-umlaut
	0xF0, 0x28, 0x25, 0x28, 0xF0,
	0x7C, 0x54, 0x55, 0x45, 0x00,
	0x20, 0x54, 0x54, 0x7C, 0x54,
	0x7C, 0x0A, 0x09, 0x7F, 0x49,
	0x32, 0x49, 0x49, 0x49, 0x32,
	0x3A, 0x44, 0x44, 0x44, 0x3A, // o-umlaut
	0x32, 0x4A, 0x48, 0x48, 0x30,
	0x3A, 0x41, 0x41, 0x21, 0x7A,
	0x3A, 0x42, 0x40, 0x20, 0x78,
	0x00, 0x9D, 0xA0, 0xA0, 0x7D,
	0x3D, 0x42, 0x42, 0x42, 0x3D, // O-umlaut
	0x3D, 0x40, 0x40, 0x40, 0x3D,
	0x3C, 0x24, 0xFF, 0x24, 0x24,
	0x48, 0x7E, 0x49, 0x43, 0x66,
	0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
	0xFF, 0x09, 0x29, 0xF6, 0x20,
	0xC0, 0x88, 0x7E, 0x09, 0x03,
	0x20, 0x54, 0x54, 0x79, 0x41,
	0x00, 0x00, 0x44, 0x7D, 0x41,
	0x30, 0x48, 0x48, 0x4A, 0x32,
	0x38, 0x40, 0x40, 0x22, 0x7A,
	0x00, 0x7A, 0x0A, 0x0A, 0x72,
	0x7D, 0x0D, 0x19, 0x31, 0x7D,
	0x26, 0x29, 0x29, 0x2F, 0x28,
	0x26, 0x29, 0x29, 0x29, 0x26,
	0x30, 0x48, 0x4D, 0x40, 0x20,
	0x38, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x38,
	0x2F, 0x10, 0xC8, 0xAC, 0xBA,
	0x2F, 0x10, 0x28, 0x34, 0xFA,
	0x00, 0x00, 0x7B, 0x00, 0x00,
	0x08, 0x14, 0x2A, 0x14, 0x22,
	0x22, 0x14, 0x2A, 0x14, 0x08,
	0x55, 0x00, 0x55, 0x00, 0x55, // #176 (25% block) missing in old code
	0xAA, 0x55, 0xAA, 0x55, 0xAA, // 50% block
	0xFF, 0x55, 0xFF, 0x55, 0xFF, // 75% block
	0x00, 0x00, 0x00, 0xFF, 0x00,
	0x10, 0x10, 0x10, 0xFF, 0x00,
	0x14, 0x14, 0x14, 0xFF, 0x00,
	0x10, 0x10, 0xFF, 0x00, 0xFF,
	0x10, 0x10, 0xF0, 0x10, 0xF0,
	0x14, 0x14, 0x14, 0xFC, 0x00,
	0x14, 0x14, 0xF7, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0xFF,
	0x14, 0x14, 0xF4, 0x04, 0xFC,
	0x14, 0x14, 0x17, 0x10, 0x1F,
	0x10, 0x10, 0x1F, 0x10, 0x1F,
	0x14, 0x14, 0x14, 0x1F, 0x00,
	0x10, 0x10, 0x10, 0xF0, 0x00,
	0x00, 0x00, 0x00, 0x1F, 0x10,
	0x10, 0x10, 0x10, 0x1F, 0x10,
	0x10, 0x10, 0x10, 0xF0, 0x10,
	0x00, 0x00, 0x00, 0xFF, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0xFF, 0x10,
	0x00, 0x00, 0x00, 0xFF, 0x14,
	0x00, 0x00, 0xFF, 0x00, 0xFF,
	0x00, 0x00, 0x1F, 0x10, 0x17,
	0x00, 0x00, 0xFC, 0x04, 0xF4,
	0x14, 0x14, 0x17, 0x10, 0x17,
	0x14, 0x14, 0xF4, 0x04, 0xF4,
	0x00, 0x00, 0xFF, 0x00, 0xF7,
	0x14, 0x14, 0x14, 0x14, 0x14,
	0x14, 0x14, 0xF7, 0x00, 0xF7,
	0x14, 0x14, 0x14, 0x17, 0x14,
	0x10, 0x10, 0x1F, 0x10, 0x1F,
	0x14, 0x14, 0x14, 0xF4, 0x14,
	0x10, 0x10, 0xF0, 0x10, 0xF0,
	0x00, 0x00, 0x1F, 0x10, 0x1F,
	0x00, 0x00, 0x00, 0x1F, 0x14,
	0x00, 0x00, 0x00, 0xFC, 0x14,
	0x00, 0x00, 0xF0, 0x10, 0xF0,
	0x10, 0x10, 0xFF, 0x10, 0xFF,
	0x14, 0x14, 0x14, 0xFF, 0x14,
	0x10, 0x10, 0x10, 0x1F, 0x00,
	0x00, 0x00, 0x00, 0xF0, 0x10,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0xFF,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0x38, 0x44, 0x44, 0x38, 0x44,
	0xFC, 0x4A, 0x4A, 0x4A, 0x34, // sharp-s or beta
	0x7E, 0x02, 0x02, 0x06, 0x06,
	0x02, 0x7E, 0x02, 0x7E, 0x02,
	0x63, 0x55, 0x49, 0x41, 0x63,
	0x38, 0x44, 0x44, 0x3C, 0x04,
	0x40, 0x7E, 0x20, 0x1E, 0x20,
	0x06, 0x02, 0x7E, 0x02, 0x02,
	0x99, 0xA5, 0xE7, 0xA5, 0x99,
	0x1C, 0x2A, 0x49, 0x2A, 0x1C,
	0x4C, 0x72, 0x01, 0x72, 0x4C,
	0x30, 0x4A, 0x4D, 0x4D, 0x30,
	0x30, 0x48, 0x78, 0x48, 0x30,
	0xBC, 0x62, 0x5A, 0x46, 0x3D,
	0x3E, 0x49, 0x49, 0x49, 0x00,
	0x7E, 0x01, 0x01, 0x01, 0x7E,
	0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
	0x44, 0x44, 0x5F, 0x44, 0x44,
	0x40, 0x51, 0x4A, 0x44, 0x40,
	0x40, 0x44, 0x4A, 0x51, 0x40,
	0x00, 0x00, 0xFF, 0x01, 0x03,
	0xE0, 0x80, 0xFF, 0x00, 0x00,
	0x08, 0x08, 0x6B, 0x6B, 0x08,
	0x36, 0x12, 0x36, 0x24, 0x36,
	0x06, 0x0F, 0x09, 0x0F, 0x06,
	0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x10, 0x10, 0x00,
	0x30, 0x40, 0xFF, 0x01, 0x01,
	0x00, 0x1F, 0x01, 0x01, 0x1E,
	0x00, 0x19, 0x1D, 0x17, 0x12,
	0x00, 0x3C, 0x3C, 0x3C, 0x3C,
	0x00, 0x00, 0x00, 0x00, 0x00  // #255 NBSP
};

/*
• When SCE is HIGH, SCLK clock signals are ignored;
during the HIGH time of SCE, the serial interface is
initialized (see Fig.12)
• SDIN is sampled at the positive edge of SCLK
• D/C indicates whether the byte is a command (D/C = 0)
or RAM data (D/C = 1); it is read with the eighth SCLK
pulse
*/

void lcd_5110_reset(){
	//printf("LCD reset start \n");
	LCD_5110_DELAY;
	LCD_5110_RST_LOW;
	LCD_5110_DELAY;
	LCD_5110_RST_HIGH;
	// lcd_5110_clear();
	//printf("LCD reset done \n");
	return;
}

void lcd_5110_clear(){
	int x, y;
	lcd_5110_setAddressX(0);
	lcd_5110_setAddressY(0);
	//printf("Clearing status: ");
	for(x=0; x<=83; x++){
		for(y=0; y<=5; y++){
			lcd_5110_sendData(0);
		}
		//printf("-");
	}
	//printf("\n");
	return;
}

void lcd_5110_clear_framebuffer(){
	int x, y;
	for(x=0; x<=83; x++){
		for(y=0; y<=5; y++) lcd_5110_pixels[x][y]=0;
	}
	return;
}

void lcd_5110_init(){
	//set data direction
	LCD_5110_RST_OUTPUT;
	LCD_5110_CE_OUTPUT;
	LCD_5110_DC_OUTPUT;
	LCD_5110_DIN_OUTPUT;
	LCD_5110_CLK_OUTPUT;
	
	//LCD_5110_CE_LOW;
	LCD_5110_CE_HIGH;
	LCD_5110_CLK_LOW;
	
	lcd_5110_reset();
	
	lcd_5110_sendCommand(0b00100001); //active, horizontal addressing, extended instruction set
	lcd_5110_sendCommand(0b10010000); //VOP register
	
	lcd_5110_sendCommand(0b00100000); //active, horizontal addressing, basic instruction set
	//delay(1000);
	//printf("All segments on!\n");
	lcd_5110_sendCommand(0b00001001); //all segments on
	//delay(1000);
	//printf("Inverted!\n");
	lcd_5110_sendCommand(0b00001101); //inverted
	//delay(1000);
	lcd_5110_sendCommand(0b00001100); //normal mode
	lcd_5110_clear();
	//printf("LCD_5110 initialize done \n");
	return;
}

void lcd_5110_sendByte(int data){ //optimize me?
	int i;
	LCD_5110_DELAY;
	LCD_5110_CE_LOW;
	LCD_5110_DELAY;


//	//bitbang SPI
//	for (i=0; i<8; i++){
//		if(data&0b10000000) LCD_5110_DIN_HIGH;
//		else LCD_5110_DIN_LOW;
//		//if(data&0b10000000) printf("1"); //DEBUG LINE
//		//else printf("0"); //DEBUG LINE
//		LCD_5110_DELAY;
//		LCD_5110_CLK_HIGH;
//		LCD_5110_DELAY;
//		LCD_5110_CLK_LOW;
//		data=data<<1;
//	} //bitbang SPI ends

	l11uxx_spi_sendByte(1, data);


	//printf("\n"); //DEBUG LINE
	LCD_5110_CE_HIGH;
	LCD_5110_DELAY;
	
	return;
}

void lcd_5110_sendData(int data){ //DC=1
	//printf("Writing data: "); //DEBUG LINE
	LCD_5110_DELAY;
	LCD_5110_DC_HIGH;
	LCD_5110_DELAY;
	lcd_5110_sendByte(data);
	return;
}

void lcd_5110_sendCommand(int data){ //DC=0
	//printf("Writing command: "); //DEBUG LINE
	LCD_5110_DELAY;
	LCD_5110_DC_LOW;
	LCD_5110_DELAY;
	lcd_5110_sendByte(data);
	return;
}

void lcd_5110_setAddressX(int address){
	address|=0b10000000;
	lcd_5110_sendCommand(address);
	return;
}

void lcd_5110_setAddressY(int address){
	address&=0b01000111;
	address|=0b01000000;
	lcd_5110_sendCommand(address);
	return;
}

void lcd_5110_setPixel(int x, int y, int state){
	int y_vector=0;
	int i;
	if(y>7)for(i=y; i>7; i=i-8, y_vector++); //get y_vector //not sure if "if" is necessary
	y=y-y_vector*8;//set y to value inside vector
	if(state) lcd_5110_pixels[x][y_vector]|=1<<y;
	else lcd_5110_pixels[x][y_vector]&=~(1<<y);
	return;
}

void lcd_5110_rect(int x1, int y1, int x2, int y2, int state){
	int i;
	if(x1>x2){ //so that x1 is always smaller than x2
		i=x1;
		x1=x2;
		x2=i;
	}
	if(y1>y2){ //so that y1 is always smaller than y2
		i=y1;
		y1=y2;
		y2=i;
	}
	//for(j=y1; j<=y2; j++) for(i=x1; i<=x2; i++) lcd_5110_setPixel(i, j, state); //this does fill I think
	for(i=x1; i<=x2; i++){
		lcd_5110_setPixel(i, y1, state);
		lcd_5110_setPixel(i, y2, state);
	}
	for(i=y1; i<=y2; i++){
		lcd_5110_setPixel(x1, i, state);
		lcd_5110_setPixel(x2, i, state);
	}
	
	return;
}

void lcd_5110_rect_fill(int x1, int y1, int x2, int y2, int state){
	int i, j;
	if(x1>x2){ //so that x1 is always smaller than x2
		i=x1;
		x1=x2;
		x2=i;
	}
	if(y1>y2){ //so that y1 is always smaller than y2
		i=y1;
		y1=y2;
		y2=i;
	}
	for(j=y1; j<=y2; j++) for(i=x1; i<=x2; i++) lcd_5110_setPixel(i, j, state); //this does fill I think
	return;
}

void lcd_5110_arc(int cx, int cy, int r, int startAng, int endAng, int state){
	//0 is top, angle measured counterclockwise
	//sin calculates x
	//cos calculates y
	double x, y;
	while(startAng!=endAng){
		//x =  cx - (sin (startAng*PI/180))*r;
		x =  cx - (sin (startAng*PI/180))*r*8/7; //even tho mathematically wrong, makes visually more pleasing circles
		y =  cy - (cos (startAng*PI/180))*r;	
		lcd_5110_setPixel((int) x, (int) y, state);
		startAng++;
		if(startAng>360)startAng=0;
	}
	return;
}

void lcd_5110_line_polar(int cx, int cy, int r1, int r2, int startAng, int state){
	//0 is top, angle measured counterclockwise
	//sin calculates x
	//cos calculates y
	double x, y;
	int i;
	if(r1>r2){
		i=r1;
		r1=r2;
		r2=i;
	}
	
	while(r1<=r2){
		//x =  cx - (sin (startAng*PI/180))*r1;
		x =  cx - (sin (startAng*PI/180))*r1*8/7; //even tho mathematically wrong, makes visually more pleasing circles
		y =  cy - (cos (startAng*PI/180))*r1;	
		lcd_5110_setPixel((int) x, (int) y, state);
		r1++;
	}
	return;
}

void lcd_5110_line(int x1, int y1, int x2, int y2, int state){
	int i;
	double slope;
	if(x1>x2){
		i=x1;
		x1=x2;
		x2=i;
	}
	if(y1>y2){
		i=y1;
		y1=y2;
		y2=i;
	}
	if((x2-x1)>(y2-y1)){
		slope=(double)(((double)(y2-y1))/((double)(x2-x1)));
		while(x1<=x2){
			lcd_5110_setPixel(x1, x1*slope, state);
			x1++;
		}
	}else if((y2-y1)>(x2-x1)){
		slope=(double)(((double)(x2-x1))/((double)(y2-y1)));
		while(y1<=y2){
			lcd_5110_setPixel(y1/slope, y1, state);
			y1++;
		}
	}
	return;
}

void lcd_5110_redraw(){
	int x, y_vector;
	lcd_5110_setAddressX(0);
	lcd_5110_setAddressY(0);
	for(y_vector=0; y_vector<=6; y_vector++){
		for(x=0; x<=83; x++){ 
			lcd_5110_sendData(lcd_5110_pixels[x][y_vector]);
		}
		lcd_5110_setAddressX(0); //comment these out, see what happens?
		lcd_5110_setAddressY(y_vector+1); //comment these out, see what happens?
	}
	return;
}

void lcd_5110_printMemory(){
	int i;
	int j=1;
	int k;
	printf("\n BEGIN DUMP \n");
	printf("+"); //does cute frame
	for(i=0; i<84; i++)printf("-"); //does cute frame
	printf("+\n"); //does cute frame
	for(k=0; k<=5; k++){
		for(j=3; j<255; j=j<<2){
			printf("|"); //does cute frame
			for(i=0; i<=83; i++){
				if ((lcd_5110_pixels[i][k])&(j&(j<<1))&&(lcd_5110_pixels[i][k])&(j&(j>>1))) printf(":");
				else if ((lcd_5110_pixels[i][k])&(j&(j>>1))) printf("'");
				else if ((lcd_5110_pixels[i][k])&(j&(j<<1))) printf(".");
				else printf(" ");
			}
			printf("|\n"); //does cute frame
		}
	}
	printf("+"); //does cute frame
	for(i=0; i<84; i++)printf("-"); //does cute frame
	printf("+"); //does cute frame
	printf("\n END DUMP \n");
	return;
}

void lcd_5110_shiftRowsUp(int rows){
	int i;
	int j;
	for(j=0; j<5; j=j+rows){
		for(i=0; i<83; i++){
			lcd_5110_pixels[i][j]=lcd_5110_pixels[i][j+1];
		}
	}
	for(i=0; i<83; i++){
				lcd_5110_pixels[i][j]=0;
	}
	//clear last line

}

void lcd_5110_printString(int charColumn, int charRow, char text[20]){ //
	//note that charColumn is for 1602 style addressing, characterColumn is column of character (0 to 4)
	int characterNumber=0;
	int characterColumn=0;
	char initialColumn = charColumn; //for \r
	//int debugvalue=0;
	while(text[characterNumber] != 0){
		//lcd_5110_pixels[84][6];

		//idiotprotector
		if(charRow>6) return;
		if((charColumn*6+characterNumber*6+characterColumn)>84) return;

		lcd_5110_pixels[charColumn*6+characterNumber*6+characterColumn][charRow] = LCDBitmapFont[((text[characterNumber])*5+characterColumn)];
		//debugvalue=LCDBitmapFont[((text[characterNumber]*5)+characterColumn)];//debug!

		characterColumn++;
		//lcd_5110_redraw(); //DEBUG
		if(characterColumn>4){
			characterNumber++;
			characterColumn=0;

		}
	}
	return;
}


char lcd_5110_console_cursorRow=0; //y
char lcd_5110_console_cursorColumn=0; //x

void lcd_5110_consolePosition_jump(char posColumn, char posRow){
	lcd_5110_console_cursorRow=posRow;
	lcd_5110_console_cursorColumn=posColumn;
	return;
}

void lcd_5110_printAsConsole(char text[40], char violentUpdate){

	char currentCharacterOfMessage=0;
	char oneCharMessage[2];
	oneCharMessage[1] = 0;

	char lengthOfMessage = strlen(text);

	while(currentCharacterOfMessage < lengthOfMessage){

		if(text[currentCharacterOfMessage] == '\n'){
			lcd_5110_console_cursorRow++;
			if(lcd_5110_console_cursorRow>5){
				lcd_5110_shiftRowsUp(1);
				lcd_5110_console_cursorRow = 5;
				//cursorColumn = 0;
			}
		} else if (text[currentCharacterOfMessage] == '\r'){
			lcd_5110_console_cursorColumn = 0;

		}
		else {
			oneCharMessage[0] = text[currentCharacterOfMessage];
			lcd_5110_printString(lcd_5110_console_cursorColumn, lcd_5110_console_cursorRow, oneCharMessage);
			lcd_5110_console_cursorColumn++;
			if(lcd_5110_console_cursorColumn > 13){
				if(lcd_5110_console_cursorRow<5) lcd_5110_console_cursorRow++;
				else lcd_5110_shiftRowsUp(1);
				lcd_5110_console_cursorColumn=0;
			}
		}
		currentCharacterOfMessage++;
		if(violentUpdate) lcd_5110_redraw(); //update after each character
	}
	return;
}

/*void lcd_5110_printMemory(){
	int i;
	int j=1;
	int k;
	printf("\n BEGIN DUMP \n");
	for(k=0; k<=5; k++){
		for(j=1; j<=128; j=j<<1){
			for(i=0; i<=83; i++){
				if ((lcd_5110_pixels[i][k])&j) printf("X");
				else printf("-");
			}
			printf("\n");
		}
		
	}
	printf("\n END DUMP \n");
	return;
}*/
