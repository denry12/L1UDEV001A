/*
 * flash25P10.c
 *
 *  Created on: 25.05.2016
 *      Author: Denry
 */

//Datasheet: http://www.mouser.com/ds/2/389/STMicroelectronics_M25P10-464490.pdf
#include "flash25P10.h"

//#define FLASH_CS_PIN 2
//#define FLASH_CS_PORT 2

//L1UDEV001A, LPC1114
//#define FLASH_CS_PIN 6
//#define FLASH_CS_PORT 2

//L1UDEV001A, LPC11U35
#define FLASH_CS_PIN 25
#define FLASH_CS_PORT 1
#define FLASH_SPI_NUMBER 1



//HW specific code starts here
int flash25P10_enableCS(){
	while(l11uxx_spi_checkIfBusy(FLASH_SPI_NUMBER)); //SPI controller busy. waits here
	//this busy waiting is done in SPI library as well, so this is redundant

	GPIOSetValue(FLASH_CS_PORT, FLASH_CS_PIN, 0);
	return 1;
}

int flash25P10_disableCS(){
	while(l11uxx_spi_checkIfBusy(FLASH_SPI_NUMBER)); //SPI controller busy. waits here
	//this busy waiting is done in SPI library as well, so this is redundant

	GPIOSetValue(FLASH_CS_PORT, FLASH_CS_PIN, 1);
	return 1;
}


//this function is basically duplicate of SPI library. REMOVE.
//void lpc_ssp1_cr0set(int DSS, int FRF, int CPOL, int CPHA, int SCR){
//	int cr0=0;
//
//	cr0|=DSS&0x0F;
//	cr0|=((FRF&0x03)<<4);
//	cr0|=((CPOL&0x01)<<6);
//	cr0|=((CPHA&0x01)<<7);
//	cr0|=((SCR&0xFF)<<8);
//	//by this line (stopping here) SCK remains screwed.
//	LPC_SSP1->CR0=cr0;
//	//by the time stopping on this line, SCK has returned to correct state
//	//while((GPIOGetValue(2,1)) == 0); //just wait until SCK returns high, deffo can't touch CS before.				//ATTN, THIS IS PROCESSOR SPECIFIC!
//	//by the time stopping on this line, SCK has returned to correct state
//	return;
//}

int flash25P10_init(){

	//about flash HW
	//HOLD should be pulled up
	//WRITE should be pulled up as well
	//no statement that it should full-duplex. To read, gonna blast out 0xFF-s.
	//don't write more than 128 bytes in a row, it will discard previous data.

	int error=0;

	//not sure if this is necessary... just to make sure
	//PLS REMOVE LATER, THIS MAKES CODE LESS UNIVERSAL
	/*#define SPI_MISO_PIN 8
	#define SPI_MISO_PORT 0
	#define SPI_MOSI_PIN 9
	#define SPI_MOSI_PORT 0
	#define SPI_SCK_PIN 11
	#define SPI_SCK_PORT 2
	GPIOSetDir(SPI_MISO_PORT, SPI_MISO_PIN, 0);
	GPIOSetDir(SPI_MOSI_PORT, SPI_MOSI_PIN, 1);
	GPIOSetDir(SPI_SCK_PORT, SPI_SCK_PIN, 1);
	GPIOSetValue(SPI_SCK_PORT, SPI_SCK_PIN, 1);*/


	GPIOSetDir(FLASH_CS_PORT, FLASH_CS_PIN, 1); //keep chipselect!!!!!!!!!!!
//	GPIOSetDir(2, 6, 1);
	flash25P10_disableCS();

	//let's do mode 1,1 (sck idle = high)
	//data clocked in on the SDI pin on the rising edge of SCK

	//expecting setup like this to have been done:
	//l11uxx_spi_pinSetup(1, 38, 26, 13);
	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0); //works well for 320x240rgblcd & ext flash


	//THESE ARE DONE IN SPI LIBRARY
	//LPC_SYSCON->PRESETCTRL|=4; //de-assert SPI1 reset
	//LPC_IOCON->PIO2_2|=2; //set to miso1 //miso1 defaults to 2_2
	//LPC_IOCON->PIO2_3|=2; //set to mosi1 //mosi1 defaults to 2_3

	//LPC_IOCON->PIO2_1|=2; //set to sck1 //this line fucking sets SCK low for a brief period

	//LPC_IOCON->SCK_LOC=1; //set SCK0 to p2.11 //sck1 defaults to 2_1


	//THESE ARE DONE IN SPI LIBRARY
	//lpc_ssp1_cr0set(0x7, 0, 1, 1, 100); //8 bit transfer, SPI format, CPOL 1, CPHA 1, prescaler 100
	//by this line, SCK has returned to correct state (even before)
	//LPC_SSP1->CPSR|=2;//some weird ass prescaler. Must be between 2 and 254
	//LPC_SSP1->CR1|=(1<<1);//enable SPI controller

	return error;
}

int flash25P10_sendByte(int dataToBeSent){
	//THESE DONE IN SPI LIBRARY
	//while(!(LPC_SSP1->SR&(1<<1))); //wait if tx FIFO is full
	//while(LPC_SSP1->SR&(1<<4)); //SPI controller busy. waits here

	//LPC_SSP1->DR=(0xFF&dataToBeSent);//send out data
	l11uxx_spi_sendByte(FLASH_SPI_NUMBER, dataToBeSent);
	return 1;
}

int flash25P10_readByte(){
	//int dataRead;
	//while(LPC_SSP1->SR&(1<<4)); //SPI controller busy. waits here
	//while(LPC_SSP1->SR&(1<<2)){ //while receive buffer not empty, keep emptying it
	//	dataRead = LPC_SSP1->DR; //empty it out, just in case
	//	printf("						Dataread to empty: %d\n", dataRead);
	//}
	//dataRead=0;
	//flash25P10_sendByte(0xFF); //to pound clock for new info
	//while(LPC_SSP1->SR&(1<<4)); //SPI controller busy. waits here
	//dataRead = LPC_SSP1->DR; //get data
	//printf("Dataread B: %d\n", dataRead);
	//while(dataRead>0xFF); //it was initialized as 0, now it's over 8bits? Lock processor, something is wrong!
	//dataRead = (LPC_SSP0->DR)&0xFF; //use this if previous no work



	//l11uxx_spi_flushRxBuffer(FLASH_SPI_NUMBER); //this may be overkill. Will see. Should be done in spi library maybe
	return l11uxx_spi_receiveByte(FLASH_SPI_NUMBER);
	//return dataRead;
}
//HW specific code ends here







int flash25P10_readStatusRegister(){
	int statusRegister=0;
	flash25P10_enableCS();
	flash25P10_sendByte(0b00000101);
	statusRegister=flash25P10_readByte();
	flash25P10_disableCS();
	return statusRegister;

}

int flash25P10_readDataBytes(int startingAddress, int bytesToRead, uint8_t *dataArray){
	int bytesRead=0;
	flash25P10_enableCS();
	flash25P10_sendByte(0b00000011);

	flash25P10_sendByte((startingAddress>>16)&0xFF);//24bit address
	flash25P10_sendByte((startingAddress>>8)&0xFF);//24bit address
	flash25P10_sendByte(startingAddress&0xFF);//24bit address


	while (bytesRead<bytesToRead){
		*(dataArray+bytesRead)=flash25P10_readByte();
		//printf("						FLASH: %x\n",dataArray[bytesRead]);
		bytesRead++;
	}
	flash25P10_disableCS();
	return 1;

}

int getWriteInProgressBit(){
	int WIP=0;
	WIP=(flash25P10_readStatusRegister());
	WIP=WIP&0x01;
	return WIP;

}

int flash25P10_eraseSector(long addressOfSector){ //any address in sector is valid for erasing sector
	int sectorAddress2;
	int sectorAddress1;
	int sectorAddress0;

	sectorAddress0=((addressOfSector>>0)&0xFF);
	sectorAddress1=((addressOfSector>>1)&0xFF);
	sectorAddress2=((addressOfSector>>2)&0xFF);

	while(getWriteInProgressBit()); //wait while device busy writing

	flash25P10_enableCS();
	flash25P10_sendByte(0x06); //write enable
	flash25P10_disableCS();

	flash25P10_enableCS();
	flash25P10_sendByte(0xD8); //sector erase
	flash25P10_sendByte(sectorAddress2);//three address bytes
	flash25P10_sendByte(sectorAddress1);//three address bytes
	flash25P10_sendByte(sectorAddress0);//three address bytes
	flash25P10_disableCS();
	return 1;
}

int flash25P10_writeDataBytes(int startingAddress, int bytesToWrite, uint8_t *dataArray, int verify){
	int bytesWritten=0;
	int sectorAddress2;
	int sectorAddress1;
	int sectorAddress0;
	if(verify){

	}

	sectorAddress0=((startingAddress>>0)&0xFF);
	sectorAddress1=((startingAddress>>1)&0xFF);
	sectorAddress2=((startingAddress>>2)&0xFF);

	while(getWriteInProgressBit()); //wait while device busy writing

	flash25P10_enableCS();
	flash25P10_sendByte(0x06); //write enable
	flash25P10_disableCS();


	flash25P10_enableCS();
	flash25P10_sendByte(0x02); //page program
	flash25P10_sendByte(sectorAddress2);//three address bytes
	flash25P10_sendByte(sectorAddress1);//three address bytes
	flash25P10_sendByte(sectorAddress0);//three address bytes
	while (bytesWritten<bytesToWrite){
		flash25P10_sendByte(*(dataArray+bytesWritten));
		bytesWritten++;
	}
	flash25P10_disableCS();


	if(verify){
		char readbackArray[bytesToWrite+2]; //change to +1?
		int i=0;
		while(getWriteInProgressBit()); //wait while device busy writing
		flash25P10_readDataBytes(startingAddress, bytesToWrite, &readbackArray);
		while(i<bytesToWrite){
			if(readbackArray[i] != (*(dataArray+i))) return 0;
			i++;
		}
	}

	//up to 256 data bytes! (for 512kbit; for 1mbit is 128 bytes)

	//write enable is reset after:
	//-power up
	//-write disable instruction completion
	//-write status register instruction completion
	//-page-program instruction completion
	//-sector erase instruction completion
	//-bulk erase instruction completion
	//flash25P10_enableCS();
	//flash25P10_sendByte(0x04); //write disable
	//flash25P10_disableCS();


	return 1;
}

