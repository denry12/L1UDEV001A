/*
 * l11uxx_spi_library.c
 *
 *  Created on: 01.10.2016
 *      Author: Denry
 */

#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif

void l11uxx_spi_pinSetup(int SPINumber, int mosiPin, int misoPin, int sckPin){ //uses physical pin numbers
	//setting up mosi
	if(SPINumber == 0) while (1); //lol I am too lazy to make it, I will prolly notice at some point
	else if(SPINumber == 1){
		if (mosiPin == 17) LPC_IOCON->PIO0_21 |= 0x2; //p0_21
		else if (mosiPin == 38) LPC_IOCON->PIO1_22 |= 0x2; //p1_22

	//setting up miso

		if (misoPin == 30) LPC_IOCON->PIO0_22 |= 0x2; //p0_22
		else if (misoPin == 26) LPC_IOCON->PIO1_21 |= 0x2; //p1_21



	//setting up sck

		if (sckPin == 43) LPC_IOCON->PIO1_15 |= 0x3; //p1_15
		else if (sckPin == 13) LPC_IOCON->PIO1_20 |= 0x2; //p1_20
	}

	return;
}
void l11uxx_spi_pinSetup_unset(int pin){ //uses physical pin numbers
	//setting up mosi
		if (pin == 17) LPC_IOCON->PIO0_21 &= (~0x07); //p0_21
		else if (pin == 38) LPC_IOCON->PIO1_22 &= (~0x07); //p1_22

	//setting up miso
		else if (pin == 30) LPC_IOCON->PIO0_22 &= (~0x07); //p0_22
		else if (pin == 26) LPC_IOCON->PIO1_21 &= (~0x07); //p1_21

	//setting up sck
		else if (pin == 43) LPC_IOCON->PIO1_15 &= (~0x07); //p1_15
		else if (pin == 13) LPC_IOCON->PIO1_20 &= (~0x07); //p1_20


	return;
}

void l11uxx_spi_init(int SPINumber, int bits, int FRF, int CPOL, int CPHA, int SCR, int MS, int CPSDVSR){ //replace most ints with chars?
	int temporaryValue=0;
	//LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<18); //enable clock to SSP1 //done on device setup?
	//LPC_SYSCON->SSP1CLKDIV = 1; //SSP1 clock divider //done on device setup?
	//LPC_SYSCON->PRESETCTRL |= (1 << 2); //remove reset from SSP1 //done on device setup?

	//setting up CR0
	bits=bits-1; //0xF is 16-bit, 0x7 is 8-bit
	if(bits<3)bits=3;
	bits &= 0b1111;

	if(FRF == 0x3) FRF=0; //illegal combo
	FRF &= 0b11;

	CPOL &= 0x1;

	CPHA &= 0x1;

	SCR &= 0b11111111;

	temporaryValue = ((bits << 0) | (FRF << 4) | (CPOL << 6) | (CPHA << 7) | (SCR << 8));

	if(SPINumber == 0) LPC_SSP0->CR0 = temporaryValue;
	else if(SPINumber == 1) LPC_SSP1->CR0 = temporaryValue;

	temporaryValue = 0;

	temporaryValue = (MS << 2);

	if(SPINumber == 0){
		LPC_SSP0->CR1=temporaryValue; //prep CR1
		LPC_SSP0->CR1=temporaryValue | (1 << 1);//enable SPI
	} else if(SPINumber == 1){
			LPC_SSP1->CR1=temporaryValue; //prep CR1
			LPC_SSP1->CR1=temporaryValue | (1 << 1);//enable SPI
	}

	//CPSDVSR must be even value, bit 0 always reads as 0
	CPSDVSR &= 0xFE;
	if(SPINumber == 0){
		LPC_SSP0->CPSR=CPSDVSR;
	} else if(SPINumber == 1){
		LPC_SSP1->CPSR=CPSDVSR;
	}


	return;
}

void l11uxx_spi_slave_outEnable(int SPINumber){
	if(SPINumber == 0) LPC_SSP0->CR1 &= (~((~(0xFFFFFFFF)) | (1<<2))); //fuck this is ugly, but this way I can be sure register stays 32bit
	else if(SPINumber == 1) LPC_SSP1->CR1 &= (~((~(0xFFFFFFFF)) | (1<<2)));
	return;
}

void l11uxx_spi_slave_outDisable(int SPINumber){
	if(SPINumber == 0) LPC_SSP0->CR1 |= (1<<2);
	else if(SPINumber == 1) LPC_SSP1->CR1 |= (1<<2);
	return;
}

void l11uxx_spi_sendByte(int SPINumber, int data){
	if(SPINumber == 0){
		while (!(LPC_SSP0->SR & (0x1 << 1))); //check if transmit buffer full
		LPC_SSP0->DR = data;
		while (LPC_SSP0->SR & (0x1 << 4)); //wait while SPI busy //ATTN, locks processor!
	}
	else if(SPINumber == 1){
		while (!(LPC_SSP1->SR & (0x1 << 1))); //check if transmit buffer full
		LPC_SSP1->DR = data;
		while (LPC_SSP1->SR & (0x1 << 4)); //wait while SPI busy //ATTN, locks processor!
	}
	return;
}

int l11uxx_spi_flushRxBuffer(int SPINumber){
	int garbage = 0;
		//FLUSH BUFFERS FIRST???

		if(SPINumber == 0){
			while ((LPC_SSP0->SR & (0x1 << 2))) garbage = LPC_SSP0->DR; //flush buffer
		}
		else if(SPINumber == 1){
			while ((LPC_SSP1->SR & (0x1 << 2))) garbage = LPC_SSP1->DR; //flush buffer

		}
	return;
}

int l11uxx_spi_receiveByte(int SPINumber){
	int data = 0;
	//int garbage = 0;
	//FLUSH BUFFERS FIRST???

	if(SPINumber == 0){
		//while (!(LPC_SSP0->SR & (0x1 << 2))) garbage = LPC_SSP0->DR; //flush buffer
		while (!(LPC_SSP0->SR & (0x1 << 1))); //check if transmit buffer full
		LPC_SSP0->DR = 0x00; //send out empty packet to move data
		while (LPC_SSP0->SR & (0x1 << 4)); //wait while SPI busy
		//while ((LPC_SSP0->SR & (0x1 << 2))); //wait until data available
		data = LPC_SSP0->DR;
	}
	else if(SPINumber == 1){
		//while (!(LPC_SSP1->SR & (0x1 << 2))) garbage = LPC_SSP1->DR; //flush buffer
		while (!(LPC_SSP1->SR & (0x1 << 1))); //check if transmit buffer full
		LPC_SSP1->DR = 0x00; //send out empty packet to move data
		while (LPC_SSP1->SR & (0x1 << 4)); //wait while SPI busy
		//while ((LPC_SSP1->SR & (0x1 << 2))); //wait until data available
		data = LPC_SSP1->DR;
	}
	return data;

}

int l11uxx_spi_checkIfBusy(int SPINumber){

	if(SPINumber == 0){
		if (LPC_SSP0->SR & (0x1 << 4)) return 1; //very busy, no disturb
	}
	else if(SPINumber == 1){
		if (LPC_SSP1->SR & (0x1 << 4)) return 1; //very busy, no disturb
	}
	return 0; //not busy
}
