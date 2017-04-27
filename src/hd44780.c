/*
 * hd44780.c
 *
 *  Created on: 27 Apr 2017
 *      Author: denry
 */


#include <stdint.h>
#include <stdbool.h>
#include "hd44780.h"

//https://www.sparkfun.com/datasheets/LCD/HD44780.pdf

void lcddelay(){
  int i=0;
  int j=0;
  //for(i=0; i<I2CLCD_DELAY; i++) for(j=0; j<I2CLCD_DELAY; j++)I2C_1602_SDA_READ;
}


/*
void hd44780_lcdshift(int output){ //LED, D4, D5, D6, D7, RS, RW, BL //our address: 4Eh (write address) //this function is recent development and is fucked up
//note et kõik port registrid on inverted
        int flipped_address = 0;
	unsigned int output_translated=0, output_translated_enter=0; //(GP7)RS,E,DB4,DB5,DB6,DB7,LITE (GP1) (GP0 free)
	//if(output&0b10000000)output_translated|=0b; //this is that extra LED, we don't have that here



	if(output&0b01000000)output_translated|=0b00001000; //D4
	if(output&0b00100000)output_translated|=0b00000100; //D5
	if(output&0b00010000)output_translated|=0b00000010; //D6
	if(output&0b00001000)output_translated|=0b00000001; //D7
	if(output&0b00000100)output_translated|=0b10000000; //RS
	if(output&0b00000010)output_translated|=0b01000000; //RW
	if(output&0b00000001)output_translated|=0b00010000; //BL

	//output_translated=output;//FOR DEBUGGERY!!!!!!!

        output_translated_enter=output_translated;
        //output_translated_enter|=0b01000000; //previous faulty shit
		output_translated_enter|=0b00100000;
	//output_translated^=0b11111111; //bcuz output inverted!
    //    output_translated_enter^=0b11111111; //bcuz output inverted!


        if(I2C_1602_ADDRESS&0b10000000)flipped_address|=0b00000001;
        if(I2C_1602_ADDRESS&0b01000000)flipped_address|=0b00000010;
        if(I2C_1602_ADDRESS&0b00100000)flipped_address|=0b00000100;
        if(I2C_1602_ADDRESS&0b00010000)flipped_address|=0b00001000;
        if(I2C_1602_ADDRESS&0b00001000)flipped_address|=0b00010000;
        if(I2C_1602_ADDRESS&0b00000100)flipped_address|=0b00100000;
        if(I2C_1602_ADDRESS&0b00000010)flipped_address|=0b01000000;
        if(I2C_1602_ADDRESS&0b00000001)flipped_address|=0b10000000;

        i2c_start();
	if(i2c_sendbyte(flipped_address)){ //mothafuckin adre
         //if we get nack
           LED_ON;
           delay(10);
           LED_OFF;
           delay(100);
           LED_ON;
           delay(100);
           LED_OFF;
           delay(100);
           LED_ON;
           delay(10);
           LED_OFF;
           delay(100);
        }
        //i2c_sendbyte(output_translated); //command
        i2c_sendbyte(output_translated_enter);
        i2c_sendbyte(output_translated);

	i2c_stop();
	return;
}


void hd44780_lcdsend(int RS, int RW, int eight, int BL, int LED){ //8bitdata D7 D6 D5 D4 D3 D2 D1 D0 //this works fo sho
  int output=0; int i=0;
  for(i=0; i<2; i++){
    if(RS>0) output|=0b00000100;
    if(RW>0) output|=0b00000010;
    if(BL>0) output|=0b00000001;
    if(LED>0) output|=0b10000000;
    if(i==0){
      if(eight&0b10000000) output|=0b00001000;
      if(eight&0b01000000) output|=0b00010000;
      if(eight&0b00100000) output|=0b00100000;
      if(eight&0b00010000) output|=0b01000000;
    }
    else{
      if(eight&0b00001000) output|=0b00001000;
      if(eight&0b00000100) output|=0b00010000;
      if(eight&0b00000010) output|=0b00100000;
      if(eight&0b00000001) output|=0b01000000;
    }
    lcdshift(output);
    output=0;
  }
  return;
}


void hd44780_lcdword(char *text){ //lcdword("Denry");
  int i, len;
  len=strlen(text);
  //lcdsend(1,0,len+48,1,0);
  for(i=0; i<len; i++){
    lcdsend(1,0,text[i], 1, 0);
  }
  return;
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

void hd44780_lcdcursor(int x, int y){
  lcdsend(0,0,0b00000010, 1, 0); //returnhome
  x=x+40*y;
  while (x>0){
    lcdsend(0,0,0b00010100, 1, 0); //increment
    x--;
  }
  return;
}

void hd44780_lcdclear(){
	lcdcursor (0,0);
	lcdword("                ");
	lcdcursor (0,1);
	lcdword("                ");
	return;
}

*/


bool hd44780_addToTxBuffer(hd44780_instance *instance, uint16_t *command){
	if(instance->LCDCommandsInBuffer < instance->LCDCommandBufferSize){
		instance->LCDCommandBuffer[instance->LCDCommandBufferIndex] = command;
		instance->LCDCommandBufferIndex++;
		instance->LCDCommandsInBuffer++;
		if(instance->LCDCommandBufferIndex >= instance->LCDCommandBufferSize) instance->LCDCommandBufferIndex = 0; //go circular if necessary
	}
	else return 1; //catastrophe (can't fit, buffer full)
	return 0; //everything success

}

//bool hd44780_getFromTxBuffer(hd44780_instance *instance, uint8_t *LSN, uint8_t *MSN, bool *RS, bool *RW){
bool hd44780_getFromTxBuffer(hd44780_instance *instance, uint8_t *MSN, bool *RS, bool *RW, bool *E){
	int firstUnsentPacket = instance->LCDCommandBufferIndex - instance->LCDCommandsInBuffer;
	if (firstUnsentPacket < 0) firstUnsentPacket += instance->LCDCommandBufferSize;
	if(instance->LCDCommandsInBuffer){
		//LSN = (uint8_t*)(((instance->LCDCommandBuffer[instance->LCDCommandBufferIndex])     ) & 0x0F);
		//MSN = (uint8_t*)(((instance->LCDCommandBuffer[instance->LCDCommandBufferIndex]) >> 4) & 0x0F);
		//RS  =    (bool*)(((instance->LCDCommandBuffer[instance->LCDCommandBufferIndex]) >> 8) & 0x01);
		//RW  =    (bool*)(((instance->LCDCommandBuffer[instance->LCDCommandBufferIndex]) >> 9) & 0x01);

		*MSN = (uint8_t)(((instance->LCDCommandBuffer[firstUnsentPacket])                  ) & 0x0F);
		*RS  =    (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> HD44780_RS_BIT) & 0x01);
		*RW  =    (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> HD44780_RW_BIT) & 0x01);
		*E   =    (bool)(((instance->LCDCommandBuffer[firstUnsentPacket]) >> HD44780_E_BIT ) & 0x01);

		if(instance->LCDCommandBufferIndex == 0) instance->LCDCommandBufferIndex = instance->LCDCommandBufferSize; //go circular if necessary
		//instance->LCDCommandBufferIndex--;
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

bool hd44780_init(hd44780_instance *instance, uint8_t columns, uint8_t *rows, bool *fiveTimesTen){
//  lcdshift(0b00000000);
//lcdshift(0b); //LED, D4, D5, D6, D7, RS, RW, BL
//  lcdshift(0b00100001);//set 4bit init

	instance->LCDCommandBufferIndex = 0;
	instance->LCDCommandsInBuffer = 0;
	instance->LCDCommandBufferSize = HD44780_TX_BUFFER_SIZE;

  //set 4-bit operation, going slightly deeper
  bool response = 0;
  response |= hd44780_addToTxBuffer(instance, 0b0010); //only DB5 high
  response |= hd44780_addToTxBuffer(instance, 0b0010 | (1<<HD44780_E_BIT));
  response |= hd44780_addToTxBuffer(instance, 0b0010);

  if(columns == 1) 	columns = 0;	//one line
  else 				columns = 1;	//two lines
  instance->twoline = columns;

  response |= hd44780_addInstructionToBuffer(instance, (1<<5)|(((uint8_t)(columns))<<3)|(((uint8_t)(fiveTimesTen))<<2)); //set 4-bit again, linecount, 5x8 or 5x10
  response |= hd44780_addInstructionToBuffer(instance, (1<<3)|(1<<2)|(1<<1)|(1<<0)); //enable display, enable cursor, enable cursor blinking
  response |= hd44780_addInstructionToBuffer(instance, (1<<2)|(1<<1)); //entry mode, increment cursor each write
  response |= hd44780_addInstructionToBuffer(instance, (1<<1)); //return cursor home
  response |= hd44780_addDataToBuffer(instance, 0b01001000); //print H (lol)
  response |= hd44780_addDataToBuffer(instance, 0b01001001); //print I (lol)

  return response;

  lcdclear(); //this is slow as fuck for some reason
  printf("LCDclear done \n");
  lcdcursor(0, 0);
  printf("Cursor set 0,0 \n");
  return response;
}
