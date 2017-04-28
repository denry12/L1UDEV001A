/*
 * hd44780.h
 *
 *  Created on: 27 Apr 2017
 *      Author: denry
 */

#ifndef INC_HD44780_H_
#define INC_HD44780_H_

//number of commands that can be prepared
//note that this is used as 8-bit value so keep it below 255 or modify as necessary
#define HD44780_TX_BUFFER_SIZE 250

#define HD44780_RS_BIT	4
#define HD44780_RW_BIT	5
#define HD44780_E_BIT	6

typedef enum{
	HD44780_STATE_IDLE,
	HD44780_STATE_TXNEEDED
}hd44780_state;

typedef enum{
	HD44780_HWTYPE_I2C,
	HD44780_HWTYPE_4BIT
}hd44780_HWtype;

typedef struct {

	//generic LCD variables
	uint8_t LCDCommandBufferSize; 						//hopefully can get away with 8-bit
	uint16_t LCDCommandBuffer[HD44780_TX_BUFFER_SIZE+1];//8 LSB are for data pins, higher are RS, R/W etc
	uint8_t LCDCommandBufferIndex; 						//hopefully can get away with 8-bit
	uint8_t LCDCommandsInBuffer;
	//bool (*lcdToBuffer)(); 							//won't be happening. Cause I don't need it.
	hd44780_state currentstate;
	hd44780_HWtype HWType;
	bool twoline;
	uint8_t columns; 									//characters per line
	//uint8_t lastSentCommand;							//to modify just one bit more easily

	//I2C related variables
	bool (*bufferToLCD_i2c)();
	uint8_t I2C_addr;
	uint8_t I2C_pinE_offset;

	//4-bit related variables
	bool (*bufferToLCD_4bitData)();						//function to set data pins
	bool (*bufferToLCD_4bitControl)();						//function to set various other pins, e.g. E

	bool (*handlerFunction)(); //handler function

} hd44780_instance;


#endif /* INC_HD44780_H_ */
