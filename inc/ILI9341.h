/*
 * ILI9341.h
 *
 *  Created on: 01.05.2017
 *      Author: Denry
 */

#ifndef INC_ILI9341_H_
#define INC_ILI9341_H_

#define ILI9341_TX_BUFFER_SIZE 5 //avoid setting it below 5. 5 works and anything below that is uncalculated. 4 MIGHT be stable, very unsure. Don't try unless you are adventurous.
#define ILI9341_DC_BIT_IN_BUFFER 8
#define ILI9341_CSCHANGE_BIT_IN_BUFFER 9 //when CS bit changes, new value is kept as LSB in "data" area

typedef struct {
	uint16_t xResolution;
	uint16_t yResolution;

	uint16_t LCDCommandBufferSize;
	uint16_t LCDCommandBuffer[ILI9341_TX_BUFFER_SIZE+2];//8 LSB are commands, then D/C (bit8 of 15)
	uint16_t LCDCommandBufferIndex;
	uint16_t LCDCommandsInBuffer;
	//ili9341_state currentstate;						//unused as of right now
	bool (*handlerFunction)(); //handler function
	bool (*BLdisableFunction)(); //Turns backlight off
	bool (*BLenableFunction)(); //Turns backlight on
	bool (*RSTdisableFunction)(); //releases LCD from reset
	bool (*RSTenableFunction)(); //forces LCD to reset


} ili9341_instance;

#endif /* INC_ILI9341_H_ */
