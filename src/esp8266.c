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
#include "bitbangUART.h"

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
	instance->rxBufferSize = RX_BUFFER_SIZE;
	instance->rxCircBufferIndex = 0;
	instance->charactersInRxBuffer = 0;
	instance->receivedFromESPbuffer[0] = 0;
	instance->receivedFromESPbuffer[1] = 0; //hopefully not necessary
	return 0;
}

bool esp8266_resetTxBuffer(esp8266_instance *instance){
	instance->txBufferSize = TX_BUFFER_SIZE;
	instance->txCircBufferIndex = 0;
	instance->charactersInTxBuffer = 0;
	instance->sendToESPbuffer[0] = 0;
	instance->sendToESPbuffer[1] = 0; //hopefully not necessary
	return 0;
}

bool esp8266_initalize(esp8266_instance *instance){
	esp8266_resetRxBuffer(instance);
	esp8266_resetTxBuffer(instance);
	instance->currentstate = ESP8266_STATE_IDLE;
	instance->rxPacketCount = 0;
	instance->rxPacketBufferSize = RX_PACKET_CONTENT_MAX_SIZE*RX_PACKET_MAX_COUNT;
	instance->rxPacketBufferIndex = 0;
	instance->rxPacketBuffer[0] = 0;
	instance->rxPacketBuffer[1] = 0; //hopefully not necessary
	return 0; //all OK
}

bool esp8266_charFromUartToBuffer(esp8266_instance *instance, char character){
	//is there room?
	if((instance->charactersInRxBuffer) >= (instance->rxBufferSize))
		return 1; //no room.

	instance->receivedFromESPbuffer[instance->rxCircBufferIndex] = character;
	instance->receivedFromESPbuffer[instance->rxCircBufferIndex+1] = 0; //add null terminator
	instance->rxCircBufferIndex++;

	//go circular if necessary
	if(instance->rxCircBufferIndex > instance->rxBufferSize) instance->rxCircBufferIndex = 0;
	instance->charactersInRxBuffer++;

	return 0; //all OK
}

int16_t esp8266_charFromBufferToUart(esp8266_instance *instance){

	//is there anything in buffer?
	if(instance->charactersInTxBuffer == 0)
		return -1; //o no

	char character;
	int index = (instance->txCircBufferIndex) - (instance->charactersInTxBuffer);
	if(index < 0)
		index += instance->txBufferSize;
	character = instance->sendToESPbuffer[index];
	instance->charactersInTxBuffer--;

	return character; //all OK
}

bool esp8266_sendString(esp8266_instance *instance, char *command){
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
	char *okResponse = 0;

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
	 rxBufferLocalWaypoint=instance->rxCircBufferIndex; //keeps in mind where we actually start our response

	 //from this point on, I know what I am sending out. Just add it to the localwaypoint value
	 rxBufferLocalWaypoint += (strlen(command));
	 rxBufferLocalWaypoint += 2; //add \r\n

	 //rxBufferLocalWaypoint += 1; //otherwise a wild extra \r (0x0D) appears. Gotta investigate later maybe - not issue tho


		//finalize command
	esp8266_sendString(instance, "\x0D\x0A"); //this is what ESP responds as well. \r\n (checked at 21.04.2017)

	while(instance->charactersInTxBuffer){ //do the actual sending from ESP buffer to UART

		(*instance).sendCharToESP(instance);
	}


	esp8266_debugOutput("CmdR:");
	esp8266_debugOutput(command);
	esp8266_debugOutput("\n\r");

	while(instance->charactersInRxBuffer < 2){ //deffo not enough data
		(*instance).getCharFromESP(instance);
		retriesDone++;
		if(retriesDone>=retriesMax) break;
		bitbangUARTloadingbar(retriesDone, retriesMax-1);
		bitbangUARTmessage(" 1/2  ");
		delay(100);
	}

	//bool readDataReturnValue = (*instance).getCharFromESP(instance);

	while((*instance).getCharFromESP(instance) == 0);//{ //and if data still keeps on coming

	bitbangUARTloadingbar(retriesMax-1, retriesMax-1);



	if(retriesDone>=retriesMax){
		esp8266_debugOutput("FAIL(A)\n\r");
		esp8266_debugOutput("\n\r");
		return 0; //very broken
	}


	retriesDone=0;
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 200\n\r");
	while(!(okResponse)){
		(*instance).getCharFromESP(instance);
		//maybe error instead!? no point to wait then
		if(strstr(&instance->receivedFromESPbuffer, "ERROR")){
			esp8266_debugOutput("FAIL(E)\n\r");
			strcpy(response, "ERROR");
			return 0; //very broken
		}
		if(strstr(&instance->receivedFromESPbuffer, "FAIL")){
			esp8266_debugOutput("FAIL(F)\n\r");
			strcpy(response, "FAIL");
			return 0; //very broken
		}
		if(strstr(&instance->receivedFromESPbuffer, "busy")){
			esp8266_debugOutput("FAIL(b)\n\r");
			strcpy(response, "busy");
			return 0; //very broken
		}
		okResponse = strstr(&(instance->receivedFromESPbuffer) + instance->rxCircBufferIndex - instance->charactersInRxBuffer, "OK"); //gives memory address to last OK (start)

		//this should contain OK as well
		//THIS HERE IS USEFUL DEBUG
		//bitbangUARTmessage("What I think I received: ");
		//bitbangUARTmessage(instance->receivedFromESPbuffer + rxBufferLocalWaypoint);

		retriesDone++;
		if(retriesDone>=retriesMax) break;
		bitbangUARTloadingbar(retriesDone, retriesMax-1);
		bitbangUARTmessage(" 2/2");
		delay(100);
	}
	bitbangUARTloadingbar(retriesMax-1, retriesMax-1);
	esp8266_debugOutput("\n\r");
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 240\n\r");

	if(retriesDone>=retriesMax){
			esp8266_debugOutput("FAIL(A)\n\r");
			return 0; //very broken
	}
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 245\n\r");
	//lengthOfResponse = okResponse; //DEBUG <- often 0x0000017b (including OK)
	//lengthOfResponse = (int)(&l11uxx_uart_rx_buffer); // <- gives nicely same as RxBSt
	//lengthOfResponse = (int)(okResponse) - (int)(&instance->receivedFromESPbuffer);
	lengthOfResponse = (instance->rxCircBufferIndex - rxBufferLocalWaypoint);
	lengthOfResponse -= 5; //stop from copying last two characetrs ("OK"+\r\n\x00)
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 250\n\r");
	//bitbangUARTmessage((&instance->receivedFromESPbuffer) + instance->rxCircBufferIndex - instance->charactersInRxBuffer);// + instance->rxCircBufferIndex);

	//"response" shall contain string between \r\n from sent command echo to OK
	strncpy(response, instance->receivedFromESPbuffer + rxBufferLocalWaypoint, lengthOfResponse);



	//by now "response" should be formatted
	//THIS HERE IS USEFUL DEBUG
	//bitbangUARTmessage("Got response: ");
	//bitbangUARTmessage(response);


	//cut string
	rxBufferLocalWaypoint=strlen(command);

	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 255\n\r");
	rxBufferLocalWaypoint = 0; //cleared, cause now we are looking through "response", not buffer
	while((response[rxBufferLocalWaypoint] == '\r') || (response[rxBufferLocalWaypoint] == '\n')) rxBufferLocalWaypoint++;
	for (i=0; i<lengthOfResponse; i++) response[i] = response[i+rxBufferLocalWaypoint];
	response[lengthOfResponse-rxBufferLocalWaypoint] = 0; //add null terminator to be sure. Likely not necessary but I have trust issues.


	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 260\n\r");

	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 320\n\r");

	//if you made it this far, all OK!
	bitbangUARTmessage("CMD ok, returning\r\n");
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


bool esp8266_checkForRxPacket(esp8266_instance *instance, char *response){
	esp8266_resetRxBuffer(instance);
	uint16_t packetLen = 0;
	uint16_t bytesReceived = 0;
	char lengthString[4] = {0, 0, 0, 0}; //fill it with null terminators to avoid waiting for 1066 bytes instead of 10
	while((*instance).getCharFromESP(instance) == 0); //and if data still keeps on coming
	char *responsePtr = 0;
	char *lenStartPtr = 0;
	char *lenEndPtr = 0;
	responsePtr = strstr(&(instance->receivedFromESPbuffer) + instance->rxCircBufferIndex - instance->charactersInRxBuffer, "+IPD,"); //gives memory address to last +IPD, (start)

	if (responsePtr == 0) return 1; //no data :' (

	lenStartPtr = strstr((responsePtr), ","); //gets first comma from "+IPD,"
	lenStartPtr = strstr((lenStartPtr+1), ","); //gets second comma from "+IPD,<id>,"
	lenStartPtr += 1; //go past the second comma, this is where length actually starts
	lenEndPtr = strstr(lenStartPtr, ":");

	strncpy(lengthString, lenStartPtr, lenEndPtr-lenStartPtr);
	packetLen = atoi(lengthString);

	//while((*instance).getCharFromESP(instance) == 0); //and if data still keeps on coming
	while(bytesReceived <= packetLen){
		bytesReceived = (&(instance->receivedFromESPbuffer[instance->rxCircBufferIndex]) - lenEndPtr);
		(*instance).getCharFromESP(instance);
	}

	esp8266_debugOutput("Getting data:");
	esp8266_debugOutput(responsePtr);
	esp8266_debugOutput("\r\n");

	memmove(response, responsePtr, strlen(responsePtr)+1); //probably could be optimized to remove this line

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
			if(esp8266_sendCommandAndWaitOK(instance, "AT+CWMODE=1")) return 1; //is OK
			break;
		case 2:
			if(esp8266_sendCommandAndWaitOK(instance, "AT+CWMODE=2")) return 1; //is OK
			break;
		default:
			if(esp8266_sendCommandAndWaitOK(instance, "AT+CWMODE=3")) return 1; //is OK
			break;
	}
	return 0; //very broken
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
		if(esp8266_sendCommandAndReadResponse(instance, "AT+CIPSTATUS", modeConfString)){
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

	return 1; //very broken
	return 0; //is OK
}

bool esp8266_receiveHandler(esp8266_instance *instance){
	int i=0;
	char temporaryString1[RX_PACKET_CONTENT_MAX_SIZE];
	if (esp8266_checkForRxPacket(instance, (temporaryString1)) == 0){
		//got new data
		instance->rxPacketPointer[instance->rxPacketCount] = &(instance->rxPacketBuffer[instance->rxPacketBufferIndex]);

		//copy it to packet buffer
		while(temporaryString1[i] != 0){ //NB! This line assumes packet may not contain 0x00
			instance->rxPacketBuffer[instance->rxPacketBufferIndex] = temporaryString1[i];
			instance->rxPacketBufferIndex++;
			i++;

			//if need to go circular
			if(instance->rxPacketBufferIndex > instance->rxPacketBufferSize) instance->rxPacketBufferIndex = 0;
		}
		instance->rxPacketCount++;
	}

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

int esp8266_getData(esp8266_instance *instance, char *data){ //gets oldest packet from rxPacketBuffer
	int i = 0;

	//check if any data available
	if (instance->rxPacketCount == 0) return 1; //can't read what ain't there


	//read out oldest packet
	char *packetStartPointer = instance->rxPacketPointer[0];
	while(instance->rxPacketPointer[i] != 0){//NB! This line assumes packet may not contain 0x00
		if((packetStartPointer + i) > (&instance->rxPacketBuffer[instance->rxPacketBufferSize] )) //going circular
			packetStartPointer -= sizeof(instance->rxPacketBuffer[0] * instance->rxPacketBufferSize); //reduce address by buffer size
		data[i] = *(packetStartPointer+i);
		i++;
	}

	instance->rxPacketCount--; //make sure everyone knows packet was read out

	//adjust the pointers
	i = 0;
	while (i < instance->rxPacketCount){
		instance->rxPacketBuffer[i] = instance->rxPacketBuffer[i+1];
		i++;
	}


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
