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

#define RX_BUFFER_SIZE 100
#define TX_BUFFER_SIZE 100

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
} esp8266_instance;

#endif /* ESP8266_H_ */
