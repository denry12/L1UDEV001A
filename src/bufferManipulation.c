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
	// this function does weird tricks to put two 8bit characters in one 16bit slot
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

	char dataToPut = data;
	if(instance->DataUnitsInBuffer < (instance->BufferSize - 2)){ //verify that there is room (and to spare)
		instance->Buffer[instance->BufferWriteIndex] = dataToPut;
		instance->BufferWriteIndex++;
		instance->Buffer[instance->BufferWriteIndex] = 0; //add null terminator
		instance->DataUnitsInBuffer++;
		if(instance->BufferWriteIndex >= instance->BufferSize)
			instance->BufferWriteIndex = 0; //go circular if necessary
			instance->Buffer[instance->BufferWriteIndex] = 0; //add null terminator
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
	//response |= circularBuffer8_put(instance, 0);


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

bool circularBuffer8_peek(circularBuffer_8bit *instance, uint8_t *data){ //shows next character that will pop out without popping it out

	if(instance->DataUnitsInBuffer <= 0) return 1; //buffer empty!?
	else {
		(*data) = (uint8_t)(instance->Buffer[instance->BufferReadIndex]);
	}

	return 0; //everything success
}



bool circularBuffer8_clear(circularBuffer_8bit *instance){
	instance->BufferWriteIndex = 0;
	instance->BufferReadIndex = 0;
	instance->DataUnitsInBuffer = 0;
	instance->Buffer[0] = 0;
	instance->Buffer[1] = 0;
	instance->Buffer[2] = 0;
	instance->Buffer[3] = 0;
	return 0;
}

bool circularBuffer8_init(circularBuffer_8bit *instance, uint16_t bufferLength, uint8_t *bufferLocation){
	instance->BufferSize = bufferLength;
	instance->Buffer = bufferLocation;
	circularBuffer8_clear(instance);
	//instance->BufferWriteIndex = 0;
	//instance->BufferReadIndex = 0;
	//instance->DataUnitsInBuffer = 0;
	return 0;
}

//if called for "data:ABCDEF;" with firststring as "data:" and "secondstring" as ";", will return "ABCDEF" to resultPtr
bool findBetweenTwoStrings(char *searchString, char *firstString, char *secondString, char *resultPtr){

	char *startIndex = 0, *endIndex = 0;
	startIndex = strstr(searchString, firstString);
	if ((!(startIndex))) return 1; //first string not found
	startIndex += strlen(firstString);
	endIndex = strstr(startIndex, secondString);
	if ((!(endIndex))) return 1; //second string not found

	memcpy(resultPtr, startIndex, (endIndex) - (startIndex));
	memcpy(resultPtr + ((endIndex) - (startIndex)), 0, 1); //add null terminator

	return 0; //all well
}

bool findBetweenTwoStrings_circularBuffer(circularBuffer_8bit *instance, char *firstString, char *secondString, char *resultPtr){
	// this function searches for string first as an entire (via other function), then
	// looks for first part after read index - if at least one character found, allows roll
	// over to buffer start and search for the rest

	// alternative solution to this function:
	// copy a smaller part from end and start, roughly the size necessary for firstString in worst case
	// so that it would be in one part, then adjust pointers where necessary


	bool response = 0;
	uint16_t i = instance->BufferReadIndex;
	uint16_t firstStringIndex = 0;  //todo: merge these two into one as "stringindex"
	uint16_t secondStringIndex; //todo: merge these two into one as "stringindex"
	uint16_t startIndex = 0, endIndex = 0; //these hold bufferinstance actual buffer indexes
	uint16_t *startIndexPtr = 0, *endIndexPtr = 0; //these are used only when response in one piece (not circular)
	uint16_t lengthOfResult = 0;
	bool startIndexFound = 0, endIndexFound = 0; //cause indexes may also be 0
	response = findBetweenTwoStrings(instance->Buffer+instance->BufferReadIndex, firstString, secondString, resultPtr);
	//response = 1; //to fake it was not found directly
	if (!(response)){ //read out from buffer so it is up to date
		//it is known that it is in one part
		//startIndexPtr = strstr(instance->Buffer+instance->BufferReadIndex, firstString);
		//endIndexPtr = strstr(startIndexPtr, secondString);

		endIndexPtr = (strstr(instance->Buffer+instance->BufferReadIndex, resultPtr));
		while (endIndexPtr > (instance->Buffer+instance->BufferReadIndex)){
			instance->BufferReadIndex += 1;
			instance->DataUnitsInBuffer -= 1;
			endIndexPtr = strstr(instance->Buffer+instance->BufferReadIndex, resultPtr);
		}
		instance->BufferReadIndex += strlen(resultPtr);//endIndexPtr - startIndexPtr;
		instance->DataUnitsInBuffer -= strlen(resultPtr); //endIndexPtr - startIndexPtr;
		return response; //found string
	}
	//char garbageForBufferEmptying;
	else if(response){
		//failed to find as a straight piece, might be available as circular
		//start looking from readindex start and search until get "0x00" or run out of buffer (which means it must be in one piece) or match
		while ((i <= instance->BufferSize) && (!(startIndexFound))){
			if(instance->Buffer[i] == 0) return 1; //not found, end  of string found instead
			else if (instance->Buffer[i] == firstString[firstStringIndex]) { //first correct character found, continue
				//now it is allowed to break string
				//firstStringIndex++;
				while(instance->Buffer[i] == firstString[firstStringIndex]){
					i++;
					if(i >= instance->BufferSize) i = 0; //necessary to go circular
					firstStringIndex++;
				}
				if(firstString[firstStringIndex] == 0){
					//firstString found, everything OK
					startIndex = i;
					startIndexFound = 1;
				}
			} else {
				//not end of string, not correct character
				firstStringIndex = 0;
			}
			i++;
		}
		secondStringIndex = 0;
		while (!(endIndexFound) && (startIndexFound)){
			if(i >= instance->BufferSize) i = 0; //necessary to go circular
			else if(instance->Buffer[i] == 0) return 1; //not found, end  of string found instead
			else if(instance->Buffer[i] == secondString[secondStringIndex]) {
				while(instance->Buffer[i] == secondString[secondStringIndex]){//correct character found, continue
					endIndex = i;
					//now it is allowed to break string
					if(i >= instance->BufferSize) i = 0; //necessary to go circular
					i++;
					secondStringIndex++;
				}
				if(secondString[secondStringIndex] == 0){
					//secondString found, everything OK
					//endIndex = i;
					endIndexFound = 1;
					break;
				}
			} else {
				//not end of string, not correct character
				if(endIndexFound == 0) firstStringIndex = 0;
			}


			i++;
		}



		if(endIndex && startIndex){ //both are nicely found

			if (endIndex > startIndex) lengthOfResult = endIndex - startIndex;
			else lengthOfResult = instance->BufferSize + endIndex - startIndex;

			memcpy(resultPtr, &(instance->Buffer[startIndex]), (instance->BufferSize) - (startIndex)); //first half
			memcpy(resultPtr+((instance->BufferSize) - (startIndex)), &(instance->Buffer[0]), endIndex); //first half //second half
			memcpy(resultPtr + lengthOfResult, 0, 1); //add null terminator

			//"read" buffer to this point
			*(&instance->BufferReadIndex) = i;

			//calculating how many datas I read out
			//if(instance->BufferSize - instance->BufferReadIndex - i)
			*(&instance->DataUnitsInBuffer) -= instance->BufferSize - instance->BufferReadIndex - i - 1; //+1 cause I want next character after my last character


			return 0; //all success
		}

		//return response;
	}

	//failed to find it starting at end of buffer and continuing at start, however it might be in one piece after buffer rollover
	//is there potential for it (do we have enough characters?)
	if((instance->BufferReadIndex + instance->DataUnitsInBuffer) > instance->BufferSize){
		//looks like it is possible (e.g. 500 + 1 = 501 (buffer[0]))
		response = findBetweenTwoStrings(instance->Buffer, firstString, secondString, resultPtr);
			if (!(response)){ //read out from buffer so it is up to date
				//it is known that it is in one part
				//startIndexPtr = strstr(instance->Buffer+instance->BufferReadIndex, firstString);
				//endIndexPtr = strstr(startIndexPtr, secondString);

				//endIndexPtr = (strstr(instance->Buffer, resultPtr));

				while (instance->BufferReadIndex > 0){
					//increment until we roll over (todo: replace with calculation)
					instance->BufferReadIndex += 1;
					instance->DataUnitsInBuffer -= 1;
					if (instance->BufferReadIndex >= instance->BufferSize) instance->BufferReadIndex = 0; //go circular
				}

				endIndexPtr = (strstr(instance->Buffer+instance->BufferReadIndex, resultPtr));
				while (endIndexPtr > (instance->Buffer+instance->BufferReadIndex)){
					instance->BufferReadIndex += 1;
					instance->DataUnitsInBuffer -= 1;
					endIndexPtr = strstr(instance->Buffer+instance->BufferReadIndex, resultPtr);
				}
				instance->BufferReadIndex += strlen(resultPtr);//endIndexPtr - startIndexPtr;
				instance->DataUnitsInBuffer -= strlen(resultPtr); //endIndexPtr - startIndexPtr;
				return response; //found string
			}
	}


	return 1; //if it got this far, there just is nothing there.
}
