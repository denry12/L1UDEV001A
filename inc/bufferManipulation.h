/*
 * bufferManipulation.h
 *
 *  Created on: 6 May 2017
 *      Author: denry
 */

#ifndef INC_BUFFERMANIPULATION_H_
#define INC_BUFFERMANIPULATION_H_


typedef struct {
	uint16_t BufferSize;
	uint16_t *Buffer;
	uint16_t BufferWriteIndex;
	uint16_t BufferReadIndex;
	uint16_t DataUnitsInBuffer; //basically gives difference of WriteIndex and ReadIndex
} circularBuffer_16bit;

typedef struct {
	uint16_t BufferSize;
	uint8_t *Buffer;
	uint16_t BufferWriteIndex;
	uint16_t BufferReadIndex;
	uint16_t DataUnitsInBuffer; //basically gives difference of WriteIndex and ReadIndex
} circularBuffer_8bit;


#endif /* INC_BUFFERMANIPULATION_H_ */
