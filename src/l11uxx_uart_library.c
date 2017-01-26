/*
 * UART11U35.c
 *
 *  Created on: 30.05.2016
 *      Author: Denry
 */
//#include "UART11U35.h"

#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif

#define L11UXX_UART_RX_BUFFER_LEN 200

char l11uxx_uart_rx_buffer[L11UXX_UART_RX_BUFFER_LEN];
int l11uxx_uart_rx_buffer_current_index = 0;

void l11uxx_uart_pinSetup_unset(int pin){ //uses physical pin numbers
	//if(UARTNumberNumber == 0){
		//setting up Tx
			if (pin == 47) LPC_IOCON->PIO0_19 &= (~0x07); //p0_19
			else if (pin == 12) LPC_IOCON->PIO1_27 &= (~0x07); //p1_27
			else if (pin == 36) LPC_IOCON->PIO1_13 &= (~0x07); //p1_13

		//setting up Rx

			else if (pin == 46) LPC_IOCON->PIO0_18 &= (~0x07); //p0_18
			else if (pin == 11) LPC_IOCON->PIO1_26 &= (~0x07); //p1_26
			else if (pin == 37) LPC_IOCON->PIO1_14 &= (~0x07); //p1_14

		//}

	return;
}

void l11uxx_uart_pinSetup(int txPin, int rxPin){ //uses physical pin numbers
	l11uxx_uart_pinSetup_unset(txPin);
	l11uxx_uart_pinSetup_unset(rxPin);
	//if(UARTNumberNumber == 0){
		//setting up Tx
			if (txPin == 47) LPC_IOCON->PIO0_19 |= 0x1; //p0_19
			else if (txPin == 12) LPC_IOCON->PIO1_27 |= 0x2; //p1_27
			else if (txPin == 36) LPC_IOCON->PIO1_13 |= 0x3; //p1_13

		//setting up Rx

			if (rxPin == 46) LPC_IOCON->PIO0_18 |= 0x1; //p0_18
			else if (rxPin == 11) LPC_IOCON->PIO1_26 |= 0x2; //p1_26
			else if (rxPin == 37) LPC_IOCON->PIO1_14 |= 0x3; //p1_14

		//}

	return;
}



int l11uxx_uart_init(uint32_t baudrate){
	//LPC_IOCON->PIO1_7|=0x1; //47 - TX to mating part
	//LPC_IOCON->PIO1_6|=0x1; //46 - RX from mating part

	uint32_t  DL;

	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
	LPC_SYSCON->UARTCLKDIV = 0x01;        //UART clock
	DL = (SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV) / (16 * baudrate * LPC_SYSCON->UARTCLKDIV);



	//char buffer[10];
	    //itoa(DL, buffer, 10);
	    //printf(buffer);
	    //printf(";");

	    DL = ((DL*125)+20) / 128;  //this is necessary, maybe because internal clock is so shit?

		    //itoa(DL, buffer, 10);
		    //printf(buffer);





	//let's hardcode this to 115200
	//DL=37; //yeah, at 48MHz maybe?, almost 19200 w/o xtal
	//DL=6; //115200 w/o xtal, 12MHz, works well
	//DL=76; //9600 w/o xtal, 12MHz,
	LPC_USART->FDR = (12 << 4) | 1; //DivAddVal = 1, MulVal = 12

	LPC_USART->LCR = 0b10000011;  //  8-bit character length, DLAB enable ( DLAB = 1)
	LPC_USART->DLM = DL / 256;    //  Determines the baud rate of the UART (MSB Register)     (Access DLAB = 1)
	LPC_USART->DLL = DL % 256;    //  Determines the baud rate of the UART (LSB Register)     (Access DLAB = 1)
	LPC_USART->LCR = 0b00000011;  //  8-bit character length , DLAB disable ( DLAB = 0)
	LPC_USART->FCR = 0b00000111;  //  FIFOEN:Active high enable for both UART Rx and TX FIFOs and U0FCR[7:1] access
	//  RXFIFORES:Writing a logic 1 to U0FCR[1] will clear all bytes in UART Rx FIFO, reset the pointer logic.
	//  TXFIFORES:Writing a logic 1 to U0FCR[2] will clear all bytes in UART TX FIFO, reset the pointer logic.

	//RX interrupt setup
	LPC_USART->IER = (1<<2); //RXLIE - RX Line Interrupt Enable
	LPC_USART->FCR |= (0x3<<6); //trigger level 3, 14 characters in RX buffer
	NVIC_EnableIRQ(UART_IRQn);

	return 1;
}


//int wifiSend(char *text){
//UARTSend(text);

//int broadcastData(char *data){
//if(wifiSend(data))


//MAIN
//char broadcastMessage[80]="RAW-DATA-UNINITIALIZED.";
//broadcastData(broadcastMessage);

int l11uxx_uart_Send(char text[]){
	int i=0;
	//printf("SENDING!\n");

	//printf(&text[i]); //shows whole message
	while(text[i] != 0){
		while (!(LPC_USART->LSR & (1<<5)));  //THRE: Transmitter Holding Register Empty.
			                                        // 0: THR contains valid data, 1: THR is empty
		LPC_USART->THR = text[i];
		i++;
	}
//	return 0; //something failed
	return 1; //everything went OK
}

void l11uxx_uart_sendToBuffer(){ //This function drains the HW UART Rx buffer to SW buffer
	while ((LPC_USART->LSR & 0x01)){//while data available
			*(l11uxx_uart_rx_buffer+l11uxx_uart_rx_buffer_current_index) = LPC_USART->RBR;
			//if(!(*(saveTo2+l11uxx_uart_rx_buffer_current_index)==0))l11uxx_uart_rx_buffer_current_index++; //I don't want array filled with zeros.
			if(!((LPC_USART->LSR & 0x01))){ //oh no, are we out of data?
				//let's wait, maybe slave is slow
				delay(1); //uncomment if delay function available
			}
			if(l11uxx_uart_rx_buffer_current_index<L11UXX_UART_RX_BUFFER_LEN) l11uxx_uart_rx_buffer_current_index++;
			//else; //dang, we're full
		}
	*(l11uxx_uart_rx_buffer+l11uxx_uart_rx_buffer_current_index+1)=0; //cause I want the string to end here
	return;
}

void l11uxx_uart_clearRxBuffer(){
	l11uxx_uart_rx_buffer_current_index=0;
	return;
}

void USART_IRQHandler(void){
//	debugMessage("UARTInterrupt fired!"); //this may cause me to lose data.
	l11uxx_uart_sendToBuffer();
}
