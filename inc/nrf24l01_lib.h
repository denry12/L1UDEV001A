#ifndef NRF24L01_LIB_H
#define NRF24L01_LIB_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	//circularBuffer_8bit *receivedFromESPbuffer;
	//circularBuffer_8bit *sendToESPbuffer;
	bool (*sendSPIpacket)();
	bool (*getSPIpacket)();
	bool (*flushSPIrxBuffer)();
	bool (*enableCSN)(); //sets CSN voltage low
	bool (*disableCSN)(); //sets CSN voltage high
	bool (*enableCE)(); //sets CE voltage high
	bool (*disableCE)(); //sets CE voltage low

	uint8_t payload_len[6]; // value for each pipe

	//nrf24l01_state currentstate;
	//int openConnections;
	//char *oldestRxPointer; //pointer to oldest packet in Rx packet FIFO
	//circularBuffer_8bit *rxPacketBuffer;
	//char rxPacketBuffer[RX_PACKET_CONTENT_MAX_SIZE*RX_PACKET_MAX_COUNT];
	//int rxPacketBufferSize;
	//int rxPacketBufferIndex;
	//char *rxPacketPointer[RX_PACKET_MAX_COUNT];
	//int rxPacketCount;
} nrf24l01_instance;

#endif
