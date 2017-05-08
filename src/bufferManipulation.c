/*
 * bufferManipulation.c
 *
 *  Created on: 6 May 2017
 *      Author: denry
 */


#include <stdint.h>
#include <stdbool.h>
#include "bufferManipulation.h"

bool circularBuffer16_put(circularBuffer_16bit *instance, uint16_t *data){
	if(instance->DataUnitsInBuffer < (instance->BufferSize - 2)){ //verify that there is room (and to spare)
		instance->Buffer[instance->BufferWriteIndex] = (uint16_t)(data);
		instance->BufferWriteIndex++;
		instance->DataUnitsInBuffer++;
		if(instance->BufferWriteIndex >= instance->BufferSize) //if e.g. 20 (comes from define), set to 0, so max value used is 19
			instance->BufferWriteIndex = 0; //go circular if necessary
	}
	else {
		//bitbangUARTmessage("BUFFER FULL!\r\n");
		return 1;
	}
	return 0; //everything success

}

/*bool circularBuffer16_put_string_heavycasting(circularBuffer_16bit *instance, char *data){
	uint16_t i = 0;

	uint16_t currentChar16;
	while (data[i] != 0){

		if (data[i+1] != 0) currentChar16 = data[i] | (data[i+1] << 8);
		else {
			currentChar16 = data[i];
			circularBuffer16_put(instance, currentChar16);
			break;
		}

		circularBuffer16_put(instance, currentChar16);
		i++;
		i++; //double cause slapping 2x8bit at once
	}

	return 0;
}*/

bool circularBuffer16_put_string(circularBuffer_16bit *instance, char *data){
	uint16_t i = 0;
	bool response;
	while (data[i] != 0){
		response |= circularBuffer16_put(instance, data[i]);
		i++;
	}

	return response;
}

bool circularBuffer16_get(circularBuffer_16bit *instance, uint16_t *data){

	if(instance->DataUnitsInBuffer <= 0) return 1; //buffer empty!?
	else {
		*data = (uint16_t)(instance->Buffer[instance->BufferReadIndex]);
		instance->DataUnitsInBuffer--;
		instance->BufferReadIndex += 1;
		if(instance->BufferReadIndex >= instance->BufferSize) //if e.g. 20 (comes from define), set to 0, so max value used is 19
			instance->BufferReadIndex = 0; //go circular if necessary
	}

	return 0; //everything success
}

bool circularBuffer16_init(circularBuffer_16bit *instance, uint16_t bufferLength, uint16_t *bufferLocation){
	instance->BufferSize = bufferLength;
	instance->Buffer = bufferLocation;
	instance->BufferWriteIndex = 0;
	instance->BufferReadIndex = 0;
	instance->DataUnitsInBuffer = 0;
	return 0;
}


bool circularBuffer8_put(circularBuffer_8bit *instance, uint8_t *data){
	if(instance->DataUnitsInBuffer < (instance->BufferSize - 2)){ //verify that there is room (and to spare)
		instance->Buffer[instance->BufferWriteIndex] = (uint8_t)(data);
		instance->BufferWriteIndex++;
		instance->DataUnitsInBuffer++;
		if(instance->BufferWriteIndex >= instance->BufferSize)
			instance->BufferWriteIndex = 0; //go circular if necessary
	}
	else {
		//bitbangUARTmessage("BUFFER FULL!\r\n");
		return 1;
	}
	return 0; //everything success

}

bool circularBuffer8_put_string(circularBuffer_8bit *instance, char *data){
	uint16_t i = 0;
	bool response = 0;
	while (data[i] != 0){
		response |= circularBuffer8_put(instance, data[i]);
		i++;
	}

	return response;
}

bool circularBuffer8_get(circularBuffer_8bit *instance, uint8_t *data){

	if(instance->DataUnitsInBuffer <= 0) return 1; //buffer empty!?
	else {
		*data = (uint8_t)(instance->Buffer[instance->BufferReadIndex]);
		instance->DataUnitsInBuffer--;
		instance->BufferReadIndex += 1;
		if(instance->BufferReadIndex >= instance->BufferSize) //if e.g. 20 (comes from define), set to 0, so max value used is 19
			instance->BufferReadIndex = 0; //go circular if necessary
	}

	return 0; //everything success
}

bool circularBuffer8_init(circularBuffer_8bit *instance, uint16_t bufferLength, uint8_t *bufferLocation){
	instance->BufferSize = bufferLength;
	instance->Buffer = bufferLocation;
	instance->BufferWriteIndex = 0;
	instance->BufferReadIndex = 0;
	instance->DataUnitsInBuffer = 0;
	return 0;
}


//if called for "data:ABCDEF;" with firststring as "data:" and "secondstring" as ";", will return "ABCDEF" to resultPtr
bool findBetweenTwoStrings(char *searchString, char *firstString, char *secondString, char *resultPtr){

	char *startIndex = 0, *endIndex = 0; //uint16_t cause strings can be longer than 255 chars
	startIndex = strstr(searchString, firstString);
	if ((!(startIndex))) return 1; //first string not found
	startIndex += strlen(firstString);
	endIndex = strstr(startIndex, secondString);
	if ((!(endIndex))) return 1; //second string not found


	memcpy(resultPtr, startIndex, (endIndex) - (startIndex));
	memcpy(resultPtr + ((endIndex) - (startIndex)), 0, 1); //add null terminator

	return 0; //all well
}

bool findBetweenTwoStrings_circularBuffer(char *searchString, char *firstString, char *secondString, char *resultPtr){
	while(1); //undone.
	return 1; //fail
}
