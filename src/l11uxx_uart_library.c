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

#include "l11uxx_uart_library.h"



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
	char debugChar;

	//char temporaryChar[2];
	//temporaryChar[1] = 0;
	while ((LPC_USART->LSR & 0x01)){//while data available

		//LPC_GPIO->SET[0] = (0x20000); //0_17
		NVIC_DisableIRQ(UART_IRQn);
		rxBusy=1;
		*(l11uxx_uart_rx_buffer+l11uxx_uart_rx_buffer_current_index) = LPC_USART->RBR;
		//LPC_GPIO->CLR[0] = (0x20000); //0_17
		//debugChar = *(l11uxx_uart_rx_buffer+l11uxx_uart_rx_buffer_current_index);

		//bitbangUARTmessage("NewUARTByte: ");
		//temporaryChar[0] = l11uxx_uart_rx_buffer[l11uxx_uart_rx_buffer_current_index];
		//if(temporaryChar[0] == '\n') temporaryChar[0] = 'n';
		//if(temporaryChar[0] == '\r') temporaryChar[0] = 'r';
		//bitbangUARTmessage(temporaryChar);
		//bitbangUARTmessage("\n\r");
		//if(!(*(saveTo2+l11uxx_uart_rx_buffer_current_index)==0))l11uxx_uart_rx_buffer_current_index++; //I don't want array filled with zeros.
		if(l11uxx_uart_rx_buffer_current_index<(L11UXX_UART_RX_BUFFER_LEN-1)) l11uxx_uart_rx_buffer_current_index++;
		else bitbangUARTmessage("UARTBUFFERFULL!\n\r"); //dang, we're full
		rxBusy=0;
		NVIC_EnableIRQ(UART_IRQn);
		//if(!((LPC_USART->LSR & 0x01))){ //oh no, are we out of data?
			//let's wait, maybe slave is slow
			//delay(10); //uncomment if delay function available <- please note that 10ms is radical delay even for 9600baud
		//}

	}

	*(l11uxx_uart_rx_buffer+l11uxx_uart_rx_buffer_current_index+0)=0; //cause I want the string to end here

	return;
}

void l11uxx_uart_clearRxBuffer_withoutReadout(){
	l11uxx_uart_rx_buffer_current_index=0;
	l11uxx_uart_rx_buffer[0]=0;
	//l11uxx_uart_rx_buffer[1]=0;
	return;
}

void l11uxx_uart_clearRxBuffer(){
	//bitbangUARTmessage("BUFFERCLEAR!\n\r");
	l11uxx_uart_sendToBuffer();
	//LPC_USART->FCR |= (1<<1); //Rx FIFO reset!
	l11uxx_uart_clearRxBuffer_withoutReadout();

	return;
}

void UART_IRQHandler(void){
	//LPC_GPIO->SET[0] = (0x20000); //0_17

	if(rxBusy); //already work happening
	else l11uxx_uart_sendToBuffer();
	//l11uxx_uart_sendToBuffer();

	//LPC_GPIO->CLR[0] = (0x20000); //0_17*/
	//return;
}

void l11uxx_uart_spewBuffer(){
	//lcd_5110_printAsConsole(l11uxx_uart_rx_buffer, 0);
	char temporaryString1[40];
	int i = 0;
	bitbangUARTmessage("LSRReg: 0b");
	itoa(LPC_USART->LSR, temporaryString1, 2);
	bitbangUARTmessage(temporaryString1);
	bitbangUARTmessage("; LSRReg: 0b");
	itoa(LPC_USART->LSR, temporaryString1, 2);
	bitbangUARTmessage(temporaryString1);
	bitbangUARTmessage("; RxBSt: 0x");
	itoa(&l11uxx_uart_rx_buffer, temporaryString1, 16);
	bitbangUARTmessage(temporaryString1);
	bitbangUARTmessage("; RxIn: 0x");
	itoa(l11uxx_uart_rx_buffer_current_index, temporaryString1, 16);
	bitbangUARTmessage(temporaryString1);
	bitbangUARTmessage("; RxBEn: 0x");
	i = &l11uxx_uart_rx_buffer;
	itoa((i+l11uxx_uart_rx_buffer_current_index), temporaryString1, 16);
	bitbangUARTmessage(temporaryString1);
	bitbangUARTmessage("\r\n");

	bitbangUARTmessage("RxBCont: ");
	i=0;
	while((l11uxx_uart_rx_buffer[i]) != 0){
		temporaryString1[0]=(l11uxx_uart_rx_buffer[i]);
		temporaryString1[1]=0;
		bitbangUARTmessage(temporaryString1);
		i++;
	}
	//bitbangUARTmessage(l11uxx_uart_rx_buffer);

	bitbangUARTmessage("!\r\n");
	return;
}


int l11uxx_uart_init(uint32_t baudrate){
	//LPC_IOCON->PIO1_7|=0x1; //47 - TX to mating part
	//LPC_IOCON->PIO1_6|=0x1; //46 - RX from mating part

	uint32_t  DL;
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
	LPC_SYSCON->UARTCLKDIV = 0x01;        //UART clock
	DL = (SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV) / (16 * baudrate * LPC_SYSCON->UARTCLKDIV);



	//let's hardcode this to 115200
	//DL=37; //yeah, at 48MHz maybe?, almost 19200 w/o xtal
	//DL=6; //115200 w/o xtal, 12MHz, works well
	//DL=76; //9600 w/o xtal, 12MHz,
	//LPC_USART->FDR = (12 << 4) | 1; //DivAddVal = 1, MulVal = 12

	LPC_USART->FDR = (1<<4) | (0); //If DivAddValue 0, will not affect USART baud rate, MulVal must be 1 or more
	LPC_USART->LCR = 0b10000011;  //  8-bit character length, DLAB enable ( DLAB = 1)
	LPC_USART->DLM = (DL >> 8) 	& 0xFF;    //  Determines the baud rate of the UART (MSB Register) (DLAB must be set to 1 first)
	LPC_USART->DLL = (DL	 )	& 0xFF;    //  Determines the baud rate of the UART (LSB Register) (DLAB must be set to 1 first)
	LPC_USART->LCR = 0b00000011;  //  8-bit character length , DLAB disable ( DLAB = 0)
	LPC_USART->FCR = 0b00000111;  //  FIFOEN:Active high enable for both UART Rx and TX FIFOs and U0FCR[7:1] access
	//  RXFIFORES:Writing a logic 1 to U0FCR[1] will clear all bytes in UART Rx FIFO, reset the pointer logic.
	//  TXFIFORES:Writing a logic 1 to U0FCR[2] will clear all bytes in UART TX FIFO, reset the pointer logic.

	//RX interrupt setup
	LPC_USART->IER |= (1<<2); //RXLIE - RX Line Interrupt Enable
	LPC_USART->IER |= (1<<0); // RBRINTEN - Receive data available interrupt enable
	//LPC_USART->FCR |= (0x3<<6); //trigger level 3, 14 characters in RX buffer


	LPC_USART->FCR |= (0x0<<6); //trigger level 0, 1 characters in RX buffer

	LPC_USART->FCR |= (1<<1); //Rx FIFO reset!
	l11uxx_uart_clearRxBuffer();

	/*static int firstTime=0;
	if(firstTime == 0){

		firstTime=1;
	}*/
	NVIC_EnableIRQ(UART_IRQn);
	return 1;
}
