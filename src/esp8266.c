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

int esp8266_sendCommandAndWaitOK(char command[]){


	return 0; //very broken
	return 1; //is OK
}

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

	return 0; //very broken
	return 1; //is OK
}

//int esp8266_getNetworkList(){
//
//	return 0; //very broken
//	return 1; //is OK
//}

int esp8266_setCipmux(int isMultichannel){
	//if 1, then multiple connections
	//if 0, is single connection
	esp_8266_cipmux_latest = isMultiChannel;

	return 0; //very broken
	return 1; //is OK
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
	if(esp_8266_cipmux_latest == 0) ;
	else ;
	return 0; //very broken
	return 1; //is OK
}

int esp8266_openConnection(int id, char[4] type, char ip[16],int port){
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
