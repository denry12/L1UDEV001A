/*
 * ILI9341.c
 *
 *  Created on: 02.10.2016
 *      Author: Denry
 */

// https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf

#include <stdint.h>
#include <stdbool.h>
#include "ILI9341.h"

int temporaryValue = 0;
int randomcounter = 0;





//this here is just to test speeding up SPI
#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif

bool ILI9341_addToTxBuffer(ili9341_instance *instance, uint16_t *command){

	//best way to bypass buffer, very fast (whoosh)
	//when speed needed, add a way to CSen/CSdis/addData/addInstr to bypass buffer and send directly to HW
	/*
	int commandlocal = ((int)(command))&0xFFF;
	if (commandlocal & (1<<ILI9341_CSCHANGE_BIT_IN_BUFFER)){
		while (LPC_SSP1->SR & (0x1 << 4)); //wait while SPI busy //ATTN, locks processor!
		GPIOSetValue(1, 29, ((~commandlocal) & 0x01));
		return 0;
	}
	if(commandlocal & (1<<ILI9341_DC_BIT_IN_BUFFER)) GPIOSetValue(1, 31, 1);
	else GPIOSetValue(1, 31, 0);
	l11uxx_spi_sendByte(1, commandlocal&0xFF);
	return 0;
	*/


/*
	int commandlocal = ((int)(command))&0xFFF;
		if (commandlocal & (1<<ILI9341_CSCHANGE_BIT_IN_BUFFER)){
			GPIOSetValue(1, 29, ((~commandlocal) & 0x01));
			return 0;
		}

		if(commandlocal & (1<<ILI9341_DC_BIT_IN_BUFFER)) GPIOSetValue(1, 31, 1);
		else GPIOSetValue(1, 31, 0);
		l11uxx_spi_sendByte(1, commandlocal&0xFF);

		return 0;

*/

/*
	int commandlocal = ((int)(command))&0xFFF;
	if (commandlocal & (1<<ILI9341_CSCHANGE_BIT_IN_BUFFER)) return 0;
	GPIOSetValue(1, 29, 0);
	if(commandlocal & (1<<ILI9341_DC_BIT_IN_BUFFER)) GPIOSetValue(1, 31, 1);
	else GPIOSetValue(1, 31, 0);
	l11uxx_spi_sendByte(1, commandlocal&0xFF);
	GPIOSetValue(1, 29, 1);
	return 0;
*/


	if(instance->LCDCommandsInBuffer < (instance->LCDCommandBufferSize - 2)){ //verify that there is room (and to spare)
		//if define = 20, then comminbuffer can be up to 18 (cause "17 < (20-2)" still enters loop)
		instance->LCDCommandBuffer[instance->LCDCommandBufferIndex] = command;

		instance->LCDCommandBufferIndex++;
		instance->LCDCommandsInBuffer++;
		if(instance->LCDCommandBufferIndex >= instance->LCDCommandBufferSize) //if e.g. 20 (comes from define), set to 0, so max value used is 19
			instance->LCDCommandBufferIndex = 0; //go circular if necessary
	}
	else {
		//bitbangUARTmessage("LCD BUFFER FULL!\r\n");
		while((*instance).handlerFunction(instance) == 0); //empty buffer a bit then
		return ILI9341_addToTxBuffer(instance, command);
	}
	return 0; //everything success

}

bool ILI9341_getFromTxBuffer(ili9341_instance *instance, uint8_t *data, bool *DC, bool *CSchange){

	//write areas are 0 (including) to 19 (including) when buffersize 20
	if(instance->LCDCommandsInBuffer <= 0) return 1; //if no commands at all, return instantly
	else {
		int firstUnsentPacket = instance->LCDCommandBufferIndex - instance->LCDCommandsInBuffer; //unsent = 0...19 - 0...18. E.g.XX: 15 - 19 = -4
		if (firstUnsentPacket < 0) //e.g.XX: and if -4
			firstUnsentPacket += instance->LCDCommandBufferSize; //e.g.XX: -4 + 20 = 16

		if(instance->LCDCommandsInBuffer){ //this is duplicated above as of right now
			*data =   (uint8_t)(((instance->LCDCommandBuffer[firstUnsentPacket])                                  ) & 0xFF);
			*DC   =      (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> ILI9341_DC_BIT_IN_BUFFER      ) & 0x01);
			*CSchange =  (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> ILI9341_CSCHANGE_BIT_IN_BUFFER) & 0x01);

			instance->LCDCommandsInBuffer--;
		}
	}

	return 0; //everything success
}


bool ILI9341_CS_enable(ili9341_instance *instance){
	uint16_t command = ((0xFF) | (1<<ILI9341_CSCHANGE_BIT_IN_BUFFER));
	bool response = 0;
	response |= ILI9341_addToTxBuffer(instance, command);
	return response;
}

bool ILI9341_CS_disable(ili9341_instance *instance){
	uint16_t command = ((0x00) | (1<<ILI9341_CSCHANGE_BIT_IN_BUFFER));
	bool response = 0;
	response |= ILI9341_addToTxBuffer(instance, command);
	return response;
}

//for data, D/C is set low (voltage wise)
bool ILI9341_addInstructionToBuffer(ili9341_instance *instance, uint16_t command){ //8-bit command: (MSB) D7 D6 D5 D4 D3 D2 D1 D0 (LSB)
	command = (command & 0xFF);
	bool response = 0;
	//response |= ILI9341_CS_enable(instance);
	response |= ILI9341_addToTxBuffer(instance, command);
	//response |= ILI9341_CS_disable(instance);
	return response;
}

//for data, D/C is set high (voltage wise)
bool ILI9341_addDataToBuffer(ili9341_instance *instance, uint16_t command){ //8-bit command: (MSB) D7 D6 D5 D4 D3 D2 D1 D0 (LSB)
	command = ((command & 0xFF) | (1<<ILI9341_DC_BIT_IN_BUFFER));
	bool response = 0;
	//response |= ILI9341_CS_enable(instance);
	response |= ILI9341_addToTxBuffer(instance, command);
	//response |= ILI9341_CS_disable(instance);
	return response;
}

bool ILI9341_begin(ili9341_instance *instance) {
	// toggle RST low to reset

	//GPIOSetValue(1, 28, 0); //do reset just in case
	//delay(300);
	//GPIOSetValue(1, 28, 1); //wake from reset
	//delay(300);

	//ILI9341_RST_disable();
	//delay(5);
	//ILI9341_RST_enable();
	//delay(20);
	//ILI9341_RST_disable();
	//delay(150);

	ILI9341_CS_enable(instance);

	ILI9341_addInstructionToBuffer(instance,0xEF); //wtf even is that
	ILI9341_addDataToBuffer(instance,0x03);
	ILI9341_addDataToBuffer(instance,0x80);
	ILI9341_addDataToBuffer(instance,0x02);

	ILI9341_addInstructionToBuffer(instance,0xCF);
	ILI9341_addDataToBuffer(instance,0x00);
	ILI9341_addDataToBuffer(instance,0XC1);
	ILI9341_addDataToBuffer(instance,0X30);

	ILI9341_addInstructionToBuffer(instance,0xED);
	ILI9341_addDataToBuffer(instance,0x64);
	ILI9341_addDataToBuffer(instance,0x03);
	ILI9341_addDataToBuffer(instance,0X12);
	ILI9341_addDataToBuffer(instance,0X81);

	ILI9341_addInstructionToBuffer(instance,0xE8);
	ILI9341_addDataToBuffer(instance,0x85);
	ILI9341_addDataToBuffer(instance,0x00);
	ILI9341_addDataToBuffer(instance,0x78);

	ILI9341_addInstructionToBuffer(instance,0xCB);
	ILI9341_addDataToBuffer(instance,0x39);
	ILI9341_addDataToBuffer(instance,0x2C);
	ILI9341_addDataToBuffer(instance,0x00);
	ILI9341_addDataToBuffer(instance,0x34);
	ILI9341_addDataToBuffer(instance,0x02);

	ILI9341_addInstructionToBuffer(instance,0xF7);
	ILI9341_addDataToBuffer(instance,0x20);

	ILI9341_addInstructionToBuffer(instance,0xEA);
	ILI9341_addDataToBuffer(instance,0x00);
	ILI9341_addDataToBuffer(instance,0x00);

	ILI9341_addInstructionToBuffer(instance,0xC0);    //Power control
	ILI9341_addDataToBuffer(instance,0x23);   //VRH[5:0]

	ILI9341_addInstructionToBuffer(instance,0xC1);    //Power control
	ILI9341_addDataToBuffer(instance,0x10);   //SAP[2:0];BT[3:0]

	ILI9341_addInstructionToBuffer(instance,0xC5);    //VCM control
	ILI9341_addDataToBuffer(instance,0x3e); //
	ILI9341_addDataToBuffer(instance,0x28);

	ILI9341_addInstructionToBuffer(instance,0xC7);    //VCM control2
	ILI9341_addDataToBuffer(instance,0x86);  //--

	ILI9341_addInstructionToBuffer(instance,0x36);    // Memory Access Control
	ILI9341_addDataToBuffer(instance,0x40 | 0x08);

	ILI9341_addInstructionToBuffer(instance,0x3A);
	ILI9341_addDataToBuffer(instance,0x55);

	ILI9341_addInstructionToBuffer(instance,0xB1);
	ILI9341_addDataToBuffer(instance,0x00);
	ILI9341_addDataToBuffer(instance,0x18);

	ILI9341_addInstructionToBuffer(instance,0xB6);    // Display Function Control
	ILI9341_addDataToBuffer(instance,0x08);
	ILI9341_addDataToBuffer(instance,0x82);
	ILI9341_addDataToBuffer(instance,0x27);

	ILI9341_addInstructionToBuffer(instance,0xF2);    // 3Gamma Function Disable
	ILI9341_addDataToBuffer(instance,0x00);

	ILI9341_addInstructionToBuffer(instance,0x26);    //Gamma curve selected
	ILI9341_addDataToBuffer(instance,0x01);

	ILI9341_addInstructionToBuffer(instance,0xE0);    //Set Gamma
	ILI9341_addDataToBuffer(instance,0x0F);
	ILI9341_addDataToBuffer(instance,0x31);
	ILI9341_addDataToBuffer(instance,0x2B);
	ILI9341_addDataToBuffer(instance,0x0C);
	ILI9341_addDataToBuffer(instance,0x0E);
	ILI9341_addDataToBuffer(instance,0x08);
	ILI9341_addDataToBuffer(instance,0x4E);
	ILI9341_addDataToBuffer(instance,0xF1);
	ILI9341_addDataToBuffer(instance,0x37);
	ILI9341_addDataToBuffer(instance,0x07);
	ILI9341_addDataToBuffer(instance,0x10);
	ILI9341_addDataToBuffer(instance,0x03);
	ILI9341_addDataToBuffer(instance,0x0E);
	ILI9341_addDataToBuffer(instance,0x09);
	ILI9341_addDataToBuffer(instance,0x00);

	ILI9341_addInstructionToBuffer(instance,0xE1);    //Set Gamma
	ILI9341_addDataToBuffer(instance,0x00);
	ILI9341_addDataToBuffer(instance,0x0E);
	ILI9341_addDataToBuffer(instance,0x14);
	ILI9341_addDataToBuffer(instance,0x03);
	ILI9341_addDataToBuffer(instance,0x11);
	ILI9341_addDataToBuffer(instance,0x07);
	ILI9341_addDataToBuffer(instance,0x31);
	ILI9341_addDataToBuffer(instance,0xC1);
	ILI9341_addDataToBuffer(instance,0x48);
	ILI9341_addDataToBuffer(instance,0x08);
	ILI9341_addDataToBuffer(instance,0x0F);
	ILI9341_addDataToBuffer(instance,0x0C);
	ILI9341_addDataToBuffer(instance,0x31);
	ILI9341_addDataToBuffer(instance,0x36);
	ILI9341_addDataToBuffer(instance,0x0F);

	ILI9341_addInstructionToBuffer(instance,0x11);    //Exit Sleep
	delay(120);
	ILI9341_addInstructionToBuffer(instance,0x29);    //Display on

	ILI9341_CS_disable(instance);

	return 0;

}


bool ILI9341_setAddrWindow(ili9341_instance *instance, int x0, int y0, int x1, int y1) { //ints used here were 16bit

	if((x0 < 0) ||(x1 >= instance->xResolution) || (y0 < 0) || (y1 >= instance->yResolution)) return 1; //coordinate error
	if(( x0 > x1) || (y0 > y1)) return 1; //coordinate error
	ILI9341_CS_enable(instance);

	ILI9341_addInstructionToBuffer(instance,0x2A); // Column addr set
	ILI9341_addDataToBuffer(instance,x0 >> 8);
	ILI9341_addDataToBuffer(instance,x0 & 0xFF);     // XSTART
	ILI9341_addDataToBuffer(instance,x1 >> 8);
	ILI9341_addDataToBuffer(instance,x1 & 0xFF);     // XEND

	ILI9341_addInstructionToBuffer(instance,0x2B); // Row(page) addr set
	ILI9341_addDataToBuffer(instance,y0>>8);
	ILI9341_addDataToBuffer(instance,y0);     // YSTART
	ILI9341_addDataToBuffer(instance,y1>>8);
	ILI9341_addDataToBuffer(instance,y1);     // YEND

	ILI9341_addInstructionToBuffer(instance,0x2C); // write to RAM

	ILI9341_CS_disable(instance);
	return 0;
}

bool ILI9341_drawPixel(ili9341_instance *instance, int x, int y, uint16_t color) { //ints used here were 16bit

	// if((x < 0) ||(x >= instance->xResolution) || (y < 0) || (y >= instance->yResolution)) return 1; //this is checked in setAddrWindow

	ILI9341_setAddrWindow(instance, x,y,x+1,y+1);

	ILI9341_CS_enable(instance);
	ILI9341_addDataToBuffer(instance, color >> 8); //l11uxx_spi_sendByte(1, color >> 8); //spiwrite(color >> 8);
	ILI9341_addDataToBuffer(instance, color); //l11uxx_spi_sendByte(1, color); //spiwrite(color);
	ILI9341_CS_disable(instance);
	return 0;
}

void ILI9341_drawFastVLine(ili9341_instance *instance, int x, int y, int h, uint16_t color) { //ints used here were 16bit

	// Rudimentary clipping
	// if((x < 0) ||(x >= instance->xResolution) || (y < 0) || (y >= instance->yResolution)) return 1; //this is checked in setAddrWindow

	if((y+h-1) >= instance->yResolution)
		h = instance->yResolution-y;

	ILI9341_setAddrWindow(instance, x, y, x, y+h-1);

	uint8_t colourMSB = color >> 8, colourLSB = color;

	//ILI9341_DC_enable(instance);
	//digitalWrite(_dc, HIGH);
	ILI9341_CS_enable(instance);
	//digitalWrite(_cs, LOW);

	while (h--) {
		ILI9341_addDataToBuffer(instance, colourMSB);
		ILI9341_addDataToBuffer(instance, colourLSB);
	}
	ILI9341_CS_disable(instance);
	//digitalWrite(_cs, HIGH);
}


bool ILI9341_drawFastHLine(ili9341_instance *instance, int x, int y, int w, uint16_t color) { //ints used here were 16bit

	// Rudimentary clipping
	// if((x >= instance->xResolution) || (y >= instance->yResolution)) return; //this is checked in setAddrWindow
	// if((x+w-1) >= instance->xResolution)  w = instance->yResolution-x; //this is checked in setAddrWindow
	ILI9341_setAddrWindow(instance, x, y, x+w-1, y);

	uint8_t colourMSB = color >> 8, colourLSB = color;
	ILI9341_CS_enable(instance);
	while (w--) {
		ILI9341_addDataToBuffer(instance, colourMSB);
		ILI9341_addDataToBuffer(instance, colourLSB);
	}
	ILI9341_CS_disable(instance);

	return 0;
}



// fill a rectangle
bool ILI9341_fillRect(ili9341_instance *instance, int x, int y, int w, int h, uint16_t color) { //ints used here were 16bit

	// rudimentary clipping (drawChar w/big text requires this)
	//if((x >= ILI9341_LCD_WIDTH) || (y >= ILI9341_LCD_HEIGHT)) return; //this is checked in setAddrWindow
	//if((x + w - 1) >= ILI9341_LCD_WIDTH)  w = ILI9341_LCD_WIDTH  - x; //this is checked in setAddrWindow
	//if((y + h - 1) >= ILI9341_LCD_HEIGHT) h = ILI9341_LCD_HEIGHT - y; //this is checked in setAddrWindow

	ILI9341_setAddrWindow(instance, x, y, x+w-1, y+h-1);

	uint8_t colourMSB = color >> 8, colourLSB = color;

	ILI9341_CS_enable(instance);
	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
			ILI9341_addDataToBuffer(instance, colourMSB);
			ILI9341_addDataToBuffer(instance, colourLSB);
		}
	}
	ILI9341_CS_disable(instance);

	return 0;
}

bool ILI9341_fillScreen(ili9341_instance *instance, int color) { //ints used here were 16bit
	return ILI9341_fillRect(instance, 0, 0,  instance->xResolution, instance->yResolution, color);
}


void drawPokey(ili9341_instance *instance, unsigned int x, unsigned int y)
{
	/*
	THIS CODE WILL DRAW POKEY THE PENGUIN ON YOUR SCREEN!!!
	http://yellow5.com/pokey FOR MORE INFO!!!

	MAYBE IT WORKS NOW!!!!!

	 */
	const unsigned int yellow = 0xFF00;//colormix(255, 255, 0);
	//if doesn't work,try other color
	//looked it up in a MSX manual,straight outta 1987!!!
	const unsigned int black = 0x00;
	const unsigned int white = 65535;

	ILI9341_fillRect(instance, x, y, 36 ,50 , white); //make area white i hope
	//let's draw Pokey now!!!
	ILI9341_drawFastVLine(instance, x, y+15, 8, black);
	ILI9341_drawFastVLine(instance, x+1, y+13, 17, black);
	ILI9341_drawFastVLine(instance, x+2, y+11, 24, black);
	ILI9341_drawFastVLine(instance, x+3, y+9, 34, black);
	ILI9341_drawFastVLine(instance, x+4, y+9, 35, black);
	ILI9341_drawFastVLine(instance, x+5, y+8, 38, black);
	ILI9341_drawFastVLine(instance, x+6, y+7, 39, black);
	ILI9341_drawFastVLine(instance, x+7, y+7, 39, black);
	ILI9341_drawFastVLine(instance, x+8, y+6, 41, black);
	ILI9341_drawFastVLine(instance, x+9, y+5, 43, black);
	ILI9341_drawFastVLine(instance, x+10, y+3, 45, black);
	ILI9341_drawFastVLine(instance, x+11, y+2, 45, black); //black back part ends,time to fuck around with whites and shit
	ILI9341_drawFastVLine(instance, x+12, y+2, 45, black); //black stripe
	ILI9341_drawFastVLine(instance, x+12, y+25, 3, white); //3 pixels white near wing
	ILI9341_drawFastVLine(instance, x+13, y+1, 46, black);
	ILI9341_drawFastVLine(instance, x+13, y+29, 8, white);
	ILI9341_drawFastVLine(instance, x+14, y+1, 47, black);
	ILI9341_drawFastVLine(instance, x+14, y+32, 9, white);
	ILI9341_drawFastVLine(instance, x+15, y, 48, black);
	ILI9341_drawFastVLine(instance, x+15, y+33, 8, white);
	ILI9341_drawFastVLine(instance, x+16, y, 48, black);
	ILI9341_drawFastVLine(instance, x+16, y+33, 9, white);
	ILI9341_drawFastVLine(instance, x+17, y, 48 , black);
	ILI9341_drawFastVLine(instance, x+17, y+27, 16, white);
	ILI9341_drawFastVLine(instance, x+17, y+43, 2, white);
	ILI9341_drawFastVLine(instance, x+18, y, 48, black);
	ILI9341_drawFastVLine(instance, x+18, y+19, 25, white);
	ILI9341_drawFastVLine(instance, x+18, y+44, 1, white);
	ILI9341_drawFastVLine(instance, x+19, y, 48, black);
	ILI9341_drawFastVLine(instance, x+19, y+7, 3, white); //1st part of eye
	ILI9341_drawFastVLine(instance, x+19, y+18, 27, white);
	ILI9341_drawFastVLine(instance, x+20, y, 48, black);
	ILI9341_drawFastVLine(instance, x+20, y+7, 4, white); //2nd part of eye
	ILI9341_drawFastVLine(instance, x+20, y+17, 29, white);
	ILI9341_drawFastVLine(instance, x+21, y, 48, black);
	ILI9341_drawFastVLine(instance, x+21, y+14, 31, white);
	ILI9341_drawFastVLine(instance, x+22, y, 49, black);
	ILI9341_drawFastVLine(instance, x+22, y+13, 32, white);
	ILI9341_drawFastVLine(instance, x+23, y, 50, black);
	ILI9341_drawFastVLine(instance, x+23, y+9, 1, white);
	ILI9341_drawFastVLine(instance, x+23, y+12, 33, white);
	ILI9341_drawFastVLine(instance, x+24, y, 9, black);
	ILI9341_drawFastVLine(instance, x+24, y+43, 2, black);
	ILI9341_drawFastVLine(instance, x+24, y+46, 2, black);
	ILI9341_drawFastVLine(instance, x+25, y+1, 8, black);
	ILI9341_drawFastVLine(instance, x+25, y+11, 7, black);
	ILI9341_drawFastVLine(instance, x+25, y+43, 1, black);
	ILI9341_drawFastVLine(instance, x+25, y+46, 3, black);
	ILI9341_drawFastVLine(instance, x+26, y+2, 8, black);
	ILI9341_drawFastVLine(instance, x+26, y+42, 1, black);
	ILI9341_drawFastVLine(instance, x+26, y+46, 2, black);
	ILI9341_drawFastVLine(instance, x+27, y+4, 6, black);
	ILI9341_drawFastVLine(instance, x+27, y+41, 1, black);
	ILI9341_drawFastVLine(instance, x+27, y+46, 2, black);
	ILI9341_drawFastVLine(instance, x+28,y+6, 3, black);
	ILI9341_drawFastVLine(instance, x+28,y+41, 2, black);
	ILI9341_drawFastVLine(instance, x+29, y+8, 2, black);
	ILI9341_drawFastVLine(instance, x+29, y+16, 3, black);
	ILI9341_drawFastVLine(instance, x+29, y+40, 1, black);
	ILI9341_drawFastVLine(instance, x+30, y+9, 3, black);
	ILI9341_drawFastVLine(instance, x+30, y+19, 2, black);
	ILI9341_drawFastVLine(instance, x+30, y+36, 4, black);
	ILI9341_drawFastVLine(instance, x+31, y+21, 14, black);
	//let's draw a beak!!!
	ILI9341_drawFastHLine(instance, x+27, y+11, 3, black);
	ILI9341_drawFastHLine(instance, x+26, y+12, 6, yellow);
	ILI9341_drawFastHLine(instance, x+32, y+12, 2, black);
	ILI9341_drawFastHLine(instance, x+26, y+13, 8, yellow);
	ILI9341_drawFastHLine(instance, x+26, y+14, 10, black);
	ILI9341_drawFastHLine(instance, x+26, y+15, 8, yellow);
	ILI9341_drawFastHLine(instance, x+26, y+16, 3, yellow);
	ILI9341_drawFastHLine(instance, x+26, y+17, 3, black);
	ILI9341_drawFastHLine(instance, x+30, y+16, 4, black);
	ILI9341_drawFastVLine(instance, x+34, y+13, 3, black);
	//pokey done!!!
}


const unsigned char ILI9341_LCDBitmapFont[] = {
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
		0x7F, 0x40, 0x40, 0x40, 0x40, //L
		0x7F, 0x02, 0x1C, 0x02, 0x7F,
		0x7F, 0x04, 0x08, 0x10, 0x7F,
		0x3E, 0x41, 0x41, 0x41, 0x3E, //O
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


void ILI9341_printString_bg(ili9341_instance *instance, int x, int y, int textColour, int bgColour, char text[100]){ //
	//Y coordinate is top left corner of text
	//40 chars * 40 chars (sic!) max
	int characterNumber=0;
	int characterColumn=0;
	int currentRow=0;
	int lineLengthText=0;
	int lineLengthBg=0;


	//todo: predraw background to text to get faster

	//idiotprotector
	//if(charRow>6) return;
	//if((charColumn*6+characterNumber*6+characterColumn)>84) return;
	//lol idiotprotector disabled


	while(text[characterNumber] != 0){
		while(characterColumn<5){
			while(currentRow<7){
				while((ILI9341_LCDBitmapFont[((text[characterNumber]*5)+characterColumn)]) & (1<<currentRow)){
					//"dark" pixel
					if(currentRow>6) break;
					lineLengthText++;
					currentRow++;
				}
				ILI9341_drawFastVLine(instance, x+characterColumn+6*characterNumber, y+currentRow-lineLengthText, lineLengthText, textColour);
				while(!((ILI9341_LCDBitmapFont[((text[characterNumber]*5)+characterColumn)]) & (1<<currentRow))){
					//"light" pixel
					if(currentRow>6) break;
					lineLengthBg++;
					currentRow++;
				}

				ILI9341_drawFastVLine(instance, x+characterColumn+6*characterNumber, y+currentRow-lineLengthBg, lineLengthBg, bgColour);
				lineLengthText=0;
				lineLengthBg=0;
			}
			if((x+6*characterNumber >= instance->xResolution) || (y >= instance->yResolution)) return;
			characterColumn++;
			currentRow=0;
		}

		//do bgcolor line between characters
		//but not for last character
		if(text[characterNumber+1] != 0) ILI9341_drawFastVLine(instance, x+6*characterNumber+5, y, 7, bgColour);

		//do a bgcolor line under char
		//ILI9341_drawFastHLine(x+6*characterNumber, y+8, 6, bgColour);

		characterColumn=0;
		characterNumber++;
	}
	return;
}

bool ILI9341_invert(ili9341_instance *instance, bool inversion){
	bool response = 0;

	response |= ILI9341_CS_enable(instance);

	if (inversion) response |= ILI9341_addInstructionToBuffer(instance,0x21);
	else response |= ILI9341_addInstructionToBuffer(instance,0x20);

	response |= ILI9341_CS_disable(instance);
	return 0;
}


//might need also command "write ctrl display", 0x53
//^ without this definitely doesn't work. Not sure if this helps
//
//CONSIDER THIS FUNCTION BROKEN
bool ILI9341_setBrightness(ili9341_instance *instance, uint8_t brightness){
	bool response = 0;

	response |= ILI9341_CS_enable(instance);

	response |= ILI9341_addInstructionToBuffer(instance, 0x51);
	response |= ILI9341_addDataToBuffer(instance, brightness);

	response |= ILI9341_CS_disable(instance);
	return 0;
}


//use this for rotating display
bool ILI9341_setMemoryAccess(ili9341_instance *instance, bool rowAddrOrder, bool columnAddrOrder, bool rowColumnExchange){
	bool response = 0;

	uint8_t memAccessCtrl = (rowAddrOrder<<7)|(columnAddrOrder<<6)|(rowColumnExchange<<5)|(1<<3);

	response |= ILI9341_CS_enable(instance);

	response |= ILI9341_addInstructionToBuffer(instance, 0x36);
	response |= ILI9341_addDataToBuffer(instance, memAccessCtrl); // "begin" sets it as 0x40 | 0x08 (MX|BGR)

	response |= ILI9341_CS_disable(instance);
	return 0;
}
//examples:
//ILI9341_setMemoryAccess(&iliLCD01, 0, 0, 0); //draws pokey facing left, bottom at pins, text mirrored
//ILI9341_setMemoryAccess(&iliLCD01, 0, 0, 1); //draws pokey facing right, right at pins, text readable
//ILI9341_setMemoryAccess(&iliLCD01, 0, 1, 0); //draws pokey facing right, bottom at pins, text readable
//ILI9341_setMemoryAccess(&iliLCD01, 0, 1, 1); //draws pokey facing left, left at pins, text mirrored
//ILI9341_setMemoryAccess(&iliLCD01, 1, 0, 0); //draws pokey facing right, top at pins, text readable
//ILI9341_setMemoryAccess(&iliLCD01, 1, 0, 1); //draws pokey facing left, right at pins, text mirrored
//ILI9341_setMemoryAccess(&iliLCD01, 1, 1, 0); //draws pokey facing left, top at pins, text mirrored
//ILI9341_setMemoryAccess(&iliLCD01, 1, 1, 1); //draws pokey facing right, left at pins, text readable







bool ILI9341_init(ili9341_instance *instance, uint8_t *width, uint8_t *height){

	bool response = 0;

	instance->LCDCommandBufferIndex = 0;
	instance->LCDCommandsInBuffer = 0;
	instance->LCDCommandBufferSize = ILI9341_TX_BUFFER_SIZE;
	instance->xResolution = width;
	instance->yResolution = height;

	response |= ILI9341_CS_disable(instance);

	response |= ILI9341_begin(instance); //this should initialize something. To be broken down later into multiple commands

	return response;
}





// Input 0...255 each colour
// (is translated to: R to 5-bit (31), G to 6-bit (63), B to 5-bit(31))
uint16_t ILI9341_Color65k(uint16_t r, uint16_t g, uint16_t b) { //uint16 cause I am doing math with these
	r = (r&0xFF)/8; //255 -> 31
	g = (g&0xFF)/4; //255 -> 63
	b = (b&0xFF)/8; //255 -> 31

	//b = ( ((r & 31) << 11) | ((g & 63) << 5) | (b & 31) );
	//return b;

	return ( ((r & 31) << 11) | ((g & 63) << 5) | (b & 31) );
}
