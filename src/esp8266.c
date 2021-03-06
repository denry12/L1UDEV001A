/*
 * esp8266.c
 *
 *  Created on: 26 Jan 2017
 *      Author: denry
 */


//https://cdn.sparkfun.com/datasheets/Wireless/WiFi/Command%20Doc.pdf //questionable content

//http://dominicm.com/esp8266-send-receive-data/
//http://www.espressif.com/sites/default/files/4b-esp8266_at_command_examples_en_v1.3.pdf
//https://espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf
//	115200, CH_PD pin must be connected to VCC. To reset, ground the RST pin.
//	boot with CH_PD unconnected (or low?).
//	AT+CIPSTART=0,"UDP","192.168.0.6",9999$0D$0A
//	Commands: https://nurdspace.nl/ESP8266
//	No parity, 1 stopbit


//AT+RST$0D$0A <- fn avail. Content missing
//AT+CIPMUX=1$0D$0A <- fn avail. Content missing
//AT+CWJAP="MySSID","SecretPassDontTellAnyone"$0D$0A <- fn avail. Content missing
//AT+CIPCLOSE=0$0D$0A
//AT+CIPSTART=0,"UDP","192.168.0.21",9999$0D$0A
//AT+CIPSEND=0,8$0D$0A
//ABCDEFFF
//(data length 8, all letters sent)

#include <stdint.h>
#include <stdbool.h>
#include "esp8266.h"
#include "hd44780.h"
#include "bitbangUART.h"

//#define ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE //blasts stuff regarding receiving packets to BBUART

#ifndef uint8_t
#define uint8_t unsigned char
#endif
#ifndef uint16_t
#define uint16_t unsigned short
#endif
#ifndef uint32_t
#define uint32_t unsigned int
#endif

#define ESP8266_RETRY_COUNT 10
#define ESP8266_RETRY_DELAY 50

const char debug_testline_messages_SCARR = 0;

//char esp_8266_cipmux_latest = 0; //only modify via special function

bool esp8266_resetRxBuffer(esp8266_instance *instance){
	//instance->rxBufferSize = RX_BUFFER_SIZE;
	//instance->rxCircBufferIndex = 0;
	//instance->charactersInRxBuffer = 0;
	//instance->receivedFromESPbuffer[0] = 0;
	//instance->receivedFromESPbuffer[1] = 0; //hopefully not necessary
	circularBuffer8_clear(instance->receivedFromESPbuffer);
	return 0;
}

bool esp8266_resetTxBuffer(esp8266_instance *instance){
	//instance->txBufferSize = TX_BUFFER_SIZE;
	//instance->txCircBufferIndex = 0;
	//instance->charactersInTxBuffer = 0;
	//instance->sendToESPbuffer[0] = 0;
	//instance->sendToESPbuffer[1] = 0; //hopefully not necessary
	circularBuffer8_clear(instance->sendToESPbuffer);
	return 0;
}

bool esp8266_initalize(esp8266_instance *instance){
	esp8266_resetRxBuffer(instance);
	esp8266_resetTxBuffer(instance);
	instance->currentstate = ESP8266_STATE_IDLE;
	instance->rxPacketCount = 0;
	//instance->rxPacketBufferSize = RX_PACKET_CONTENT_MAX_SIZE*RX_PACKET_MAX_COUNT;
	//instance->rxPacketBufferIndex = 0;
	//instance->rxPacketBuffer[0] = 0;
	//instance->rxPacketBuffer[1] = 0; //hopefully not necessary
	return 0; //all OK
}

bool esp8266_charFromUartToBuffer(esp8266_instance *instance, char character){
	//is there room?
	/*if((instance->charactersInRxBuffer) >= (instance->rxBufferSize))
		return 1; //no room.

	instance->receivedFromESPbuffer[instance->rxCircBufferIndex] = character;
	instance->receivedFromESPbuffer[instance->rxCircBufferIndex+1] = 0; //add null terminator
	instance->rxCircBufferIndex++;

	//go circular if necessary
	if(instance->rxCircBufferIndex > instance->rxBufferSize) instance->rxCircBufferIndex = 0;
	instance->charactersInRxBuffer++;
	return 0; //all OK
	*/

	return circularBuffer8_put(instance->receivedFromESPbuffer, character);


}

int16_t esp8266_charFromBufferToUart(esp8266_instance *instance){
	char character;
	circularBuffer8_get(instance->sendToESPbuffer, &character);
	return character;
	/*//is there anything in buffer?
	if(instance->charactersInTxBuffer == 0)
		return -1; //o no

	char character;
	int index = (instance->txCircBufferIndex) - (instance->charactersInTxBuffer);
	if(index < 0)
		index += instance->txBufferSize;
	character = instance->sendToESPbuffer[index];
	instance->charactersInTxBuffer--;

	return character; //all OK*/
}

bool esp8266_sendString(esp8266_instance *instance, char *command){

	return circularBuffer8_put_string(instance->sendToESPbuffer, command);

	/*
	int i=0;
	while(command[i] != 0){
		if((instance->charactersInTxBuffer) < (instance->txBufferSize)){
			instance->sendToESPbuffer[(instance->txCircBufferIndex)] = command[i];
			instance->charactersInTxBuffer++;
			instance->txCircBufferIndex++;
		}
		i++;

	}

	return 0;
	*/
}

//HW specific code starts here
int esp8266_debugOutput(char *message){
	//!!!printf version
		//remove last char, ONLY if it's \n or \r
		//this is done for "printf", cause it seems to consider them equal, adding unnecessary empty lines
		/*char cutBuffer[strlen(text)];
		strcpy(cutBuffer, text);
		char *p = cutBuffer;
		if(((p[strlen(p)-1]) == '\n') || ((p[strlen(p)-1]) == '\r')) p[strlen(p)-1] = 0;
		printf(cutBuffer);*/

	//UART version
	//l11uxx_uart_Send(message);

	//violent uart action
	//bitbangUARTmessage(message);



	//why is there lcd? this why!
	//static int currentLine=0;
	//if(currentLine<5) currentLine++;;
	//lcd_5110_printString(0,currentLine, message);

	bitbangUARTmessage(message);


	lcd_5110_printAsConsole(message, 0);
	lcd_5110_redraw();
	return 1;
}















//"response" contains everything after \r\n after command echo
//and ends prior to "OK" (so ends with \r\n)
int esp8266_sendCommandAndReadResponse(esp8266_instance *instance, char *command, char *response){ //with error, errorcode may be contained in "response". Not guaranteed.
	//char *okResponse = 0;
	bool okResponse = 0;

	//bitbangUARTmessage("esp01addrSCARR: ");
	//bitbangUARThex(instance,0,0);
	//bitbangUARTmessage("\r\n");

	//todo: check if module is idle

	*response = 0;
	char retriesMax = 30;
	char retriesDone = 0;
	//unsigned int debug= 0 ;
	unsigned int lengthOfResponse = 0;
	int rxBufferLocalWaypoint;
	int i;


	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 50\n\r");
	//send out most of command
	esp8266_sendString(instance, command);
	esp8266_resetRxBuffer(instance);

	 //rxBufferLocalWaypoint=instance->rxCircBufferIndex; //keeps in mind where we actually start our response

	 //from this point on, I know what I am sending out. Just add it to the localwaypoint value
	 //rxBufferLocalWaypoint += (strlen(command));
	 //rxBufferLocalWaypoint += 2; //add \r\n

	 //rxBufferLocalWaypoint += 1; //otherwise a wild extra \r (0x0D) appears. Gotta investigate later maybe - not issue tho

	while(instance->sendToESPbuffer->DataUnitsInBuffer){ //do the actual sending from ESP buffer to UART, most part

			(*instance).sendCharToESP(instance);
	}

		//finalize command
	esp8266_sendString(instance, "\x0D\x0A"); //this is what ESP responds as well. \r\n (checked at 21.04.2017)

	instance->currentstate = ESP8266_STATE_TXNEEDED;

	while(instance->sendToESPbuffer->DataUnitsInBuffer){ //do the actual sending from ESP buffer to UART

		(*instance).sendCharToESP(instance);
	}

	instance->currentstate = ESP8266_STATE_WAITINGRESPONSE;









	esp8266_debugOutput("CmdR:");
	esp8266_debugOutput(command);
	esp8266_debugOutput("\n\r");

	while(instance->receivedFromESPbuffer->DataUnitsInBuffer < 2){ //deffo not enough data
		//this should be replaced with check if I have a response in buffer
		//(*instance).getCharFromESP(instance);
		while((*instance).getCharFromESP(instance) == 0);//{ //and if data still keeps on coming
		retriesDone++;
		if(retriesDone>=retriesMax) break;
		bitbangUARTloadingbar(retriesDone, retriesMax-1);
		bitbangUARTmessage(" 1/3  ");
		delay(100);
	}

	if(retriesDone>=retriesMax){
			esp8266_debugOutput("FAIL(A)1/3\n\r");
			esp8266_debugOutput("\n\r");
			instance->currentstate = ESP8266_STATE_IDLE;
			return 0; //very broken
	}


	while((*instance).getCharFromESP(instance) == 0);//{ //and if data still keeps on coming

	//right now buffer still has echo inside. Read out until \r or \n
	circularBuffer8_peek(instance->receivedFromESPbuffer, response);
	while ( (!((*(response)) == '\r')) && (!( (*(response)) == '\n'))){
			if(circularBuffer8_get(instance->receivedFromESPbuffer, response)) return 0; //something is broken, buffer empty
			if(circularBuffer8_peek(instance->receivedFromESPbuffer, response)) return 0; //something is broken, buffer empty
	}


	//at this point my Rx buffer should start with \r\r\n or \r\n or smth like that
	//read it out and have actual response available
	circularBuffer8_peek(instance->receivedFromESPbuffer, response); //unnecessary, already have it from previous reading
	while (( (*(response)) == '\r') || ( (*(response)) == '\n')){
		if(!(circularBuffer8_get(instance->receivedFromESPbuffer, response))) retriesDone = 0;
		if(!(circularBuffer8_peek(instance->receivedFromESPbuffer, response))) retriesDone = 0;
		else {
			retriesDone++;
			if(retriesDone>=retriesMax) break;
			bitbangUARTloadingbar(retriesDone, retriesMax-1);
			bitbangUARTmessage(" 2/3  ");
			delay(100);

		}
	}
	if(retriesDone>=retriesMax){
			esp8266_debugOutput("FAIL(A)2/3\n\r");
			esp8266_debugOutput("\n\r");
			instance->currentstate = ESP8266_STATE_IDLE;
			return 0; //very broken
	}




	//next thing that will pop out is response
	i=0;
	while(!( (circularBuffer8_get(instance->receivedFromESPbuffer, ((response+i) )) ))){
		//as long as it doesn't end with a 0, copy to response
		if( (*(response+i)) == 0) break;
		i++;
	}
	(*(response+i)) = 0;	//add null terminator

	//bool readDataReturnValue = (*instance).getCharFromESP(instance);



	bitbangUARTloadingbar(retriesMax-1, retriesMax-1);






	retriesDone=0;
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 200\n\r");
	while(!(okResponse)){
		(*instance).getCharFromESP(instance);
		//maybe error instead!? no point to wait then
		//findBetweenTwoStrings_circularBuffer(instance->receivedFromESPbuffer, "\r\n", "\r\n", &response);


		//TODO: GO BACK FROM findBetweenTwoStrings to strstr

		//if(!findBetweenTwoStrings_circularBuffer(instance->receivedFromESPbuffer, "ERROR", "\r\n", response)){
		if(strstr(response, "ERROR")){
			esp8266_debugOutput("FAIL(E)\n\r");
			strcpy(response, "ERROR");
			instance->currentstate = ESP8266_STATE_IDLE;
			return 0; //very broken
		}
		//if(!findBetweenTwoStrings_circularBuffer(instance->receivedFromESPbuffer, "FAIL", "\r\n", response)){
		if(strstr(response, "FAIL")){
			esp8266_debugOutput("FAIL(F)\n\r");
			strcpy(response, "FAIL");
			instance->currentstate = ESP8266_STATE_IDLE;
			return 0; //very broken
		}

		//if(!findBetweenTwoStrings_circularBuffer(instance->receivedFromESPbuffer, "busy", "\r\n", response)){
		if(strstr(response, "busy")){
			esp8266_debugOutput("FAIL(b)\n\r");
			strcpy(response, "busy");
			instance->currentstate = ESP8266_STATE_IDLE;
			return 0; //very broken
		}
		//okResponse = (!findBetweenTwoStrings_circularBuffer(instance->receivedFromESPbuffer, "OK", "\r\n", response));
		okResponse = strstr(response, "OK");

		//this should contain OK as well
		//THIS HERE IS USEFUL DEBUG <- used to be, that is
		//bitbangUARTmessage("What I think I received: ");
		//bitbangUARTmessage(instance->receivedFromESPbuffer + rxBufferLocalWaypoint);

		retriesDone++;
		if(retriesDone>=retriesMax) break;
		bitbangUARTloadingbar(retriesDone, retriesMax-1);
		bitbangUARTmessage(" 3/3");
		delay(100);
	}
	bitbangUARTloadingbar(retriesMax-1, retriesMax-1);
	esp8266_debugOutput("\n\r");
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 240\n\r");

	if(retriesDone>=retriesMax){
			esp8266_debugOutput("FAIL(A)3/3\n\r");
			instance->currentstate = ESP8266_STATE_IDLE;
			return 0; //very broken
	}
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 245\n\r");
	//lengthOfResponse = okResponse; //DEBUG <- often 0x0000017b (including OK)
	//lengthOfResponse = (int)(&l11uxx_uart_rx_buffer); // <- gives nicely same as RxBSt
	//lengthOfResponse = (int)(okResponse) - (int)(&instance->receivedFromESPbuffer);

	//lengthOfResponse = (instance->rxCircBufferIndex - rxBufferLocalWaypoint);
	//lengthOfResponse -= 5; //stop from copying last two characetrs ("OK"+\r\n\x00)
	lengthOfResponse = strlen(&response);
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 250\n\r");

	//bitbangUARTmessage((&instance->receivedFromESPbuffer) + instance->rxCircBufferIndex - instance->charactersInRxBuffer);// + instance->rxCircBufferIndex);

	//"response" shall contain string between \r\n from sent command echo to OK
	//strncpy(response, instance->receivedFromESPbuffer + rxBufferLocalWaypoint, lengthOfResponse);



	//by now "response" should be formatted
	//THIS HERE IS USEFUL DEBUG
	//bitbangUARTmessage("Got response: ");
	//bitbangUARTmessage(response);


	//cut string
	//rxBufferLocalWaypoint=strlen(command);

	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 255\n\r");
	//rxBufferLocalWaypoint = 0; //cleared, cause now we are looking through "response", not buffer
	//while((response[rxBufferLocalWaypoint] == '\r') || (response[rxBufferLocalWaypoint] == '\n')) rxBufferLocalWaypoint++;
	//for (i=0; i<lengthOfResponse; i++) response[i] = response[i+rxBufferLocalWaypoint];
	//response[lengthOfResponse-rxBufferLocalWaypoint] = 0; //add null terminator to be sure. Likely not necessary but I have trust issues.


	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 260\n\r");

	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 320\n\r");

	//if you made it this far, all OK!
	bitbangUARTmessage("CMD ok, returning\r\n");
	instance->currentstate = ESP8266_STATE_IDLE;
	return 1; //is OK
	return 0; //very broken

}















int esp8266_sendCommandAndWaitOK(esp8266_instance *instance, char *command){
	char response[80]; //this could be written anywhere, it is never read
	//80 chars seems to be enough to fit any crap it may output

	if(esp8266_sendCommandAndReadResponse(instance, command, response)){ //this "response" is discarded, so it should be written to somewhere where nobody cares
		//esp8266_debugOutput("\r\n"); //cause I don't get newline with this
		return 1; //is OK
	}
	//this fail is actually outputted in sendandread function as well
	//esp8266_debugOutput("FAIL\r\n"); //cause I don't get newline with this
	return 0; //very broken
}


//returns response starting after "+IPD,", so it starts with connection ID.
bool esp8266_checkForRxPacket(esp8266_instance *instance, char *response){
	//esp8266_resetRxBuffer(instance);
	uint16_t packetLen = 0;
	uint16_t bytesReceived = 0;
	uint16_t i = 0;

	uint16_t millisecsToWaitData = 200; //TODO: return this to 20, 20 is enough.
	uint16_t millisecsWaited = 0;

	char lengthString[4] = {0, 0, 0, 0}; //fill it with null terminators to avoid waiting for 1066 bytes instead of 10

	if((*instance).getCharFromESP(instance) == 0){
		//some new data incoming
		while(millisecsWaited <= millisecsToWaitData){
			if((*instance).getCharFromESP(instance) == 0){
				//if more coming, reset timeout timer
				millisecsWaited = 0;
			} else {
				//else increment timer
				millisecsWaited++;
			}

			delay(1);
		}
	}

	char *responsePtr = 0;
	char responseString[RX_PACKET_CONTENT_MAX_SIZE+10]; //add room for header
	 responseString[0] = 0;
	 responseString[1] = 0;
	 responseString[2] = 0;
	 responseString[3] = 0;
	 responseString[4] = 0;
	 responseString[5] = 0;
	 char *responseData;
	char *lenStartPtr = 0;
	char *lenEndPtr = 0;
	char charactersFromBuffer[2];
	//char *responseEndPtr = 0;
	if ((findBetweenTwoStrings_circularBuffer(instance->receivedFromESPbuffer, "+IPD,", ":", responseString))) return 1; //no data :' (
	//responsestring contains "<id>,<len>"
	strcat(responseString,":"); //because it is one of the limiter-markers, it doesn't come along

	lenStartPtr = strstr((responseString), ","); //gets first comma from "<id>,<len>:"
	lenStartPtr = lenStartPtr+1; //this sets "lenStartPtr" just after first "," in "<id>,<len>:"
	//lenEndPtr = strstr(lenStartPtr, ":");

	//and last protection prior to strncpy
	//if(!(lenStartPtr) || (!(lenEndPtr))) return 1;

	strcpy(lengthString, lenStartPtr);

	//memcpy(lengthString, lenStartPtr, lenEndPtr-lenStartPtr);
	//lengthString [(lenEndPtr - lenStartPtr)] = 0; //add null terminator


	//PROBLEM: length is discovered faultily. Because "responseString" comes in broken - WHOA it is all correct. But it goes nuts when short packet comes

	//if ((findBetweenTwoStrings_circularBuffer(instance->receivedFromESPbuffer, ",", ":", lengthString)))
		//return 1; //find lengthstring
	packetLen = atoi(lengthString);

	if (packetLen < 5){ //raise an alert
		bitbangUARTmessage("SHORT PACKET DETECTED!\r\n");
	}

	//at this point, "responseString" has only "<id>,<len>"
	//so gotta add dataLen amount of bytes

	charactersFromBuffer[1] = 0;
	//bitbangUARTmessage("0)To buffer:");
	while(i < (packetLen)){
		if (circularBuffer8_get(instance->receivedFromESPbuffer, charactersFromBuffer) == 0){

			charactersFromBuffer[1] = 0; //grr it shouldn't be necessary!?
			//bitbangUARTmessage(charactersFromBuffer);
			//if(charactersFromBuffer[0] == 0) strcat(responseString,'!'); //string is broken
			//else
			strcat(responseString,charactersFromBuffer);
		}	else return 1;
		i++;
	}

	//bitbangUARTmessage(".\r\n");

	//bytesReceived = (lenEndPtr + 1) ???;
/*
	bytesReceived = strlen(lenEndPtr+1);
	while (bytesReceived < packetLen) {
		while(! ((*instance).getCharFromESP(instance))); //and if data still keeps on coming
		if (circularBuffer8_get(instance, lenEndPtr+bytesReceived) == 0) bytesReceived += 1;

	}*/

	//while((*instance).getCharFromESP(instance) == 0); //and if data still keeps on coming
	/*while(bytesReceived <= packetLen){
		bytesReceived = ((instance->receivedFromESPbuffer->DataUnitsInBuffer));
		(*instance).getCharFromESP(instance);
	}*/

	//TODO: check if lengthstring matches data length, if not, return error

#ifdef ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE
	bitbangUARTmessage("1)Getting data:");
	//esp8266_debugOutput(responsePtr);
	bitbangUARTmessage(responseString);
	bitbangUARTmessage(".\r\n");
#endif

#ifdef ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE
	bitbangUARTmessage("2)RxPktmemmove:");
	bitbangUARThex(response,0,8);
	bitbangUARTmessage(" ");
	bitbangUARThex(responseString,0,8);
	bitbangUARTmessage(" ");
	bitbangUARTint(strlen(responseString),0,0);
	bitbangUARTmessage(".\r\n");
#endif

	//memmove(response, responsePtr, strlen(responsePtr)+1);
	//memmove(response, responseString, strlen(responseString)); //no wanna use memmove cause doesn't add 0 terminator

	//if(strlen(responseString) == 0){
	//	//empty string, what are you trying to pull
	//	return 1;
	//}

	strcpy(response, responseString);

#ifdef ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE
	bitbangUARTmessage("2.2)PostStrCpy:");
	bitbangUARTmessage(response);
	bitbangUARTmessage(".\r\n");
#endif

	return 0; //success
}

int esp8266_isAlive(esp8266_instance *instance){
	if(esp8266_sendCommandAndWaitOK(instance, "AT")) return 1; //is OK;
	return 0; //very broken

}

int esp8266_SWreset(esp8266_instance *instance){
	esp8266_sendCommandAndWaitOK(instance, "AT+RST");
	return 0; //very broken
	return 1; //is OK
}

int esp8266_setUARTMode(esp8266_instance *instance, int baudrate, char bits, char parity, char flowControl){
	char modeConfString[40];
	strcpy(modeConfString,"AT+UART_CUR=");

	//TODO: make this part universal
	if(baudrate==9600)strcat(modeConfString,"9600");
	else if(baudrate==110)strcat(modeConfString,"110");
	else if(baudrate==300)strcat(modeConfString,"300");
	else if(baudrate==19200)strcat(modeConfString,"19200");
	else if(baudrate==115200)strcat(modeConfString,"115200");
	strcat(modeConfString,",");
	//strcat(modeConfString,"8,2,0");
	strcat(modeConfString,"8,1,0"); //8bits, 1stopbit, no parity

	//ATTENTION:
	//As of 9th of February, 2017
	//setting "Flow Control" to "0" (disable)
	//ESP8266 heats up a lot, eventually causing
	//voltage drop to reset MCU
	//
	//Using "1" (RTS) confuses poor LPC
	//Using "2" (CTS) works well
	//Using "3" probably confuses, cause has RTS.
	strcat(modeConfString,",1"); //keep it as 1, unless you know why and what you are doing. Ignore previous comment for some reason!?

	bitbangUARTmessage("Doing CMD for UARTspeed\r\n");
	if(esp8266_sendCommandAndWaitOK(instance, modeConfString)){
		bitbangUARTmessage("UARTSpeed CMD success\r\n");
		return 1; //is OK;
	}
	bitbangUARTmessage("UARTSpeed CMD fail\r\n");
	return 0; //very broken

}

//int esp8266_HWreset()

int esp8266_setMode(esp8266_instance *instance, int mode){
	//1 = sta <- this is client
	//2 = AP <- this is "router", but without DHCP
	//3 = both
	switch(mode){
		case 1:
			if(esp8266_sendCommandAndWaitOK(instance, "AT+CWMODE_CUR=1")) return 0; //is OK
			break;
		case 2:
			if(esp8266_sendCommandAndWaitOK(instance, "AT+CWMODE_CUR=2")) return 0; //is OK
			break;
		default:
			if(esp8266_sendCommandAndWaitOK(instance, "AT+CWMODE_CUR=3")) return 0; //is OK
			break;
	}
	return 1; //very broken
}

//int esp8266_getNetworkList(){
//
//	return 0; //very broken
//	return 1; //is OK
//}

int esp8266_setCipmux(esp8266_instance *instance, int isMultichannel){
	//if 1, then multiple connections
	//if 0, is single connection
	if(isMultichannel){
		if(esp8266_sendCommandAndWaitOK(instance, "AT+CIPMUX=1")){
			instance->cipmux_latest = isMultichannel; //updated only if all went well
			return 1; //is OK
		}
	} else {
		if(esp8266_sendCommandAndWaitOK(instance, "AT+CIPMUX=0")){
			instance->cipmux_latest = isMultichannel; //updated only if all went well
			return 1; //is OK
		}
	}
	return 0; //very broken

}

int esp8266_joinAP(esp8266_instance *instance, char *ssid, char *passwd){
	volatile char modeConfString[80];
	int retryCounter=0;
	const int maxRetries=10;
	int i;
	bitbangUARTmessage("Attempting to join AP: ");
	bitbangUARTmessage(ssid);
	bitbangUARTmessage(" with pass: ");
	bitbangUARTmessage(passwd);
	bitbangUARTmessage("\n\r");

	strcpy(modeConfString,"AT+CWJAP_CUR=");
	strcat(modeConfString,"\x22"); //" character
	strcat(modeConfString,ssid);
	strcat(modeConfString,"\x22"); //" character
	strcat(modeConfString,",");
	strcat(modeConfString,"\x22"); //" character
	strcat(modeConfString,passwd);
	strcat(modeConfString,"\x22"); //" character

	if(esp8266_sendCommandAndWaitOK(instance, modeConfString)) return 0; //is OK;
	//likely timed out
	//check status
	strcpy(modeConfString,"");
	while(retryCounter<maxRetries){
		if(esp8266_sendCommandAndReadResponse(instance, "AT+CIPSTATUS", &modeConfString)){
			//bitbangUARTmessage("\n\r!!");
			bitbangUARTmessage(modeConfString);
			//bitbangUARTmessage("!!\n\r");
			if(strstr(modeConfString, "STATUS:2")){
				bitbangUARTmessage("Wifi connected succesfully!!\n\r");
				return 0;
			} else {
				//gets OK response but does not contain STATUS:"
				bitbangUARTmessage("Wifi connect failed.\n\r");
				return 1; //very broken
			}
		}
		else if(strstr(modeConfString, "busy")){
			//extra grace time cause busy
			bitbangUARTmessage("Extra time cause reported busy!\n\r");
			for(i=0; i<=10; i++){
				bitbangUARTloadingbar(i, 10);
				delay(500);
			}
			bitbangUARTmessage("\r\n");
		} else delay(1000);
	retryCounter++;
	}


	return 1; //very broken
}

int esp8266_leaveAP(esp8266_instance *instance){
	//AT+CWQAP
	bitbangUARTmessage("Ditching AP\n\r");
	if(esp8266_sendCommandAndWaitOK(instance, "AT+CWQAP")){
		bitbangUARTmessage("Ditched AP successfully\n\r");
		return 0; //is OK
	}
	bitbangUARTmessage("AP ditch fail\n\r");
	return 1; //very broken

}

int esp8266_getOwnIP(esp8266_instance *instance, char *IPoutput){
	//AT+CIFSR
	//volatile char modeConfString[80];
	//int debug=0;
	char *stringCutPointer;
	bitbangUARTmessage("Getting IP\n\r");
	if(esp8266_sendCommandAndReadResponse(instance, "AT+CIFSR", IPoutput)){

		//currently has +CIRFSR:STAIP,"xxx.xxx.xxx.xxx"\r\n+CIFSR:STAMAC,"ab:cd:ab:cd:ab:cd"\r\n"
		stringCutPointer = strstr(IPoutput, "+CIFSR:STAIP,\x22");
		stringCutPointer = stringCutPointer + strlen("+CIFSR:STAIP,\x22");
		memmove(IPoutput, stringCutPointer, strlen(IPoutput)+1);  //currently has xxx.xxx.xxx.xxx"\r\n+CIFSR:STAMAC,"ab:cd:ab:cd:ab:cd"\r\n"

		stringCutPointer = strstr(IPoutput, "\x22"); 	//find next " symbol
		if(!(stringCutPointer)){
					bitbangUARTmessage("IP string not found\n\r");
					bitbangUARTmessage("No IP 4 U\n\r");
					return 1; //very broken
				}
		stringCutPointer[0] = 0; 						//and slam a null terminator there
														//NB! No \r\n

		bitbangUARTmessage("Looks like IP is: ");
		//debug = (int)(&IPoutput);
		bitbangUARTmessage((IPoutput));
		bitbangUARTmessage("\n\r");
		return 0; //is OK
	}
	bitbangUARTmessage("No IP 4 U\n\r");
	return 1; //very broken
}


int esp8266_getOwnMAC(esp8266_instance *instance, char *IPoutput){
	//AT+CIFSR
	//volatile char modeConfString[80];
	//int debug=0;
	char *stringCutPointer;
	bitbangUARTmessage("Getting MAC\n\r");
	if(esp8266_sendCommandAndReadResponse(instance, "AT+CIFSR", IPoutput)){

		//currently has +CIRFSR:STAIP,"xxx.xxx.xxx.xxx"\r\n+CIFSR:STAMAC,"ab:cd:ab:cd:ab:cd"\r\n"
		//bitbangUARTmessage("Doing cutting\n\r");
		stringCutPointer = strstr(IPoutput, "+CIFSR:STAMAC,\x22");
		//bitbangUARTmessage("1");
		stringCutPointer = stringCutPointer + strlen("+CIFSR:STAMAC,\x22");
		//bitbangUARTmessage("2");
		memmove(IPoutput, stringCutPointer, strlen(IPoutput)+1);  //currently has ab:cd:ab:cd:ab:cd"\r\n"
		//bitbangUARTmessage("3");
		stringCutPointer = strstr(IPoutput, "\x22"); 	//find next " symbol
		if(!(stringCutPointer)){
			bitbangUARTmessage("MAC string not found\n\r");
			bitbangUARTmessage("No MAC 4 U\n\r");
			return 1; //very broken
		}
		//bitbangUARTmessage("4");
		stringCutPointer[0] = 0; 						//and slam a null terminator there
														//NB! No \r\n
		//bitbangUARTmessage("5\n\r");
		bitbangUARTmessage("Looks like MAC is: ");
		//debug = (int)(&IPoutput);
		bitbangUARTmessage((IPoutput));
		bitbangUARTmessage("\n\r");
		return 0; //is OK
	}
	bitbangUARTmessage("No MAC 4 U\n\r");
	return 1; //very broken
}

bool esp8266_closeConnection(esp8266_instance *instance, uint8_t id){
	char modeConfString[80];
	char idString[2]; //should never go over 1 char+nullterminator
	strcpy(modeConfString,"AT+CIPCLOSE");
	if(instance->cipmux_latest == 0){
		bitbangUARTmessage("Closing only connection.\r\n");
	} else {
		strcat(modeConfString,"=");
		itoa(id, idString, 10);
		strcat(modeConfString, idString);
		bitbangUARTmessage("Closing connection ");
		bitbangUARTmessage(idString);
		//bitbangUARTint(id, 0, 1);
		bitbangUARTmessage(".\r\n");
	}

	if(esp8266_sendCommandAndWaitOK(instance, modeConfString)){
		bitbangUARTmessage("Closed.\r\n");
		return 0; //is OK
	}
	bitbangUARTmessage("Not closed.\r\n");
	return 1; //very broken
}

bool esp8266_openConnection(esp8266_instance *instance, uint8_t id, char *type, char *ip, uint16_t port){
	char modeConfString[80];
	char idString[2]; //should never go over 1 char+nullterminator
	char portString[6]; //should never go over 5 char+nullterminator

	itoa(id, idString, 10);
	itoa(port, portString, 10);

	if(strcmp(type, "TCP") == 0){
		//open link
		//AT+CIPSTART="TCP","<IP>",<PORT>,<KEEPALIVE> //keepalive e.g. 1000, in seconds.

		//start server?????
		//AT+CIPSERVER=1,<port>


		strcpy(modeConfString,"AT+CIPSTART=");

		if(instance->cipmux_latest == 0){
			bitbangUARTmessage("Opening only connection to ");
			bitbangUARTmessage(ip);
			bitbangUARTmessage(":");
			bitbangUARTmessage(portString);
			bitbangUARTmessage(".\r\n");
		} else {
			strcat(modeConfString, idString);
			bitbangUARTmessage("Opening connection ");
			bitbangUARTmessage(idString);
			bitbangUARTmessage(" to ");
			bitbangUARTmessage(ip);
			bitbangUARTmessage(":");
			bitbangUARTmessage(portString);
			bitbangUARTmessage(".\r\n");
		}

		strcat(modeConfString, ",\x22"); //add ,"
		strcat(modeConfString, type);
		strcat(modeConfString, "\x22,\x22"); //add ","
		strcat(modeConfString, ip);
		strcat(modeConfString, "\x22,"); //add ",
		strcat(modeConfString, portString); //TCP remote port
		strcat(modeConfString, ","); //add ,
		strcat(modeConfString, "1000"); //keepalivetime


	} else if(strcmp(type, "UDP") == 0){


		strcpy(modeConfString,"AT+CIPSTART=");

		if(instance->cipmux_latest == 0){
			bitbangUARTmessage("Opening only connection to ");
			bitbangUARTmessage(ip);
			bitbangUARTmessage(":");
			bitbangUARTmessage(portString);
			bitbangUARTmessage(".\r\n");
		} else {
			strcat(modeConfString, idString);
			bitbangUARTmessage("Opening connection ");
			bitbangUARTmessage(idString);
			bitbangUARTmessage(" to ");
			bitbangUARTmessage(ip);
			bitbangUARTmessage(":");
			bitbangUARTmessage(portString);
			bitbangUARTmessage(".\r\n");
		}

		strcat(modeConfString, ",\x22"); //add ,"
		strcat(modeConfString, type);
		strcat(modeConfString, "\x22,\x22"); //add ","
		strcat(modeConfString, ip);
		strcat(modeConfString, "\x22,"); //add ",
		strcat(modeConfString, portString); //UDP remote port
		strcat(modeConfString, ","); //add ,
		strcat(modeConfString, portString); //UDP local port
		strcat(modeConfString, ",0"); //add , and "destination peer entity of UDP will not change"
	} else return 1; //very broken, I only support TCP and UDP

	if(esp8266_sendCommandAndWaitOK(instance, modeConfString)){
		bitbangUARTmessage("Opened.\r\n");
		return 0; //is OK
	}
	bitbangUARTmessage("Not opened.\r\n");
	return 1; //very broken
}

int esp8266_sendData(esp8266_instance *instance, uint8_t id, int length, char *data[]){
	char modeConfString[80];
	char responseString[20];
	char idString[2]; //should never go over 1 char+nullterminator
	char lengthString[5]  = {0, 0, 0, 0, 0}; //should never go over 4 char+nullterminator
	char *okResponse = 0;
	strcpy(modeConfString,"AT+CIPSEND=");

	if(instance->cipmux_latest != 0){
		itoa(id, idString, 10);
		strcat(modeConfString, idString);
	}
	strcat(modeConfString,",");
	itoa(length, lengthString, 10);
	strcat(modeConfString, lengthString);

	if(esp8266_sendCommandAndWaitOK(instance, modeConfString) == 0){ //returns 1 if all cool cause outdated.
		instance->currentstate = ESP8266_STATE_IDLE;
		esp8266_debugOutput("Send FAIL! (not ready)\r\n");
		return 1; //very broken
	}
	instance->currentstate = ESP8266_STATE_WAITINGREADYFORDATA;

	//going slightly deeper
	esp8266_sendString(instance, data);
	instance->currentstate = ESP8266_STATE_TXNEEDED;

	while(instance->sendToESPbuffer->DataUnitsInBuffer){ //do the actual sending from ESP buffer to UART

		(*instance).sendCharToESP(instance);
	}

	//verify that sending succeeded (looking for "SEND OK" string basically)
	while((*instance).getCharFromESP(instance) == 0);//{ //and if data still keeps on coming



	findBetweenTwoStrings_circularBuffer(instance->receivedFromESPbuffer, "\r\n", "\r\n", &responseString);
	okResponse = strstr(&responseString, "OK");
	if (okResponse == 0){
		instance->currentstate = ESP8266_STATE_IDLE;
		esp8266_debugOutput("Send FAIL!\r\n");
		return 1; //very broken
	}

	instance->currentstate = ESP8266_STATE_IDLE;
	esp8266_debugOutput("Send success.\r\n");
	return 0; //is OK


}

bool esp8266_receiveHandler(esp8266_instance *instance){
	int i=0;
	char temporaryString1[RX_PACKET_CONTENT_MAX_SIZE];
	//bitbangUARTmessage("!");
	if (esp8266_checkForRxPacket(instance, (temporaryString1)) == 0){
		//got new data
		//bitbangUARTmessage("&");




		//instance->rxPacketPointer[instance->rxPacketCount] = &(instance->rxPacketBuffer[instance->rxPacketBufferIndex]);

		//bitbangUARTmessage("New data to be added@");
		//bitbangUARThex(instance->rxPacketPointer[instance->rxPacketCount],0,8);
		//bitbangUARTmessage(";");
		//bitbangUARTint(instance->rxPacketCount,0,0);
		//bitbangUARTmessage(".\r\n");

		//copy it to packet buffer
		if(i >= RX_PACKET_CONTENT_MAX_SIZE-1 ) return 1; //getting creepily close.

#ifdef ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE
		bitbangUARTmessage("2.3)New data is:");
		bitbangUARTmessage(temporaryString1);
		bitbangUARTmessage(".\r\n");
#endif


		//after this is flaw
//		while((i <= RX_PACKET_CONTENT_MAX_SIZE-2) && (temporaryString1[i] != 0)){ //NB! This line assumes packet may not contain 0x00
//
//			if (i == (RX_PACKET_CONTENT_MAX_SIZE-1)){
//				esp8266_debugOutput("Received packet too big, crash imminent!? \r\n");
//				return 1;
//			}
//
//			instance->rxPacketBuffer[instance->rxPacketBufferIndex] = temporaryString1[i];
//			instance->rxPacketBufferIndex++;
//			i++;
//
//			//if need to go circular
//			if(instance->rxPacketBufferIndex >= instance->rxPacketBufferSize) instance->rxPacketBufferIndex = 0;
//		}
		//BEFORE THIS IS FLAW

		if(circularBuffer8_put_string(instance->rxPacketBuffer, temporaryString1)){
			bitbangUARTmessage("Failed to insert data to rxPacketBuffer\r\n");
			return 1; //something went wrong
		}
		if(circularBuffer8_put(instance->rxPacketBuffer, 0)){ //add null terminator too
			bitbangUARTmessage("Failed to insert data to rxPacketBuffer\r\n");
			return 1; //something went wrong
		}
		//bitbangUARTmessage("2.4)New data after move:");
		//bitbangUARTmessage(instance->rxPacketPointer[instance->rxPacketCount]);
		//bitbangUARTmessage(".\r\n");

		//instance->rxPacketBuffer[instance->rxPacketBufferIndex] = 0; //add null terminator
		//instance->rxPacketBufferIndex++;

		char *templol = instance->rxPacketBuffer->Buffer+instance->rxPacketBuffer->BufferReadIndex;

		instance->rxPacketCount++;
#ifdef ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE
		bitbangUARTmessage("Data in rxPacketBuffer\r\n");
#endif

		//bitbangUARTmessage("New data@");
		//bitbangUARThex(instance->rxPacketPointer[instance->rxPacketCount-1],0,8);
		//bitbangUARTmessage(";");
		//bitbangUARTint(instance->rxPacketCount,0,0);
		//bitbangUARTmessage(".\r\n");



	}
	//bitbangUARTmessage("'");
	return 0; //is OK
}

//
//bool esp8266_receiveHandler(esp8266_instance *instance){
//	return 1; //very broken
//
//	int rxPointerSize = sizeof(instance->oldestRxPointer);
//	char temporaryString1[rxPointerSize+RX_PACKET_CONTENT_MAX_SIZE];
//	int i = 0;
//	int *currentPacketPointer = 0;
//
//	if (esp8266_checkForRxPacket(instance, ((temporaryString1)+rxPointerSize))){
//		//we have new data!
//		instance->rxPackets += 1;
//		for(i=0; i<rxPointerSize; i++) temporaryString1[i] = 0; //set the "next packet address" to 0
//
//		//find newest packet without next packet ID (where it is 0)
//		while (*currentPacketPointer != 0) currentPacketPointer = *instance->oldestRxPointer;
//
//
//	}
//
//	//no new data but still all cool
//	return 0; //is OK
//
//}

int esp8266_getData(esp8266_instance *instance, char *data, uint16_t *length, uint8_t *id){ //gets oldest packet from rxPacketBuffer
	int i = 0;

	//check if any data available
	if (instance->rxPacketCount <= 0) return 1; //can't read what ain't there

	char temporaryString1[RX_PACKET_CONTENT_MAX_SIZE];

	int packetLen;
	char lengthString[4] = {0, 0, 0, 0}; //fill it with null terminators to avoid waiting for 1066 bytes instead of 10
	char idString[2] = {0, 0}; //fill it with null terminators

	do {
		if (circularBuffer8_get(instance->rxPacketBuffer, temporaryString1+i)){
			//received error. Buffer empty?
			//however, did we get end of string?
			if(temporaryString1[i] != 0){
				//something is wrong.
				instance->rxPacketCount--; //make sure everyone knows packet was read out
				return 1;
			}
		}
		i++;
	} while (temporaryString1[i-1] != 0);
	i=0;

	//read out oldest packet
	//note that packet format is: "<ID>,<length>:<data>"
	char *idStartPtr = temporaryString1;

#ifdef ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE
	bitbangUARTmessage("2.5)getData-data:");
	bitbangUARTmessage(idStartPtr);
	bitbangUARTmessage(".\r\n");
#endif

	//bitbangUARTmessage("rxPktPtr ");
	//bitbangUARThex(instance->rxPacketPointer[0],0,8);
	//bitbangUARTmessage("\r\n");


	char *idEndPtr = (char*)(strstr((idStartPtr), ",")); //gets first comma, after ID and before length

	char *lenStartPtr = idEndPtr+1;
	char *lenEndPtr = (char*)(strstr((lenStartPtr), ":")); //gets the colon, after length and before data

	//bitbangUARTmessage("Getdata strncpy ");
	//bitbangUARThex(lengthString,0,8);
	//bitbangUARTmessage(" ");
	//bitbangUARThex(lenStartPtr,0,8);
	//bitbangUARTmessage(" ");
	//bitbangUARThex(lenEndPtr,0,8);

	//bitbangUARTmessage("PktBuffInd:");
	//bitbangUARTint(instance->rxPacketBufferIndex,0,0);
	//bitbangUARTmessage(".\r\n");

	if((lenEndPtr == 0)||(lenStartPtr == 0)){
		//if this goes on, it hardfaults. Must be some broken packet.
		//also gotta "clear" this packet, otherwise I am stuck in endless loop

		if(lenEndPtr == 0) bitbangUARTmessage("Broken packet(end)!\r\n");
		if(lenStartPtr == 0) bitbangUARTmessage("Broken packet(start)!\r\n");


		bitbangUARTmessage("Reducing packet count. Prev:");
		bitbangUARTint(instance->rxPacketCount,0,1);

		instance->rxPacketCount--; //make sure everyone knows packet was read out

		bitbangUARTmessage(";now:");
		bitbangUARTint(instance->rxPacketCount,0,1);
		bitbangUARTmessage("\r\n");


		//adjust the pointers
		//i = 0;
		//while (i < instance->rxPacketCount){
		//	instance->rxPacketPointer[i] = instance->rxPacketPointer[i+1];
		//	i++;
		//	bitbangUARTmessage("BrokenPktAdjust.\r\n");
		//}
		//bitbangUARTmessage("\r\n");

		return 1;
	}

	strncpy(lengthString, lenStartPtr, lenEndPtr-lenStartPtr);
	//bitbangUARTmessage("\r\n");

	packetLen = (uint16_t)(atoi(lengthString));
	*id = (uint8_t)(atoi(idString));

	*length = packetLen;

	//char *packetStartPointer = instance->rxPacketPointer[0];
	char *packetStartPointer = lenEndPtr+1;


#ifdef ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE
	bitbangUARTmessage("3)precopydata:");
	bitbangUARTmessage(packetStartPointer);
	bitbangUARTmessage(".\r\n");
#endif

	//while((packetStartPointer[i] != 0) || (i<=packetLen)){//NB! This line assumes packet may not contain 0x00
	//	if((packetStartPointer + i) > (&instance->rxPacketBuffer[instance->rxPacketBufferSize] )) //going circular
	//		packetStartPointer -= (sizeof(instance->rxPacketBuffer[0])  * instance->rxPacketBufferSize); //reduce address by buffer size
	//	data[i] = *(packetStartPointer+i);
	//	i++;
	//}
	//data[packetLen] = 0; //null terminator!

	strcpy(data, packetStartPointer);

#ifdef ESP_RXPACKET_BBUARTDEBUGMESSAGES_ENABLE
	bitbangUARTmessage("4)postcopydata:");
	bitbangUARTmessage(data);
	bitbangUARTmessage(".\r\n");
#endif

	//bitbangUARTmessage("ESPGET:W TO ");
	//bitbangUARTint(j,0, 3);
	//bitbangUARTmessage("\r\n");

	instance->rxPacketCount--; //make sure everyone knows packet was read out
	//bitbangUARTmessage("Packet read out. New rxPacketCount: ");
	//bitbangUARTint(instance->rxPacketCount,0,2);
	//bitbangUARTmessage("\r\n");


	//adjust the pointers
	i = 0;
	//bitbangUARTmessage("i=");
	//bitbangUARTint(i,0, 0);
	//bitbangUARTmessage("\r\n");
//	while (i < instance->rxPacketCount){
//		//bitbangUARTmessage("i=");
//		//bitbangUARTint(i,0, 0);
//		//bitbangUARTmessage("\r\n");
//		instance->rxPacketPointer[i] = instance->rxPacketPointer[i+1];
//		i++;
//	}

	//bitbangUARTmessage("Pointer adjusted after readout. New rxPacketCount: ");
	//bitbangUARTint(instance->rxPacketCount,0,2);
	//bitbangUARTmessage("\r\n");

	return 0; //is OK
}



//int esp8266_getNetworkList(){
//
//	return 0; //very broken
//	return 1; //is OK
//}





//socket level stuff starts here

//bool esp8266_connLayer_init(esp8266_instance *instance, esp8266_connection_instance *conn, char *conntype, char *targetIP, uint16_t *targetPort){
//	conn->txBufferSize = TX_BUFFER_SIZE;
//	conn->rxBufferSize = RX_BUFFER_SIZE;
//	conn->rxBufferIndex = 0;
//	conn->txBufferIndex = 0;
//	conn->charsInRxBuffer = 0;
//	conn->charsInTxBuffer = 0;
//	conn->txBuffer[0] = 0;
//	conn->rxBuffer[0] = 0;
//	conn->owner = instance;
//	conn->deadBuffer = 0;
//
//	bool result;
//	result = esp8266_openConnection(instance, instance->openConnections, conntype, targetIP, targetPort);
//	if(result == 0){
//		//was success
//		instance->openConnections++;
//		return 0; //all good
//	}
//
//
//	return 1; //smth wrong
//}
//
//bool esp8266_connLayer_addSend(esp8266_connection_instance *conn, char *dataToSend){
//	int dataLength = strlen(dataToSend);
//	strcat(conn->txBuffer, dataToSend);
//	conn->txBufferIndex += dataLength;
//	conn->txBuffer[conn->txBufferIndex+1] = 0; //add 0 terminator cause I am paranoid
//	return 0; //all good
//	return 1; //smth wrong
//}
//
////THIS CONNLAYER STOPPED MAKING SENSE CAUSE I NEED TO REFER TO CONNECTION LAYERS SOMEHOW NOT ONLY ESP INSTANCE. ALL OF THEM. (cause RX)
////how about just enum the socket IDs?
//
//bool esp8266_connLayer_handleBuffers(esp8266_instance *instance){
//	int currentConn = 0;
//
//	//handle Rx according to ID
//
//	while (currentConn < instance->openConnections){
//		//do transfers according to connBuffers
//		//if(
//		currentConn++;
//	}
//}
