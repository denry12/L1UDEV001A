/*
 * esp8266.h
 *
 *  Created on: 19.04.2017
 *      Author: Denry
 */

#ifndef ESP8266_H_
#define ESP8266_H_
#ifdef __USE_CMSIS
#include "LPC11Uxx.h"			/* LPC11Uxx Peripheral Registers */
#endif
#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>

#define RX_BUFFER_SIZE 500
#define TX_BUFFER_SIZE 500


//#define CONN_RX_BUFFER_SIZE 50
//#define CONN_TX_BUFFER_SIZE 50

#define RX_PACKET_CONTENT_MAX_SIZE 50
#define RX_PACKET_MAX_COUNT 10

typedef enum{
	ESP8266_STATE_IDLE,
	ESP8266_STATE_TXNEEDED,
	ESP8266_STATE_WAITINGRESPONSE


}esp8266_state;

//typedef enum{
//	IDLE_CONN,
//	TXNEEDED_CONN
//
//}esp8266_connectionstate;


typedef struct {
	int rxBufferSize;
	char receivedFromESPbuffer[RX_BUFFER_SIZE];
	int rxCircBufferIndex;
	int charactersInRxBuffer;
	int txBufferSize;
	char sendToESPbuffer[TX_BUFFER_SIZE];
	int txCircBufferIndex;
	int charactersInTxBuffer;
	bool (*getCharFromESP)();
	bool (*sendCharToESP)();
	esp8266_state currentstate;
	char cipmux_latest;
	int openConnections;
	//char *oldestRxPointer; //pointer to oldest packet in Rx packet FIFO
	char rxPacketBuffer[RX_PACKET_CONTENT_MAX_SIZE*RX_PACKET_MAX_COUNT];
	int rxPacketBufferSize;
	int rxPacketBufferIndex;
	char *rxPacketPointer[RX_PACKET_MAX_COUNT];
	int rxPacketCount;
} esp8266_instance;

//typedef struct{
//	esp8266_instance *owner;
//	char txBuffer[CONN_TX_BUFFER_SIZE];
//	char rxBuffer[CONN_RX_BUFFER_SIZE];
//	int txBufferSize;
//	int rxBufferSize;
//	int charsInRxBuffer;
//	int charsInTxBuffer;
//	int txBufferIndex;
//	int rxBufferIndex;
//	char socketType[5]; //UDP or TCP
//	bool deadBuffer; //set to 1 when connection closed
//	esp8266_connectionstate state;
//} esp8266_connection_instance;

#endif /* ESP8266_H_ */
