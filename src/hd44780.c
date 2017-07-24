/*
 * hd44780.c
 *
 *  Created on: 27 Apr 2017
 *      Author: denry
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "hd44780.h"

//https://www.sparkfun.com/datasheets/LCD/HD44780.pdf


/*
void lcddelay(){
  int i=0;
  int j=0;
  //for(i=0; i<I2CLCD_DELAY; i++) for(j=0; j<I2CLCD_DELAY; j++)I2C_1602_SDA_READ;
}



void lcdnr(int nr, int spaces){
  int x=0;
  while(nr>9999){
    nr-=10000;
    x++;
  }
  if(spaces>4)lcdsend(1,0,x+48,1,0);
  x=0;
  while(nr>999){
    nr-=1000;
    x++;
  }
  if(spaces>3)lcdsend(1,0,x+48,1,0);
  x=0;
  while(nr>99){
    nr-=100;
    x++;
  }
  if(spaces>2)lcdsend(1,0,x+48,1,0);
  x=0;
  while(nr>9){
    nr-=10;
    x++;
  }
  if(spaces>1)lcdsend(1,0,x+48,1,0);
  lcdsend(1,0,nr+48,1,0);
  return;
}


*/


bool hd44780_addToTxBuffer(hd44780_instance *instance, uint16_t *command){
	if(instance->LCDCommandsInBuffer < (instance->LCDCommandBufferSize - 2)){
		instance->LCDCommandBuffer[instance->LCDCommandBufferIndex] = command;
		instance->LCDCommandBufferIndex++;
		instance->LCDCommandsInBuffer++;
		if(instance->LCDCommandBufferIndex >= instance->LCDCommandBufferSize)
			instance->LCDCommandBufferIndex = 0; //go circular if necessary
		//bitbangUARTmessage("x");
		//while((*instance).handlerFunction(instance) == 0); //empty buffer a bit then
	}
	//else return 1; //catastrophe (can't fit, buffer full)
	else {
		//bitbangUARTmessage("LCD BUFFER FULL!\r\n");
		while((*instance).handlerFunction(instance) == 0); //empty buffer a bit then
		return hd44780_addToTxBuffer(instance, &command);
		//return 0; //and return success

	}
	return 0; //everything success

}


bool hd44780_getFromTxBuffer(hd44780_instance *instance, uint8_t *MSN, bool *RS, bool *RW, bool *E){
	uint32_t firstUnsentPacket;
	if ((instance->LCDCommandBufferIndex) < (instance->LCDCommandsInBuffer))  firstUnsentPacket = (instance->LCDCommandBufferIndex) + instance->LCDCommandBufferSize;
	else firstUnsentPacket = (instance->LCDCommandBufferIndex);
	firstUnsentPacket -= (instance->LCDCommandsInBuffer);
	//int firstUnsentPacket = (instance->LCDCommandBufferIndex) - (instance->LCDCommandsInBuffer);
	//if (firstUnsentPacket < 0) firstUnsentPacket += instance->LCDCommandBufferSize;
	if(instance->LCDCommandsInBuffer){
		*MSN = (uint8_t)(((instance->LCDCommandBuffer[firstUnsentPacket])                  ) & 0x0F);
		*RS  =    (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> HD44780_RS_BIT) & 0x01);
		*RW  =    (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> HD44780_RW_BIT) & 0x01);
		*E   =    (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> HD44780_E_BIT ) & 0x01);

		instance->LCDCommandsInBuffer--;
	}
	else return 1; //catastrophe (nothing to read out)

	return 0; //everything success
}

//this function assumes R/W to be 0 (write) and RS to be 0 (instruction register)
bool hd44780_addInstructionToBuffer(hd44780_instance *instance, uint16_t command){ //8-bit command: (MSB) D7 D6 D5 D4 D3 D2 D1 D0 (LSB)
	uint8_t commandMSB = (command >> 4) & 0x0F;
	uint8_t commandLSB = (command     ) & 0x0F;
	bool response = 0;
	response |= hd44780_addToTxBuffer(instance, 0); //to clear that fcking E bit
	response |= hd44780_addToTxBuffer(instance, commandMSB);
	response |= hd44780_addToTxBuffer(instance, commandMSB | (1<<HD44780_E_BIT));
	response |= hd44780_addToTxBuffer(instance, commandMSB);
	response |= hd44780_addToTxBuffer(instance, 0); //to clear that fcking E bit
	response |= hd44780_addToTxBuffer(instance, commandLSB);
	response |= hd44780_addToTxBuffer(instance, commandLSB | (1<<HD44780_E_BIT));
	response |= hd44780_addToTxBuffer(instance, commandLSB);
	 return response;
	//return 0; //everything success
	//return 1; //catastrophe
}

//this function assumes R/W to be 0 (write) and RS to be 1 (data register)
bool hd44780_addDataToBuffer(hd44780_instance *instance, uint16_t command){ //8-bit command: (MSB) D7 D6 D5 D4 D3 D2 D1 D0 (LSB)
	uint8_t commandMSB = ((command >> 4) & 0x0F) | (1<<HD44780_RS_BIT);
	uint8_t commandLSB = ((command     ) & 0x0F) | (1<<HD44780_RS_BIT);
	bool response = 0;
	response |= hd44780_addToTxBuffer(instance, commandMSB);
	response |= hd44780_addToTxBuffer(instance, commandMSB | (1<<HD44780_E_BIT));
	response |= hd44780_addToTxBuffer(instance, commandMSB);
	response |= hd44780_addToTxBuffer(instance, commandLSB);
	response |= hd44780_addToTxBuffer(instance, commandLSB | (1<<HD44780_E_BIT));
	response |= hd44780_addToTxBuffer(instance, commandLSB);

	return response;
	//return 0; //everything success
	//return 1; //catastrophe
}

bool hd44780_incrementcursor(hd44780_instance *instance, bool directionToRight){
	bool response = 0;
	hd44780_addInstructionToBuffer(instance, (0b00010000 | (directionToRight << 2))); //increment
	while((*instance).handlerFunction(instance) == 0); //empty buffer a bit
	return response;
}

bool hd44780_lcdcursor(hd44780_instance *instance, uint8_t x, uint8_t y){
	bool response = 0;
	response |= hd44780_addInstructionToBuffer(instance, (1<<1)); //return cursor home
	x=x+40*y;
	while (x>0){
		hd44780_addInstructionToBuffer(instance, 0b00010100); //increment
		x--;
	}
	while((*instance).handlerFunction(instance) == 0); //empty buffer a bit
	return response;
}

bool hd44780_printtext(hd44780_instance *instance, char *text){ //lcdword("Denry");
  int i, len;
  bool response = 0;
  len=strlen(text);
  //bitbangUARTmessage("LCD char:");
  for(i=0; i<len; i++){
	  response |= hd44780_addDataToBuffer(instance, text[i]);
	  //this appears to send characters OK, still think it is LCD fault.

	  //bitbangUARTbin(text[i], 0, 8);
	  //bitbangUARTmessage(";");
  }
  while((*instance).handlerFunction(instance) == 0); //empty buffer a bit
  return response;
}

bool hd44780_clear(hd44780_instance *instance){
	bool response = 0;
	response |= hd44780_addInstructionToBuffer(instance, 0x01); //clear display
	while((*instance).handlerFunction(instance) == 0); //empty buffer a bit
	return response;
}

bool hd44780_disp_cursor(hd44780_instance *instance, bool displayState, bool cursorVisible, bool cursorBlink){
	bool response = 0;
	response |= hd44780_addInstructionToBuffer(instance, (1<<3)|(displayState<<2)|(cursorVisible<<1)|(cursorBlink<<0));
	while((*instance).handlerFunction(instance) == 0); //empty buffer a bit
	return response;
}

bool hd44780_init(hd44780_instance *instance, uint8_t columns, uint8_t *rows, bool *fiveTimesTen){
	instance->LCDCommandBufferIndex = 0;
	instance->LCDCommandsInBuffer = 0;
	instance->LCDCommandBufferSize = HD44780_TX_BUFFER_SIZE;

  bool response = 0;


  //note that reset is performed by setting it to 8-bit mode (cause it needs single 4-bit command afterwards, ruining sync)
  //this command is sent three times, because it occasionally failed to work when sending once. Two might work too
  //response |= hd44780_addInstructionToBuffer(instance, (1<<5)|(1<<4)|(((uint8_t)(columns))<<3)|(((uint8_t)(fiveTimesTen))<<2)); //set 8-bit mode as reset, linecount, 5x8 or 5x10
  //response |= hd44780_addInstructionToBuffer(instance, (1<<5)|(1<<4)|(((uint8_t)(columns))<<3)|(((uint8_t)(fiveTimesTen))<<2)); //set 8-bit mode as reset, linecount, 5x8 or 5x10
  //response |= hd44780_addInstructionToBuffer(instance, (1<<5)|(1<<4)|(((uint8_t)(columns))<<3)|(((uint8_t)(fiveTimesTen))<<2)); //set 8-bit mode as reset, linecount, 5x8 or 5x10

/*
  //and repeat to avoid single nibble
  response |= hd44780_addToTxBuffer(instance, 0b0000); //all low now
  response |= hd44780_addToTxBuffer(instance, 0b0000 | (1<<HD44780_E_BIT));
  response |= hd44780_addToTxBuffer(instance, 0b0000);
*/


  //to "reset" the LCD we request 8-bit mode and we assume we are in 4-bit mode.
  response |= hd44780_addToTxBuffer(instance, 0b0011);
  response |= hd44780_addToTxBuffer(instance, 0b0011 | (1<<HD44780_E_BIT));
  response |= hd44780_addToTxBuffer(instance, 0b0011);
	//and we send same thing again, cause 4-bit mode doesn't care about lower nibble and 8-bit wants what we sent just now
		 response |= hd44780_addToTxBuffer(instance, 0b0011);
		  response |= hd44780_addToTxBuffer(instance, 0b0011 | (1<<HD44780_E_BIT));
		  response |= hd44780_addToTxBuffer(instance, 0b0011);


  //it is essential to start with the odd command without a mating 4-bit command, to enable the 4-bit mode.
  response |= hd44780_addToTxBuffer(instance, 0b0010);
  response |= hd44780_addToTxBuffer(instance, 0b0010 | (1<<HD44780_E_BIT));
  response |= hd44780_addToTxBuffer(instance, 0b0010);


  if(columns == 1) 	columns = 0;	//one line
  else 				columns = 1;	//two lines
  instance->twoline = columns;
  instance->I2C_pinE_offset = 2; 	//default is 2, for 4-line LCDs you might need to use 3 as well (with backlight hardware hack)


  response |= hd44780_addInstructionToBuffer(instance, (1<<5)|(((uint8_t)(columns))<<3)|(((uint8_t)(fiveTimesTen))<<2)); //set 4-bit again, linecount, 5x8 or 5x10

  //response |= hd44780_disp_cursor(instance, 1, 1, 1); //do not use this here, it attempts to do send and during init may send to wrong LCD
  response |= hd44780_addInstructionToBuffer(instance, (1<<3)|(1<<2)|(1<<1)|(1<<0));

  response |= hd44780_addInstructionToBuffer(instance, (1<<2)|(1<<1)); //entry mode, increment cursor each write

  //response |= hd44780_clear(instance); //do not use this here, it attempts to do send and during init may send to wrong LCD
  response |= hd44780_addInstructionToBuffer(instance, 0x01); //clear display

  response |= hd44780_addInstructionToBuffer(instance, (1<<1)); //return cursor home

  //response |= hd44780_addDataToBuffer(instance, 0b01001000); //print H
  //response |= hd44780_printtext(instance, "HELLO"); //<- not allowed, cause attempts to write data again!

  //while((*instance).handlerFunction(instance) == 0); //while can send data, send it //NB! this is bad cause might want to use different E pin

  return response;
}
