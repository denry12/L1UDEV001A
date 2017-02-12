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

char esp_8266_cipmux_latest = 0; //only modify via special function


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



//THIS IS NOT USED.
int esp8266_sendCommandAndWaitOK_big(char *command){
	char *okResponse = 0;
	char retriesMax = 30;
	char retriesDone = 0;
	extern char *l11uxx_uart_rx_buffer;
	l11uxx_uart_clearRxBuffer(); //maybe unnecessary

	//send out most of command
	l11uxx_uart_Send(command);
	//clear buffer again to remove echo
	l11uxx_uart_clearRxBuffer();

	//finalize command
	l11uxx_uart_Send("\x0D\x0A");

	//esp8266_debugOutput("RxC\n\r");
	esp8266_debugOutput("Cmd:");
	esp8266_debugOutput(command);
	esp8266_debugOutput("\n\r");

	bitbangUARTmessage("CmdR:");
		bitbangUARTmessage(command);
		bitbangUARTmessage("\n\r");

	while(!(okResponse)){
			okResponse = strstr(&l11uxx_uart_rx_buffer, "OK\x0D\x0A");
			//okResponse = strstr(&l11uxx_uart_rx_buffer, "OK");
			l11uxx_uart_sendToBuffer();
			retriesDone++;
			if(retriesDone>=retriesMax) break;
			esp8266_debugOutput(".");
			delay(100);
	}
	esp8266_debugOutput("\n\r");
	//clear buffer again
	l11uxx_uart_clearRxBuffer();
	if(okResponse){
		esp8266_debugOutput("OK GET\n\r");
		return 1; //is OK
	}
	else {
		esp8266_debugOutput("FAIL\n\r");
		return 0; //very broken
	}

}



//"response" contains everything after \r\n after command echo
//and ends prior to "OK" (so ends with \r\n)
int esp8266_sendCommandAndReadResponse(char *command, char *response){ //with error, errorcode may be contained in "response". Not guaranteed.
	char *okResponse = 0;
	//char *response = 0;
	*response = 0;
	char retriesMax = 30;
	char retriesDone = 0;
	unsigned int debug= 0 ;
	unsigned int lengthOfResponse = 0;
	//char temporaryBuffer[100];
	int rxBufferLocalWaypoint;
	int i;
	extern char *l11uxx_uart_rx_buffer;
	volatile extern int l11uxx_uart_rx_buffer_current_index;
	l11uxx_uart_clearRxBuffer(); //maybe unnecessary
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 50\n\r");
	//send out most of command
	l11uxx_uart_Send(command);



	//clear buffer again to remove echo <- do not you dare, we use it for referencing now.
	//l11uxx_uart_clearRxBuffer();
	//bitbangUARTmessage("     Testline: 70\n\r");



	//finalize command
	delay(100); //without it rxcBufferLocalWaypoint is wrong :' ( EVEN WITH THAT IS!?
	//rxBufferLocalWaypoint=l11uxx_uart_rx_buffer_current_index; //keeps in mind where we actually start our response
	//rxBufferLocalWaypoint+=2; //"+2" stands for next line
	l11uxx_uart_Send("\x0D\x0A");


	//bitbangUARTmessage("     Testline: 100\n\r");
	//esp8266_debugOutput("RxC\n\r");
	l11uxx_uart_sendToBuffer(); //keep receiving
	esp8266_debugOutput("CmdR:");
	esp8266_debugOutput(command);
	esp8266_debugOutput("\n\r");


	l11uxx_uart_sendToBuffer(); //keep receiving
	//bitbangUARTmessage("     Testline: 150\n\r");
	//while(!(response)){
	//is last thing in UART buffer CR+LF?
	while((l11uxx_uart_rx_buffer_current_index < 2)){ //deffo not enough data
		l11uxx_uart_sendToBuffer(); //keep receiving
		retriesDone++;
		if(retriesDone>=retriesMax) break;
		bitbangUARTloadingbar(retriesDone, retriesMax-1);
		delay(100);
	}
	bitbangUARTloadingbar(retriesMax-1, retriesMax-1);
	esp8266_debugOutput("\n\r");

	//bitbangUARTmessage("     Testline: 180\n\r");
	//l11uxx_uart_spewBuffer();
	if(retriesDone>=retriesMax){
		esp8266_debugOutput("FAIL(A)\n\r");
		return 0; //very broken
	}
	//debug = l11uxx_uart_rx_buffer_current_index-2;
	//debug = &l11uxx_uart_rx_buffer[l11uxx_uart_rx_buffer_current_index-2];
	//strcpy(temporaryBuffer, &l11uxx_uart_rx_buffer);


	//l11uxx_uart_spewBuffer();
	//l11uxx_uart_sendToBuffer(); //POSSIBLY REMOVABLE?
	//l11uxx_uart_spewBuffer();


	//bitbangUARTmessage("     Testline: 190\n\r");
	//				while(((strcmp("\x0D\x0A",((l11uxx_uart_rx_buffer[l11uxx_uart_rx_buffer_current_index-2])))) != 0))
	//while(((strcmp(((&l11uxx_uart_rx_buffer[l11uxx_uart_rx_buffer_current_index-2])), "\x0D\x0A")) != 0))
	//while(((strcmp("\x0D\x0A",((&l11uxx_uart_rx_buffer + l11uxx_uart_rx_buffer_current_index-2)))) != 0))
	/*while(((strcmp(((&l11uxx_uart_rx_buffer + l11uxx_uart_rx_buffer_current_index-2)),"\x0A\x0D")) != 0)) { //when last two chars are not end of command by ESP
		//^ change this while to check if AD is there (there will be at least one
		//there must be another one somewhere after it
		//between these two, there is result.

		strcpy(temporaryBuffer, (&l11uxx_uart_rx_buffer+(l11uxx_uart_rx_buffer_current_index-l11uxx_uart_rx_buffer_current_index)));

		//bitbangUARTmessage("     Testline: 190\n\r");
		//bitbangUARTmessage((&l11uxx_uart_rx_buffer + l11uxx_uart_rx_buffer_current_index-2));
		//bitbangUARTmessage("     Testline: 200\n\r");
		l11uxx_uart_sendToBuffer(); //keep receiving
		retriesDone++;
		if(retriesDone>=retriesMax) break;
		esp8266_debugOutput(".");
		delay(100);
	}*/

	retriesDone=0;
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 200\n\r");
	while(!(okResponse)){
		//maybe error instead!? no point to wait then
		if(strstr(&l11uxx_uart_rx_buffer, "ERROR")){
			esp8266_debugOutput("FAIL(E)\n\r");
			strcpy(response, "ERROR");
			return 0; //very broken
		}
		if(strstr(&l11uxx_uart_rx_buffer, "FAIL")){
			esp8266_debugOutput("FAIL(F)\n\r");
			strcpy(response, "FAIL");
			return 0; //very broken
		}
		if(strstr(&l11uxx_uart_rx_buffer, "busy")){
			esp8266_debugOutput("FAIL(b)\n\r");
			strcpy(response, "busy");
			return 0; //very broken
		}
		//okResponse = strstr(&l11uxx_uart_rx_buffer, "OK\x0D\x0A");
		okResponse = strstr(&l11uxx_uart_rx_buffer, "OK");
		l11uxx_uart_sendToBuffer();
		retriesDone++;
		if(retriesDone>=retriesMax) break;
		bitbangUARTloadingbar(retriesDone, retriesMax-1);
		delay(100);
	}
	bitbangUARTloadingbar(retriesMax-1, retriesMax-1);
	esp8266_debugOutput("\n\r");
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 240\n\r");
	//l11uxx_uart_spewBuffer();
	if(retriesDone>=retriesMax){
			esp8266_debugOutput("FAIL(A)\n\r");
			return 0; //very broken
	}
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 245\n\r");
	//lengthOfResponse = okResponse; //DEBUG <- often 0x0000017b (including OK)
	//lengthOfResponse = (int)(&l11uxx_uart_rx_buffer); // <- gives nicely same as RxBSt
	lengthOfResponse = (int)(okResponse) - (int)(&l11uxx_uart_rx_buffer);
	lengthOfResponse -= 2; //remove "OK"
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 250\n\r");


	//get string to buffer. not sure why this is necessary, but didn't work when I changed "0" to anything else
	strncpy(response, (&l11uxx_uart_rx_buffer+0), lengthOfResponse);
	//bitbangUARTmessage("     Testline: 252\n\r");
	//cut string
	rxBufferLocalWaypoint=strlen(command);
	strncpy(response, (&l11uxx_uart_rx_buffer+0), lengthOfResponse); // REMOVE THIS, IT IS DOUBLED!?
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 255\n\r");
	while((response[rxBufferLocalWaypoint] == '\r') || (response[rxBufferLocalWaypoint] == '\n')) rxBufferLocalWaypoint++;
	for (i=0; i<lengthOfResponse; i++) response[i] = response[i+rxBufferLocalWaypoint];
	response[lengthOfResponse-rxBufferLocalWaypoint] = 0; //add null terminator to be sure. Likely not necessary but I have trust issues.


	//strcpy(response, temporaryBuffer);


	//l11uxx_uart_sendToBuffer(); //DEBUG ONLY, REMOVE!!!
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 260\n\r");

	//if(retriesDone>=retriesMax){
	//	esp8266_debugOutput("FAIL(B)\n\r");
	//	return 0; //very broken
	//}
	//esp8266_debugOutput("\n\r");
	//bitbangUARTmessage("     Testline: 290\n\r");
	//bitbangUARTmessage("     Testline: 300\n\r");
	//}
	//esp8266_debugOutput("\n\r");



	//this causes occasional "FF""US" crap that "clears" screen
	//esp8266_debugOutput("R: ");
	//esp8266_debugOutput(&response);
	//bitbangUARTmessage(&response);


	//bitbangUARTmessage("     Testline: 310\n\r");
	if(debug_testline_messages_SCARR) bitbangUARTmessage("     Testline: 320\n\r");

	//clear buffer again
	l11uxx_uart_clearRxBuffer();

	//if you made it this far, all OK!
	bitbangUARTmessage("CMD ok, returning\r\n");
	return 1; //is OK
	return 0; //very broken

}

int esp8266_sendCommandAndWaitOK(char *command){
	char response[80]; //this could be written anywhere, it is never read
	//80 chars seems to be enough to fit any crap it may output

	if(esp8266_sendCommandAndReadResponse(command, response)){ //this "response" is discarded, so it should be written to somewhere where nobody cares
		//esp8266_debugOutput("\r\n"); //cause I don't get newline with this
		return 1; //is OK
	}
	//this fail is actually outputted in sendandread function as well
	//esp8266_debugOutput("FAIL\r\n"); //cause I don't get newline with this
	return 0; //very broken
}


int esp8266_checkForRxPacket(){

}

//HW specific code ends here

int esp8266_isAlive(){
	if(esp8266_sendCommandAndWaitOK("AT")) return 1; //is OK;
	return 0; //very broken

}

int esp8266_SWreset(){
	esp8266_sendCommandAndWaitOK("AT+RST");
	return 0; //very broken
	return 1; //is OK
}

int esp8266_setUARTMode(int baudrate, char bits, char parity, char flowControl){
	char modeConfString[40];
	strcpy(modeConfString,"AT+UART_CUR=");

	//TODO: make this part universal
	if(baudrate==9600)strcat(modeConfString,"9600");
	else if(baudrate==19200)strcat(modeConfString,"19200");
	else if(baudrate==115200)strcat(modeConfString,"115200");
	strcat(modeConfString,",");
	strcat(modeConfString,"8,2,0");

	//ATTENTION:
	//As of 9th of February, 2017
	//setting "Flow Control" to "0" (disable)
	//ESP8266 heats up a lot, eventually causing
	//voltage drop to reset MCU
	//
	//Using "1" (RTS) confuses poor LPC
	//Using "2" (CTS) works well
	//Using "3" probably confuses, cause has RTS.
	strcat(modeConfString,",1"); //keep it as 1, unless you know why and what you are doing

	bitbangUARTmessage("Doing CMD for UARTspeed\r\n");
	if(esp8266_sendCommandAndWaitOK(modeConfString)){
		bitbangUARTmessage("UARTSpeed CMD success\r\n");
		return 1; //is OK;
	}
	bitbangUARTmessage("UARTSpeed CMD fail\r\n");
	return 0; //very broken

}

//int esp8266_HWreset()

int esp8266_setMode(int mode){
	//1 = sta <- this is client
	//2 = AP <- this is "router", but without DHCP
	//3 = both
	switch(mode){
		case 1:
			if(esp8266_sendCommandAndWaitOK("AT+CWMODE=1")) return 1; //is OK
			break;
		case 2:
			if(esp8266_sendCommandAndWaitOK("AT+CWMODE=2")) return 1; //is OK
			break;
		default:
			if(esp8266_sendCommandAndWaitOK("AT+CWMODE=3")) return 1; //is OK
			break;
	}
	return 0; //very broken
}

//int esp8266_getNetworkList(){
//
//	return 0; //very broken
//	return 1; //is OK
//}

int esp8266_setCipmux(int isMultichannel){
	//if 1, then multiple connections
	//if 0, is single connection
	if(isMultichannel){
		if(esp8266_sendCommandAndWaitOK("AT+CIPMUX=1")){
			esp_8266_cipmux_latest = isMultichannel; //updated only if all went well
			return 1; //is OK
		}
	} else {
		if(esp8266_sendCommandAndWaitOK("AT+CIPMUX=0")){
			esp_8266_cipmux_latest = isMultichannel; //updated only if all went well
			return 1; //is OK
		}
	}
	return 0; //very broken

}

int esp8266_joinAP(char *ssid, char *passwd){
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

	if(esp8266_sendCommandAndWaitOK(modeConfString)) return 1; //is OK;
	//likely timed out
	//check status
	strcpy(modeConfString,"");
	while(retryCounter<maxRetries){
		if(esp8266_sendCommandAndReadResponse("AT+CIPSTATUS", modeConfString)){
			//bitbangUARTmessage("\n\r!!");
			bitbangUARTmessage(modeConfString);
			//bitbangUARTmessage("!!\n\r");
			if(strstr(modeConfString, "STATUS:2")){
				bitbangUARTmessage("Wifi connected succesfully!!\n\r");
				return 1;
			} else {
				//gets OK response but does not contain STATUS:"
				bitbangUARTmessage("Wifi connect failed.\n\r");
				return 0; //very broken
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


	return 0; //very broken
}

int esp8266_leaveAP(){
	//AT+CWQAP
	bitbangUARTmessage("Ditching AP\n\r");
	if(esp8266_sendCommandAndWaitOK("AT+CWQAP")){
		bitbangUARTmessage("Ditched AP successfully\n\r");
		return 1; //is OK
	}
	bitbangUARTmessage("AP ditch fail\n\r");
	return 0; //very broken

}

int esp8266_getOwnIP(char *IPoutput){
	//AT+CIFSR
	//volatile char modeConfString[80];
	//int debug=0;
	char *stringCutPointer;
	bitbangUARTmessage("Getting IP\n\r");
	if(esp8266_sendCommandAndReadResponse("AT+CIFSR", IPoutput)){

		//currently has +CIRFSR:STAIP,"xxx.xxx.xxx.xxx"\r\n+CIFSR:STAMAC,"ab:cd:ab:cd:ab:cd"\r\n"
		stringCutPointer = strstr(IPoutput, "+CIFSR:STAIP,\x22");
		stringCutPointer = stringCutPointer + strlen("+CIFSR:STAIP,\x22");
		memmove(IPoutput, stringCutPointer, strlen(IPoutput)+1);  //currently has xxx.xxx.xxx.xxx"\r\n+CIFSR:STAMAC,"ab:cd:ab:cd:ab:cd"\r\n"

		stringCutPointer = strstr(IPoutput, "\x22"); 	//find next " symbol
		if(!(stringCutPointer)){
					bitbangUARTmessage("IP string not found\n\r");
					bitbangUARTmessage("No IP 4 U\n\r");
					return 0; //very broken
				}
		stringCutPointer[0] = 0; 						//and slam a null terminator there
														//NB! No \r\n

		bitbangUARTmessage("Looks like IP is: ");
		//debug = (int)(&IPoutput);
		bitbangUARTmessage((IPoutput));
		bitbangUARTmessage("\n\r");
		return 1; //is OK
	}
	bitbangUARTmessage("No IP 4 U\n\r");
	return 0; //very broken
}


int esp8266_getOwnMAC(char *IPoutput){
	//AT+CIFSR
	//volatile char modeConfString[80];
	//int debug=0;
	char *stringCutPointer;
	bitbangUARTmessage("Getting MAC\n\r");
	if(esp8266_sendCommandAndReadResponse("AT+CIFSR", IPoutput)){

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
			return 0; //very broken
		}
		//bitbangUARTmessage("4");
		stringCutPointer[0] = 0; 						//and slam a null terminator there
														//NB! No \r\n
		//bitbangUARTmessage("5\n\r");
		bitbangUARTmessage("Looks like MAC is: ");
		//debug = (int)(&IPoutput);
		bitbangUARTmessage((IPoutput));
		bitbangUARTmessage("\n\r");
		return 1; //is OK
	}
	bitbangUARTmessage("No MAC 4 U\n\r");
	return 0; //very broken
}

int esp8266_closeConnection(int id){
	//if(esp_8266_cipmux_latest == 0) ;
	//else ;
	return 0; //very broken
	return 1; //is OK
}

int esp8266_openConnection(int *id, char *type, char *ip, int *port){
	char modeConfString[40]; // should be enough


		strcpy(modeConfString,"AT+CIPSTART=");
		if(esp_8266_cipmux_latest == 1)strcat(modeConfString,"0,");

		strcat(modeConfString,"8,2,0");
	return 0; //very broken
	return 1; //is OK
}

int esp8266_sendData(int id, int length, char data[]){

	return 0; //very broken
	return 1; //is OK
}

int esp8266_getData(int id, int length, char data[]){

	return 0; //very broken
	return 1; //is OK
}

//int esp8266_getNetworkList(){
//
//	return 0; //very broken
//	return 1; //is OK
//}
