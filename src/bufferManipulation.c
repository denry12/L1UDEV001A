/*
 * bufferManipulation.c
 *
 *  Created on: 6 May 2017
 *      Author: denry
 */


#include <stdint.h>
#include <stdbool.h>
#include "bufferManipulation.h"


//#define FBTSCB_BBUART //to enable bitbang debug messages from findbetweentwostrings_circularbuffer function


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

	char dataToPut = (char)data;
	if(instance->DataUnitsInBuffer < (instance->BufferSize - 2)){ //verify that there is room (and to spare)
		instance->Buffer[instance->BufferWriteIndex] = dataToPut;
		instance->BufferWriteIndex++;
		instance->Buffer[instance->BufferWriteIndex] = 0; //add null terminator
		instance->DataUnitsInBuffer++;
		if(instance->BufferWriteIndex >= instance->BufferSize){
			instance->BufferWriteIndex = 0; //go circular if necessary
			instance->Buffer[instance->BufferWriteIndex] = 0; //add null terminator
		}
	}
	else {
		//bitbangUARTmessage("BUFFER FULL!\r\n");
		return 1;
	}
	return 0; //everything success

}

// NB! This function does not add null terminator
bool circularBuffer8_put_string(circularBuffer_8bit *instance, char *data){
	uint16_t i = 0;
	bool response = 0;
	while (data[i] != 0){
		response |= circularBuffer8_put(instance, data[i]);
		i++;
	}
	//if(data[i] == 0) response |= circularBuffer8_put(instance, 0);


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

//if called for "data:ABCDEF;" with firststring as "data:" and "secondstring" as ";", will return "ABCDEF" to resultPtr
//and followingly "reads" the buffer up to that point as if it was just read out // NB, give strings between " chars, not '.
bool findBetweenTwoStrings_circularBuffer(circularBuffer_8bit *instance, char *firstString, char *secondString, char *resultPtr){
	// this function searches for string first as an entire (via other function), then
	// looks for first part after read index - if at least one character found, allows roll
	// over to buffer start and search for the rest

	// alternative solution to this function:
	// copy a smaller part from end and start, roughly the size necessary for firstString in worst case
	// so that it would be in one part, then adjust pointers where necessary


	// somewhat more reasonable way to rewrite this function is to
	// add subfunction to takes out a piece of buffer from startIndex and endIndex and "reads" it up to that part
	// then do search here and cut out in other function

	bool response = 0;
	uint16_t i = instance->BufferReadIndex;
	uint16_t firstStringIndex = 0;  //todo: merge these two into one as "stringindex"
	uint16_t secondStringIndex; //todo: merge these two into one as "stringindex"
	uint16_t startIndex = 0, endIndex = 0, initialReadIndex = instance->BufferReadIndex; //these hold bufferinstance actual buffer indexes
	uint8_t *startIndexPtr = 0, *endIndexPtr = 0; //these are used only when response in one piece (not circular)
	uint16_t lengthOfResult = 0;
	bool startIndexFound = 0, endIndexFound = 0; //cause indexes may also be 0
	bool goneCircular = 0; //to verify we don't remain endlessly looping
	response = findBetweenTwoStrings(instance->Buffer+instance->BufferReadIndex, firstString, secondString, resultPtr);
	//response = 1; //to fake it was not found directly


	if ((instance->BufferReadIndex + strlen(resultPtr) ) >  instance->BufferSize){
		//found data is out of buffer.
		response = 1; //so we need to consider it "not as one part in the end"
	}

	if (!(response)){ //read out from buffer so it is up to date
		//it is known that it is in one part
		endIndexPtr = (strstr(instance->Buffer+instance->BufferReadIndex, resultPtr));
		#ifdef FBTSCB_BBUART
		bitbangUARTmessage("Buff:OnePart1\r\n");
		bitbangUARTmessage("DUIB1=");
		bitbangUARTint(instance->DataUnitsInBuffer,0, 0);
		bitbangUARTmessage("/");
		bitbangUARTint(instance->BufferSize,0, 0);
		bitbangUARTmessage("\r\n");
		#endif




		while (endIndexPtr > (instance->Buffer+instance->BufferReadIndex)){
			if (instance->DataUnitsInBuffer == 0){
				//buffer is empty, do not dare to attempt reading anything
				return 1;
			}
			instance->BufferReadIndex += 1;
			instance->DataUnitsInBuffer -= 1;

			//bitbangUARTmessage("DUIB1=");
			//bitbangUARTint(instance->DataUnitsInBuffer,0, 0);
			//bitbangUARTmessage("\r\n");

				endIndexPtr = strstr(instance->Buffer+instance->BufferReadIndex, resultPtr);
		}
		instance->BufferReadIndex += strlen(resultPtr);
		instance->DataUnitsInBuffer -= strlen(resultPtr);
		instance->BufferReadIndex += strlen(secondString);
		instance->DataUnitsInBuffer -= strlen(secondString);
		if(instance->BufferReadIndex >= instance->BufferSize) instance->BufferReadIndex -= instance->BufferSize;

	//	if(instance->DataUnitsInBuffer > 65000){
	//						goneCircular = 0; //allow to get breakpoint if something went wrong
	//	}

		return response; //found string
	}



	// at this point "i == instance->BufferReadIndex;"
	else if(response){
		//failed to find as a straight piece, might be available as circular
		//start looking from readindex start and search until get "0x00" or run out of buffer (which means it must be in one piece) or match
		while ((i <= instance->BufferSize) && (!(startIndexFound))){
			if(instance->Buffer[i] == 0){ //not found, end  of string found instead
				if(i == instance->BufferReadIndex) //but only consider this if not the first character looked at cause rarely we start at the null terminator of last string
					return 1;
			}
			else if (instance->Buffer[i] == (char)(firstString[firstStringIndex])) { //first correct character found, continue
				//now it is allowed to break string
				while(instance->Buffer[i] == (char)(firstString[firstStringIndex])){
					i++;
					if(i >= instance->BufferSize) i = 0; //necessary to go circular
					firstStringIndex++;
				}
				if((char)(firstString[firstStringIndex]) == 0){
					//firstString found, everything OK
					startIndex = i; //here starts actual response

#ifdef FBTSCB_BBUART
	bitbangUARTmessage("startIndex ");
	bitbangUARTint(startIndex,0, 0);
	bitbangUARTmessage("\r\n");
#endif

					//startIndex += strlen(firstString); //here starts actual response
					//if(startIndex >= instance->BufferSize) startIndex -= instance->BufferSize;

					startIndexFound = 1;
					i--; //this is necessary, otherwise we will increment i soon too much before entering next loop
					//not optimal but... sue me.
				}
			} else {
				//not end of string, not correct character
				firstStringIndex = 0;
			}
			i++;
		}
		while (!(endIndexFound) && (startIndexFound)){
			if(i >= instance->BufferSize){ //necessary to go circular
				if (goneCircular) { // it is not allowed to go circular twice, otherwise we end up in endless loop.
					return 1; // make statement that no string was found.
				}
				i = 0;
				goneCircular = 1;
			}

			if(instance->Buffer[i] == secondString[secondStringIndex]) {
				while(instance->Buffer[i] == secondString[secondStringIndex]){//correct character found, continue
					//now it is allowed to break string
					if(i >= instance->BufferSize) i = 0; //necessary to go circular
					i++;
					secondStringIndex++;
				}
				if(secondString[secondStringIndex] == 0){
					//secondString found, everything OK
					endIndex = i - 1; //-1 cause currently we are at first char of "secondString"
					if(endIndex < strlen(secondString)) endIndex += instance->BufferSize;
					endIndex -= strlen(secondString);
					endIndexFound = 1;
					break;
				}
			} else {
				//not end of string, not correct character
				secondStringIndex = 0; //To start search again from beginning
			}


			i++;
		}



		if(endIndexFound && startIndexFound){ //both are nicely found


#ifdef FBTSCB_BBUART
	bitbangUARTmessage("InitialBufferReadIndex ");
	bitbangUARTint(instance->BufferReadIndex,0, 0);
	bitbangUARTmessage(";");
	bitbangUARTint(initialReadIndex,0, 0);
	bitbangUARTmessage("\r\n");
#endif

			if (endIndex > startIndex) lengthOfResult = (endIndex+1) - startIndex; // did not have rollover
			else lengthOfResult = instance->BufferSize + (endIndex+1) - startIndex; // did have rollover (went circular)
			//+1 for endindex cause it also contains data

			#ifdef FBTSCB_BBUART
			bitbangUARTmessage("!!!!!!Buff:Twopart;SI");
			bitbangUARTint(startIndex,0,0);
			bitbangUARTmessage(";EI");
			bitbangUARTint(endIndex,0,0);
			bitbangUARTmessage(";LOR");
			bitbangUARTint(lengthOfResult,0,0);
			bitbangUARTmessage(";BRI");
			bitbangUARTint(instance->BufferReadIndex,0,0);
			int temporaryBRI = instance->BufferReadIndex;
			#endif


			if(endIndex < startIndex) { //if actual data starts at end and rolls over
#ifdef FBTSCB_BBUART
			bitbangUARTmessage("endIndex<startIndex\r\n");
#endif
				memcpy(resultPtr, &(instance->Buffer[startIndex]), (instance->BufferSize) - (startIndex)); //first half
				memcpy(resultPtr + ((instance->BufferSize) - (startIndex) - 0), instance->Buffer, endIndex+1); //second half //endIndex+1 cuz it also contains data, along with slot 0
			} else { //if actual data starts at beginning and does not roll over
#ifdef FBTSCB_BBUART
			bitbangUARTmessage("endIndex>=startIndex\r\n");
#endif
				memcpy(resultPtr, &(instance->Buffer[startIndex]), ((endIndex + 1) - startIndex)); //second half (actually entireity) //endIndex+1 cuz it also contains data, along with slot 0
			}


			memcpy(resultPtr + lengthOfResult, 0, 1); //add null terminator

			//"read" buffer to this point
			*(&instance->BufferReadIndex) = i;
#ifdef FBTSCB_BBUART
			bitbangUARTmessage(";BRI'");
			bitbangUARTint(instance->BufferReadIndex,0,0);

			bitbangUARTmessage(";DUIB");
			bitbangUARTint(instance->DataUnitsInBuffer,0,0);
#endif
			//ADD FOR WHAT WAS BEFORE firstString!
			//I will change the use of startIndex from where data starts to where firstString starts
			if (startIndex >= strlen(firstString)) startIndex -= strlen(firstString);
			else startIndex = startIndex + instance->BufferSize - strlen(firstString);
			if ( initialReadIndex > startIndex){ // rollover has occurred, where data is in new segment
				*(&instance->DataUnitsInBuffer) -= (instance->BufferSize + startIndex) - initialReadIndex;
			}
			else *(&instance->DataUnitsInBuffer) -= startIndex - (initialReadIndex);

			//if((instance->DataUnitsInBuffer - lengthOfResult - strlen(firstString) - strlen(secondString)) > 65000){
			//	goneCircular = 0; //allow to get breakpoint if something went wrong
			//}

			// CURRENT ISSUE:
			// when looping with data of interest being split, "dataunitsinbuffer" rolls to max

			*(&instance->DataUnitsInBuffer) -= lengthOfResult; //14+5=19
			*(&instance->DataUnitsInBuffer) -= strlen(firstString); //4
			*(&instance->DataUnitsInBuffer) -= strlen(secondString); //1
#ifdef FBTSCB_BBUART
			bitbangUARTmessage(";DUIB'");
			bitbangUARTint(instance->DataUnitsInBuffer,0,0);
			bitbangUARTmessage(";CPY'D:");
			bitbangUARTmessage(resultPtr);
			bitbangUARTmessage(";FLW0:");
			bitbangUARTmessage(&(instance->Buffer[temporaryBRI]));
			bitbangUARTmessage(";FLW1:");
			bitbangUARTmessage(&(instance->Buffer[0]));
			bitbangUARTmessage(";FIN:");
			bitbangUARTmessage(resultPtr);
			bitbangUARTmessage(".");
			bitbangUARTmessage("\r\n");
#endif
			if(instance->DataUnitsInBuffer > 65000){
				goneCircular = 0; //allow to get breakpoint if something went wrong

			}

			return 0; //all success
		}
	}


	//failed to find it starting at end of buffer and continuing at start, however it might be in one piece after buffer rollover
	//is there potential for it (do we have enough characters?)
	if((instance->BufferReadIndex + instance->DataUnitsInBuffer) > instance->BufferSize){
		//looks like it is possible (e.g. 500 + 1 = 501 (buffer[0]))
		response = findBetweenTwoStrings(instance->Buffer, firstString, secondString, resultPtr);
			if (!(response)){ //read out from buffer so it is up to date
				//it is known that it is in one part
				//resultPtr now contains the data between firstString and secondString

				//but it is undetermined if it's valid (maybe in unused area of buffer)
				//we have already established we need2go circular
				if(!((instance->BufferReadIndex + instance->DataUnitsInBuffer - instance->BufferSize) //gives first free bufferslot address
						> (instance->BufferReadIndex - instance->BufferSize + strlen(firstString)+ strlen(secondString)+ strlen(resultPtr)))) { //and checks if it's same or higher than last used slot by message
					//and if there is no way it would fit in there, note that message just ain't there
					return 1;
				}

				while (instance->BufferReadIndex > 0){
					//increment until we roll over (todo: replace with calculation)
					if(instance->DataUnitsInBuffer == 0) return 1;
					instance->BufferReadIndex += 1;
					instance->DataUnitsInBuffer -= 1;

					//bitbangUARTmessage("DUIB2=");
					//bitbangUARTint(instance->DataUnitsInBuffer,0, 0);
					//bitbangUARTmessage("\r\n");

					if (instance->BufferReadIndex >= instance->BufferSize) instance->BufferReadIndex = 0; //go circular
				}

				startIndexPtr = (strstr(instance->Buffer+instance->BufferReadIndex, resultPtr)); //

				while (startIndexPtr > (instance->Buffer+instance->BufferReadIndex)){
					instance->BufferReadIndex += 1;
					instance->DataUnitsInBuffer -= 1;
				}

				//startIndexPtr holds now pointer to where message starts
				endIndexPtr = strstr(startIndexPtr, secondString);
				//and endIndexPtr now holds pointer to first char of secondString

				// no copying, cause resultPtr already holds message

				//and update buffer indexes. due to previous functions this starts at buffer[startIndex]
				while (endIndexPtr > (instance->Buffer+instance->BufferReadIndex)){
					instance->BufferReadIndex += 1;
					instance->DataUnitsInBuffer -= 1;
					//endIndexPtr = strstr(instance->Buffer+instance->BufferReadIndex, resultPtr);
				}
				instance->BufferReadIndex += strlen(secondString);
				instance->DataUnitsInBuffer -= strlen(secondString);
#ifdef FBTSCB_BBUART
				bitbangUARTmessage("Buff:Onepart2\r\n");
#endif
			//	if(instance->DataUnitsInBuffer > 65000){
			//		goneCircular = 0; //allow to get breakpoint if something went wrong
			//	}
				return response; //found string
			}
	}


	return 1; //if it got this far, there just is nothing there.
}


void dumpbuffer_8(circularBuffer_8bit *instance, bool validonly){
	char tinystring[2];
	tinystring[1] = 0;
	int i, j;
	i = 0;
	bitbangUARTmessage("DUMP:");
	if (validonly){
		j = instance->DataUnitsInBuffer;
		i = instance->BufferReadIndex;
		while(j){
			if(i >= instance->BufferSize){
				i = 0;
				bitbangUARTmessage("\n"); //rolled over
			}
			tinystring[0] = instance->Buffer[i];
			if(tinystring[0] == 0) bitbangUARTmessage("_");
			else bitbangUARTmessage(tinystring);
			j--;
			i++;
		}
	} else {
		while (i <= instance->BufferSize){
			tinystring[0] = instance->Buffer[i];
			if(tinystring[0] == 0) bitbangUARTmessage("_");
			else bitbangUARTmessage(tinystring);
			i++;
		}
		bitbangUARTmessage(" ");
		bitbangUARTint(instance->BufferReadIndex, 0, 2);
	}
	bitbangUARTmessage("\r\n");
	return;
}


// this function seems to run well as of 22.07.2017, only failing to report success if string cut in half
void buffertester_8(){

	uint32_t execCounter = 0;

	bitbangUARTmessage("\r\n\r\n");
	uint8_t testbufferData[40+2]; //nb, change this in init too //not sure whether I need that +2. Hopefully not
	circularBuffer_8bit testbuffer;
	circularBuffer8_init(&testbuffer, 40, &testbufferData);
	char temporarystring[40];
	uint8_t testmethod = 3;
	//char *pointer;
	bool result;
	while(1){

		bitbangUARTmessage("EXEC:");
		bitbangUARTint(execCounter,0,3);
		bitbangUARTmessage("\r\n");
		if ((execCounter == 7)||(execCounter == 8)){
			execCounter++;
			execCounter--;
		}

		//while(circularBuffer8_put_string(&testbuffer, ":HI;qwerty") == 0); //fill while can

		if(testmethod == 0){
		circularBuffer8_put_string(&testbuffer, "xy");
		circularBuffer8_put_string(&testbuffer, ":;:HI:;:");
		circularBuffer8_put_string(&testbuffer, "10");
		}
		if(testmethod == 1){
				circularBuffer8_put_string(&testbuffer, "xyz");
				circularBuffer8_put_string(&testbuffer, ":HI;");
				circularBuffer8_put_string(&testbuffer, "43210");
		}
		if(testmethod == 2){
				circularBuffer8_put_string(&testbuffer, "xyz");
				circularBuffer8_put_string(&testbuffer, ";;:HI;");
				circularBuffer8_put_string(&testbuffer, "10");
		}
		if(testmethod == 3){
			testbuffer.BufferReadIndex = 0;
			testbuffer.DataUnitsInBuffer = 0;
			//circularBuffer8_put_string(&testbuffer, ",14:LCDTXT:AYYLMAO\r\n blahblah +IPD:0");
			circularBuffer8_put_string(&testbuffer, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
			while(circularBuffer8_get(&testbuffer, temporarystring) == 0); //clear out the buffer
			circularBuffer8_put_string(&testbuffer, "+IPD,0,14:LCDTXT:AYYLMAO\r\n");
		}

		bitbangUARTmessage("FILLD");
		dumpbuffer_8(&testbuffer, 0); //all, not just valid

		bitbangUARTmessage("VALID");
		dumpbuffer_8(&testbuffer, 1); //only valid

		if(testmethod == 0) result = findBetweenTwoStrings_circularBuffer(&testbuffer, ":;:", ":;:", temporarystring);
		if(testmethod == 1) result = findBetweenTwoStrings_circularBuffer(&testbuffer, ":", ";", temporarystring);
		if(testmethod == 2) result = findBetweenTwoStrings_circularBuffer(&testbuffer, ":", ";", temporarystring);
		if(testmethod == 3) result = findBetweenTwoStrings_circularBuffer(&testbuffer, "+IPD,", ":", temporarystring);

		bitbangUARTmessage("VLD02");
		dumpbuffer_8(&testbuffer, 1); //only valid

		if (!(result)){
			bitbangUARTmessage("Found:");
			bitbangUARTmessage(temporarystring);
			bitbangUARTmessage(".\r\n");
		} else {
			//bitbangUARTmessage("Failed to find.\r\n");
			bitbangUARTmessage("Failed to find");
			bitbangUARTmessage(".\r\n");
		}

		//dumpbuffer_8(&testbuffer, 1);
		dumpbuffer_8(&testbuffer, 0); //all, not just valid
		while(circularBuffer8_get(&testbuffer, temporarystring) == 0); //clear out the buffer
		//bitbangUARTmessage("CLEAR");
		//dumpbuffer_8(&testbuffer, 0); //all, not just valid

		//bitbangUARTmessage("CLVLD");
		//dumpbuffer_8(&testbuffer, 1); //only valid

		bitbangUARTmessage("\r\n");
		execCounter++;
	}
}
