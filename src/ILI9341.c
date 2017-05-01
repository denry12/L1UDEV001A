/*
 * ILI9341.c
 *
 *  Created on: 02.10.2016
 *      Author: Denry
 */




#include <stdint.h>
#include <stdbool.h>
#include "ILI9341.h"


int temporaryValue = 0;
int randomcounter = 0;

bool ILI9341_addToTxBuffer(ili9341_instance *instance, uint16_t *command){



	int commandlocal = ((int)(command))&0xFFF;
	if (commandlocal & (1<<ILI9341_CSCHANGE_BIT_IN_BUFFER)) return 0;
	GPIOSetValue(1, 29, 0);
	if(commandlocal & (1<<ILI9341_DC_BIT_IN_BUFFER)) GPIOSetValue(1, 31, 1);
	else GPIOSetValue(1, 31, 0);
	l11uxx_spi_sendByte(1, commandlocal&0xFF);
	GPIOSetValue(1, 29, 1);
	return 0;





	if(instance->LCDCommandsInBuffer < (instance->LCDCommandBufferSize - 2)){
		instance->LCDCommandBuffer[instance->LCDCommandBufferIndex] = command;

		//if(instance->LCDCommandsInBuffer == 0) //first command going in
		//	temporaryValue = instance->LCDCommandBufferIndex;

		instance->LCDCommandBufferIndex++;
		instance->LCDCommandsInBuffer++;
		if(instance->LCDCommandBufferIndex >= instance->LCDCommandBufferSize)
			instance->LCDCommandBufferIndex = 0; //go circular if necessary
		//bitbangUARTmessage("x");
		//while((*instance).handlerFunction(instance) == 0); //empty buffer a bit then
	}
	//else return 1; //catastrophe (can't fit, buffer full)
	else {
		bitbangUARTmessage("LCD BUFFER FULL!\r\n");
		while((*instance).handlerFunction(instance) == 0); //empty buffer a bit then
		return ILI9341_addToTxBuffer(instance, &command);
		//return 0; //and return success

	}
	return 0; //everything success

}

bool ILI9341_getFromTxBuffer(ili9341_instance *instance, uint8_t *data, bool *DC, bool *CSchange){
	int firstUnsentPacket = instance->LCDCommandBufferIndex - instance->LCDCommandsInBuffer;
	if (firstUnsentPacket < 0)
		firstUnsentPacket += instance->LCDCommandBufferSize+1;
	if(instance->LCDCommandsInBuffer){
		*data =   (uint8_t)(((instance->LCDCommandBuffer[firstUnsentPacket])                                  ) & 0xFF);
		*DC   =      (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> ILI9341_DC_BIT_IN_BUFFER      ) & 0x01);
		*CSchange =  (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> ILI9341_CSCHANGE_BIT_IN_BUFFER) & 0x01);
		instance->LCDCommandsInBuffer--;
		if(instance->LCDCommandBufferIndex == 0) instance->LCDCommandBufferIndex = instance->LCDCommandBufferSize; //go circular if necessary

	}
	else return 1; //catastrophe (nothing to read out)

	return 0; //everything success
}



bool ILI9341_buffertester(ili9341_instance *instance){
	uint16_t testval8;
	bool response = 0;
	while(temporaryValue < 999){
		testval8 = (temporaryValue & 0xFF);
		response |= ILI9341_addToTxBuffer(instance, testval8);
		temporaryValue++;
		if(randomcounter > 60){
			while((*instance).handlerFunction(instance) == 0); //empty buffer a bit then
			randomcounter = 0;
		}
		randomcounter++;
	}
	return 0;
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
	response |= ILI9341_CS_enable(instance);
	response |= ILI9341_addToTxBuffer(instance, command);
	response |= ILI9341_CS_disable(instance);
	 return response;
}

//for data, D/C is set high (voltage wise)
bool ILI9341_addDataToBuffer(ili9341_instance *instance, uint16_t command){ //8-bit command: (MSB) D7 D6 D5 D4 D3 D2 D1 D0 (LSB)
	command = ((command & 0xFF) | (1<<ILI9341_DC_BIT_IN_BUFFER));
	bool response = 0;
	response |= ILI9341_CS_enable(instance);
	response |= ILI9341_addToTxBuffer(instance, command);
	response |= ILI9341_CS_disable(instance);
	return response;
}

//bool ILI9341_addDataToBuffer_noCS(ili9341_instance *instance, uint16_t command){ //8-bit command: (MSB) D7 D6 D5 D4 D3 D2 D1 D0 (LSB)
//	command = ((command & 0xFF) | (1<<ILI9341_DC_BIT_IN_BUFFER));
//	bool response = 0;
//	response |= ILI9341_addToTxBuffer(instance, command);
//	return response;
//}

/*void ILI9341_DC_enable(){ //this sets it high, voltage wise
	GPIOSetValue(1, 31, 1);
	return;
}

void ILI9341_DC_disable(){ //this sets it low, voltage wise
	GPIOSetValue(1, 31, 0);
	return;
}

void ILI9341_RST_enable(){
	GPIOSetValue(1, 28, 0);
	return;
}

void ILI9341_RST_disable(){
	GPIOSetValue(1, 28, 1);
	return;
}

void ILI9341_BL_enable(){
	GPIOSetValue(1, 27, 0);
	return;
}

void ILI9341_BL_disable(){
	GPIOSetValue(1, 27, 1);
	return;
}

void ILI9341_SPI_send(int data){
	l11uxx_spi_sendByte(1, data);
	return;
}

void ILI9341_GPIO_init(){
	GPIOSetDir(1, 29, 1); //CS
	GPIOSetDir(1, 31, 1); //DC
	GPIOSetDir(1, 28, 1); //RST
	GPIOSetDir(1, 27, 1); //BL
	ILI9341_CS_disable();
	ILI9341_DC_disable();
	ILI9341_RST_disable();
	ILI9341_BL_enable();

	return;
}



void ILI9341_addInstructionToBuffer(instance,ili9341_instance *instance, char c) {
	ILI9341_DC_disable(); //dc disable sets low voltage wise, meaning instruction
  //digitalWrite(_dc, LOW);
  //CLEAR_BIT(clkport, clkpinmask);
  //digitalWrite(_sclk, LOW);
  ILI9341_CS_enable(instance);
  //digitalWrite(_cs, LOW);

  //spiwrite(c);
  ILI9341_SPI_send(c);

  ILI9341_CS_disable(instance);
  //digitalWrite(_cs, HIGH);







}




*/
bool ILI9341_begin(ili9341_instance *instance) {
//  pinMode(_rst, OUTPUT);
//  digitalWrite(_rst, LOW);
//  pinMode(_dc, OUTPUT);
//  pinMode(_cs, OUTPUT);
//#ifdef __AVR__
//  csport    = portOutputRegister(digitalPinToPort(_cs));
//  dcport    = portOutputRegister(digitalPinToPort(_dc));
//#endif
//#if defined(__SAM3X8E__)
//  csport    = digitalPinToPort(_cs);
//  dcport    = digitalPinToPort(_dc);
//#endif
//#if defined(__arm__) && defined(CORE_TEENSY)
//  mosiport = &_mosi;
//  clkport = &_sclk;
//  rsport = &_rst;
//  csport    = &_cs;
//  dcport    = &_dc;
//#endif
//  cspinmask = digitalPinToBitMask(_cs);
//  dcpinmask = digitalPinToBitMask(_dc);
//
//  if(hwSPI) { // Using hardware SPI
//    SPI.begin();
//#ifdef __AVR__
//    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
//#endif
//#if defined(__SAM3X8E__)
//    SPI.setClockDivider(11); // 85MHz / 11 = 7.6 MHz (full! speed!)
//#endif    SPI.setBitOrder(MSBFIRST);
//    SPI.setBitOrder(MSBFIRST);
//    SPI.setDataMode(SPI_MODE0);
//  } else {
//    pinMode(_sclk, OUTPUT);
//    pinMode(_mosi, OUTPUT);
//    pinMode(_miso, INPUT);
//#ifdef __AVR__
//    clkport     = portOutputRegister(digitalPinToPort(_sclk));
//    mosiport    = portOutputRegister(digitalPinToPort(_mosi));
//#endif
//#if defined(__SAM3X8E__)
//    clkport     = digitalPinToPort(_sclk);
//    mosiport    = digitalPinToPort(_mosi);
//#endif
//    clkpinmask  = digitalPinToBitMask(_sclk);
//    mosipinmask = digitalPinToBitMask(_mosi);
//    CLEAR_BIT(clkport, clkpinmask);
//    CLEAR_BIT(mosiport, mosipinmask);
//  }

  // toggle RST low to reset


	GPIOSetValue(1, 28, 0); //do reset just in case
	delay(300);
	GPIOSetValue(1, 28, 1); //wake from reset
	delay(300);

	//ILI9341_RST_disable();
	//delay(5);
	//ILI9341_RST_enable();
	//delay(20);
	//ILI9341_RST_disable();
	//delay(150);

	//ILI9341_CS_enable(instance);

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

	//ILI9341_addInstructionToBuffer(instance,ILI9341_PWCTR1);    //Power control
	ILI9341_addInstructionToBuffer(instance,0xC0);    //Power control
	ILI9341_addDataToBuffer(instance,0x23);   //VRH[5:0]

	//ILI9341_addInstructionToBuffer(instance,ILI9341_PWCTR2);    //Power control
	ILI9341_addInstructionToBuffer(instance,0xC1);    //Power control
	ILI9341_addDataToBuffer(instance,0x10);   //SAP[2:0];BT[3:0]

	//ILI9341_addInstructionToBuffer(instance,ILI9341_VMCTR1);    //VCM control
	ILI9341_addInstructionToBuffer(instance,0xC5);    //VCM control
	ILI9341_addDataToBuffer(instance,0x3e); //�Աȶȵ���
	ILI9341_addDataToBuffer(instance,0x28);

	//ILI9341_addInstructionToBuffer(instance,ILI9341_VMCTR2);    //VCM control2
	ILI9341_addInstructionToBuffer(instance,0xC7);    //VCM control2
	ILI9341_addDataToBuffer(instance,0x86);  //--

	//ILI9341_addInstructionToBuffer(instance,ILI9341_MADCTL);    // Memory Access Control
	ILI9341_addInstructionToBuffer(instance,0x36);    // Memory Access Control
	//ILI9341_addDataToBuffer(instance,ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR);
	ILI9341_addDataToBuffer(instance,0x40 | 0x08);

	//ILI9341_addInstructionToBuffer(instance,ILI9341_PIXFMT);
	ILI9341_addInstructionToBuffer(instance,0x3A);
	ILI9341_addDataToBuffer(instance,0x55);

	//ILI9341_addInstructionToBuffer(instance,ILI9341_FRMCTR1);
	ILI9341_addInstructionToBuffer(instance,0xB1);
	ILI9341_addDataToBuffer(instance,0x00);
	ILI9341_addDataToBuffer(instance,0x18);

	//ILI9341_addInstructionToBuffer(instance,ILI9341_DFUNCTR);    // Display Function Control
	ILI9341_addInstructionToBuffer(instance,0xB6);    // Display Function Control
	ILI9341_addDataToBuffer(instance,0x08);
	ILI9341_addDataToBuffer(instance,0x82);
	ILI9341_addDataToBuffer(instance,0x27);

	ILI9341_addInstructionToBuffer(instance,0xF2);    // 3Gamma Function Disable
	ILI9341_addDataToBuffer(instance,0x00);

	//ILI9341_addInstructionToBuffer(instance,ILI9341_GAMMASET);    //Gamma curve selected
	ILI9341_addInstructionToBuffer(instance,0x26);    //Gamma curve selected
	ILI9341_addDataToBuffer(instance,0x01);

	//ILI9341_addInstructionToBuffer(instance,ILI9341_GMCTRP1);    //Set Gamma
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

	//ILI9341_addInstructionToBuffer(instance,ILI9341_GMCTRN1);    //Set Gamma
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

	//  ILI9341_addInstructionToBuffer(instance,ILI9341_SLPOUT);    //Exit Sleep
	ILI9341_addInstructionToBuffer(instance,0x11);    //Exit Sleep
	delay(120);
	//ILI9341_addInstructionToBuffer(instance,ILI9341_DISPON);    //Display on
	ILI9341_addInstructionToBuffer(instance,0x29);    //Display on

	//ILI9341_CS_disable(instance);

	return 0;

}


void ILI9341_setAddrWindow(ili9341_instance *instance, int x0, int y0, int x1, int y1) { //ints used here were 16bit


	//ILI9341_CS_enable(instance);

  //ILI9341_addInstructionToBuffer(instance,ILI9341_CASET); // Column addr set
	ILI9341_addInstructionToBuffer(instance,0x2A); // Column addr set
  ILI9341_addDataToBuffer(instance,x0 >> 8);
  ILI9341_addDataToBuffer(instance,x0 & 0xFF);     // XSTART
  ILI9341_addDataToBuffer(instance,x1 >> 8);
  ILI9341_addDataToBuffer(instance,x1 & 0xFF);     // XEND

  //ILI9341_addInstructionToBuffer(instance,ILI9341_PASET); // Row addr set
  ILI9341_addInstructionToBuffer(instance,0x2B); // Row addr set
  ILI9341_addDataToBuffer(instance,y0>>8);
  ILI9341_addDataToBuffer(instance,y0);     // YSTART
  ILI9341_addDataToBuffer(instance,y1>>8);
  ILI9341_addDataToBuffer(instance,y1);     // YEND

  //ILI9341_addInstructionToBuffer(instance,ILI9341_RAMWR); // write to RAM
  ILI9341_addInstructionToBuffer(instance,0x2C); // write to RAM

  //ILI9341_CS_disable(instance);
}


//void Adafruit_ILI9341::pushColor(uint16_t color) {
//  //digitalWrite(_dc, HIGH);
//  SET_BIT(dcport, dcpinmask);
//  //digitalWrite(_cs, LOW);
//  CLEAR_BIT(csport, cspinmask);
//
//  spiwrite(color >> 8);
//  spiwrite(color);
//
//  SET_BIT(csport, cspinmask);
//  //digitalWrite(_cs, HIGH);
//}

void ILI9341_drawPixel(ili9341_instance *instance, int x, int y, int color) { //ints used here were 16bit

	//ILI9341_CS_enable(instance);


  if((x < 0) ||(x >= ILI9341_LCD_WIDTH) || (y < 0) || (y >= ILI9341_LCD_HEIGHT)) return; //todo: get values from instance

  ILI9341_setAddrWindow(instance, x,y,x+1,y+1);

  //digitalWrite(_dc, HIGH);
  //ILI9341_DC_enable();
  //digitalWrite(_cs, LOW);

  ILI9341_CS_enable(instance);
  ILI9341_addDataToBuffer(instance, color >> 8); //l11uxx_spi_sendByte(1, color >> 8); //spiwrite(color >> 8);
  ILI9341_addDataToBuffer(instance, color); //l11uxx_spi_sendByte(1, color); //spiwrite(color);
  ILI9341_CS_disable(instance);
  //digitalWrite(_cs, HIGH);

  //ILI9341_CS_disable(instance);
}
/*











void ILI9341_drawFastVLine(ili9341_instance *instance, int x, int y, int h, int color) { //ints used here were 16bit

  // Rudimentary clipping
  if((x >= ILI9341_LCD_WIDTH) || (y >= ILI9341_LCD_HEIGHT)) return; //todo: get values from instance

  if((y+h-1) >= ILI9341_LCD_HEIGHT)
    h = ILI9341_LCD_WIDTH-y;

  ILI9341_setAddrWindow(instance, x, y, x, y+h-1);

  char hi = color >> 8, lo = color;

  ILI9341_DC_enable(instance);
  //digitalWrite(_dc, HIGH);
  ILI9341_CS_enable(instance);
  //digitalWrite(_cs, LOW);

  while (h--) {
	  ILI9341_SPI_send(hi); //spiwrite(hi);
	  ILI9341_SPI_send(lo); //spiwrite(lo);
  }
  ILI9341_CS_disable(instance);
  //digitalWrite(_cs, HIGH);
}


void ILI9341_drawFastHLine(ili9341_instance *instance, int x, int y, int w, int color) { //ints used here were 16bit

  // Rudimentary clipping
  if((x >= ILI9341_LCD_WIDTH) || (y >= ILI9341_LCD_HEIGHT)) return;
  if((x+w-1) >= ILI9341_LCD_WIDTH)  w = ILI9341_LCD_WIDTH-x;
  ILI9341_setAddrWindow(instance, x, y, x+w-1, y);

  char hi = color >> 8, lo = color;
  ILI9341_DC_enable(instance);
  ILI9341_CS_enable(instance);
  //digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, LOW);
  while (w--) {
	  ILI9341_SPI_send(hi); //spiwrite(hi);
	  ILI9341_SPI_send(lo); //spiwrite(lo);
  }
  ILI9341_CS_disable(instance);
  //digitalWrite(_cs, HIGH);
}



// fill a rectangle
void ILI9341_fillRect(ili9341_instance *instance, int x, int y, int w, int h, int color) { //ints used here were 16bit

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= ILI9341_LCD_WIDTH) || (y >= ILI9341_LCD_HEIGHT)) return;
  if((x + w - 1) >= ILI9341_LCD_WIDTH)  w = ILI9341_LCD_WIDTH  - x;
  if((y + h - 1) >= ILI9341_LCD_HEIGHT) h = ILI9341_LCD_HEIGHT - y;

  ILI9341_setAddrWindow(x, y, x+w-1, y+h-1);

  char hi = color >> 8, lo = color;

  ILI9341_DC_enable();
  //digitalWrite(_dc, HIGH);
  ILI9341_CS_enable();
  //digitalWrite(_cs, LOW);

  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
    	 ILI9341_SPI_send(hi); //spiwrite(hi);
    	 ILI9341_SPI_send(lo); //spiwrite(lo);
    }
  }
  //digitalWrite(_cs, HIGH);
  ILI9341_CS_disable();
}

void ILI9341_fillScreen(ili9341_instance *instance, int color) { //ints used here were 16bit
	ILI9341_fillRect(0, 0,  ILI9341_LCD_WIDTH, ILI9341_LCD_HEIGHT, color);
}



// Pass 8-bit (each) R,G,B, get back 16-bit packed color
int ILI9341_Color565(ili9341_instance *instance, char r, char g, char b) { //ints used here were 16bit
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}*/






//void drawPokey(unsigned int x, unsigned int y){
/*
THIS CODE WILL DRAW POKEY THE PENGUIN ON YOUR SCREEN!!!
http://yellow5.com/pokey FOR MORE INFO!!!

MAYBE IT WORKS NOW!!!!!

*/
/*
const unsigned int yellow = 0xFF00;//colormix(255, 255, 0);
//if doesn't work,try other color
//looked it up in a MSX manual,straight outta 1987!!!
const unsigned int black = 0x00;
const unsigned int white = 65535;
void drawPokey(ili9341_instance *instance, unsigned int x, unsigned int y)
{
	ILI9341_fillRect(x, y, 36 ,50 , white); //make area white i hope
  //let's draw Pokey now!!!
  ILI9341_drawFastVLine(x, y+15, 8, black);
  ILI9341_drawFastVLine(x+1, y+13, 17, black);
  ILI9341_drawFastVLine(x+2, y+11, 24, black);
  ILI9341_drawFastVLine(x+3, y+9, 34, black);
  ILI9341_drawFastVLine(x+4, y+9, 35, black);
  ILI9341_drawFastVLine(x+5, y+8, 38, black);
  ILI9341_drawFastVLine(x+6, y+7, 39, black);
  ILI9341_drawFastVLine(x+7, y+7, 39, black);
  ILI9341_drawFastVLine(x+8, y+6, 41, black);
  ILI9341_drawFastVLine(x+9, y+5, 43, black);
  ILI9341_drawFastVLine(x+10, y+3, 45, black);
  ILI9341_drawFastVLine(x+11, y+2, 45, black); //black back part ends,time to fuck around with whites and shit
  ILI9341_drawFastVLine(x+12, y+2, 45, black); //black stripe
  ILI9341_drawFastVLine(x+12, y+25, 3, white); //3 pixels white near wing
  ILI9341_drawFastVLine(x+13, y+1, 46, black);
  ILI9341_drawFastVLine(x+13, y+29, 8, white);
  ILI9341_drawFastVLine(x+14, y+1, 47, black);
  ILI9341_drawFastVLine(x+14, y+32, 9, white);
  ILI9341_drawFastVLine(x+15, y, 48, black);
  ILI9341_drawFastVLine(x+15, y+33, 8, white);
  ILI9341_drawFastVLine(x+16, y, 48, black);
  ILI9341_drawFastVLine(x+16, y+33, 9, white);
  ILI9341_drawFastVLine(x+17, y, 48 , black);
  ILI9341_drawFastVLine(x+17, y+27, 16, white);
  ILI9341_drawFastVLine(x+17, y+43, 2, white);
  ILI9341_drawFastVLine(x+18, y, 48, black);
  ILI9341_drawFastVLine(x+18, y+19, 25, white);
  ILI9341_drawFastVLine(x+18, y+44, 1, white);
  ILI9341_drawFastVLine(x+19, y, 48, black);
  ILI9341_drawFastVLine(x+19, y+7, 3, white); //1st part of eye
  ILI9341_drawFastVLine(x+19, y+18, 27, white);
  ILI9341_drawFastVLine(x+20, y, 48, black);
  ILI9341_drawFastVLine(x+20, y+7, 4, white); //2nd part of eye
  ILI9341_drawFastVLine(x+20, y+17, 29, white);
  ILI9341_drawFastVLine(x+21, y, 48, black);
  ILI9341_drawFastVLine(x+21, y+14, 31, white);
  ILI9341_drawFastVLine(x+22, y, 49, black);
  ILI9341_drawFastVLine(x+22, y+13, 32, white);
  ILI9341_drawFastVLine(x+23, y, 50, black);
  ILI9341_drawFastVLine(x+23, y+9, 1, white);
  ILI9341_drawFastVLine(x+23, y+12, 33, white);
  ILI9341_drawFastVLine(x+24, y, 9, black);
  ILI9341_drawFastVLine(x+24, y+43, 2, black);
  ILI9341_drawFastVLine(x+24, y+46, 2, black);
  ILI9341_drawFastVLine(x+25, y+1, 8, black);
  ILI9341_drawFastVLine(x+25, y+11, 7, black);
  ILI9341_drawFastVLine(x+25, y+43, 1, black);
  ILI9341_drawFastVLine(x+25, y+46, 3, black);
  ILI9341_drawFastVLine(x+26, y+2, 8, black);
  ILI9341_drawFastVLine(x+26, y+42, 1, black);
  ILI9341_drawFastVLine(x+26, y+46, 2, black);
  ILI9341_drawFastVLine(x+27, y+4, 6, black);
  ILI9341_drawFastVLine(x+27, y+41, 1, black);
  ILI9341_drawFastVLine(x+27, y+46, 2, black);
  ILI9341_drawFastVLine(x+28,y+6, 3, black);
  ILI9341_drawFastVLine(x+28,y+41, 2, black);
  ILI9341_drawFastVLine(x+29, y+8, 2, black);
  ILI9341_drawFastVLine(x+29, y+16, 3, black);
  ILI9341_drawFastVLine(x+29, y+40, 1, black);
  ILI9341_drawFastVLine(x+30, y+9, 3, black);
  ILI9341_drawFastVLine(x+30, y+19, 2, black);
  ILI9341_drawFastVLine(x+30, y+36, 4, black);
  ILI9341_drawFastVLine(x+31, y+21, 14, black);
  //let's draw a beak!!!
  ILI9341_drawFastHLine(x+27, y+11, 3, black);
  ILI9341_drawFastHLine(x+26, y+12, 6, yellow);
  ILI9341_drawFastHLine(x+32, y+12, 2, black);
  ILI9341_drawFastHLine(x+26, y+13, 8, yellow);
  ILI9341_drawFastHLine(x+26, y+14, 10, black);
  ILI9341_drawFastHLine(x+26, y+15, 8, yellow);
  ILI9341_drawFastHLine(x+26, y+16, 3, yellow);
  ILI9341_drawFastHLine(x+26, y+17, 3, black);
  ILI9341_drawFastHLine(x+30, y+16, 4, black);
  ILI9341_drawFastVLine(x+34, y+13, 3, black);
  //pokey done!!!
}
*/

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

/*
void ILI9341_printString_bg(ili9341_instance *instance, int x, int y, int textColour, int bgColour, char text[100]){ //
	//Y coordinate is top left corner of text
	//40 chars * 40 chars (sic!) max
	int characterNumber=0;
	int characterColumn=0;
	int currentRow=0;
	int lineLengthText=0;
	int lineLengthBg=0;

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
				ILI9341_drawFastVLine(x+characterColumn+6*characterNumber, y+currentRow-lineLengthText, lineLengthText, textColour);
				while(!((ILI9341_LCDBitmapFont[((text[characterNumber]*5)+characterColumn)]) & (1<<currentRow))){
					//"light" pixel
					if(currentRow>6) break;
					lineLengthBg++;
					currentRow++;
				}

				ILI9341_drawFastVLine(x+characterColumn+6*characterNumber, y+currentRow-lineLengthBg, lineLengthBg, bgColour);
				lineLengthText=0;
				lineLengthBg=0;
			}
			if((x+6*characterNumber >= ILI9341_LCD_WIDTH) || (y >= ILI9341_LCD_HEIGHT)) return;
			characterColumn++;
			currentRow=0;
		}

		//do bgcolor line between characters
		//but not for last character
		if(text[characterNumber+1] != 0) ILI9341_drawFastVLine(x+6*characterNumber+5, y, 7, bgColour);

		//do a bgcolor line under char
		//ILI9341_drawFastHLine(x+6*characterNumber, y+8, 6, bgColour);


		characterColumn=0;
		characterNumber++;
	}
	return;
}
*/

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




















//rudimentary crap here:

/*

void ILI9341_addDataToBuffer(instance,ili9341_instance *instance, char c) {
	ILI9341_DC_enable(); //dc disable sets high voltage wise, meaning data
  //digitalWrite(_dc, HIGH);
  //CLEAR_BIT(clkport, clkpinmask);
  //digitalWrite(_sclk, LOW);
  ILI9341_CS_enable();
  //digitalWrite(_cs, LOW);

  //spiwrite(c);
  ILI9341_SPI_send(c);

  //digitalWrite(_cs, HIGH);
  ILI9341_CS_disable();
}*/

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
//#define DELAY 0x80

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
//void Adafruit_ILI9340::commandList(uint8_t *addr) {
//
//  uint8_t  numCommands, numArgs;
//  uint16_t ms;
//
//  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
//  while(numCommands--) {                 // For each command...
//    writecommand(pgm_read_byte(addr++)); //   Read, issue command
//    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
//    ms       = numArgs & DELAY;          //   If hibit set, delay follows args
//    numArgs &= ~DELAY;                   //   Mask out delay bit
//    while(numArgs--) {                   //   For each argument...
//      writedata(pgm_read_byte(addr++));  //     Read, issue argument
//    }
//
//    if(ms) {
//      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
//      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
//      delay(ms);
//    }
//  }
//}



/*

void Adafruit_ILI9340::setRotation(uint8_t m) {

  writecommand(ILI9340_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     writedata(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
     _width  = ILI9340_TFTWIDTH;
     _height = ILI9340_TFTHEIGHT;
     break;
   case 1:
     writedata(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
     _width  = ILI9340_TFTHEIGHT;
     _height = ILI9340_TFTWIDTH;
     break;
  case 2:
    writedata(ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR);
     _width  = ILI9340_TFTWIDTH;
     _height = ILI9340_TFTHEIGHT;
    break;
   case 3:
     writedata(ILI9340_MADCTL_MV | ILI9340_MADCTL_MY | ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
     _width  = ILI9340_TFTHEIGHT;
     _height = ILI9340_TFTWIDTH;
     break;
  }
}


void Adafruit_ILI9340::invertDisplay(boolean i) {
  writecommand(i ? ILI9340_INVON : ILI9340_INVOFF);
}*/
