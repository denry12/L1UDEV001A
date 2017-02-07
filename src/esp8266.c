/*
 * esp8266.c
 *
 *  Created on: 26 Jan 2017
 *      Author: denry
 */


//https://cdn.sparkfun.com/datasheets/Wireless/WiFi/Command%20Doc.pdf //questionable content

//http://dominicm.com/esp8266-send-receive-data/
//http://www.espressif.com/sites/default/files/4b-esp8266_at_command_examples_en_v1.3.pdf

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




#define ESP8266_RETRY_COUNT 10
#define ESP8266_RETRY_DELAY 50

char esp_8266_cipmux_latest = 0; //only modify via special function



int esp8266_debugOutput(char message[]){
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


	lcd_5110_printAsConsole(message, 0);
	lcd_5110_redraw();
	return 1;
}

//HW specific code starts here
int esp8266_sendCommandAndWaitOK(char command[]){
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
	while(!(okResponse)){
			//okResponse = strstr(l11uxx_uart_rx_buffer, "OK\x0D\x0A");
			okResponse = strstr(&l11uxx_uart_rx_buffer, "OK");
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

int esp8266_sendCommandAndReadResponse(char command[]){


	return 0; //very broken
	return 1; //is OK
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
	if(isMultiChannel){
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

int esp8266_joinAP(char ssid[], char passwd[]){
	return 0; //very broken
	return 1; //is OK
}

int esp8266_leaveAP(char ssid[], char passwd[]){
	return 0; //very broken
	return 1; //is OK
}

int esp8266_getOwnIP(char *IPoutput){
	//AT+CIFSR

	return 0; //very broken
	return 1; //is OK
}


int esp8266_closeConnection(int id){
	//if(esp_8266_cipmux_latest == 0) ;
	//else ;
	return 0; //very broken
	return 1; //is OK
}

int esp8266_openConnection(int id, char type[4], char ip[16], int port){
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
