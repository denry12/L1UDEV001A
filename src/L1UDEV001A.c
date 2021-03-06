/*
===============================================================================
 Name        : L1UDEV001A.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/


#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif

#include <cr_section_macros.h>

#include <stdio.h>
#include "gpio.h"
#include "lcd_5110_lib.h"
//#include "UART1114.h"
#include "esp8266.h"
#include "bitbangUART.h"
#include "hd44780.h"
#include "ILI9341.h"
#include "bufferManipulation.h"
#include "nrf24l01_lib.h"
#include "lcd_hx1230_lib.h"

//hwtests

//#include "JDP_wifi_creds.h" 	//NB! You do not have this file. It just overwrites next two defines
//#include "le_wifi_creds.h" 	//NB! You do not have this file. It just overwrites next two defines
#ifndef WIFI_SSID
#define WIFI_SSID "4A50DD"
//#define WIFI_SSID "Test-asus"
#endif
#ifndef WIFI_PASSWD
#define WIFI_PASSWD "2444666668888888"
//#define WIFI_PASSWD "24681357"
#endif


// TODO: insert other include files here

// TODO: insert other definitions and declarations here

/*#define LED_0_ON GPIOSetValue(0, 8, 0)
#define LED_1_ON GPIOSetValue(0, 9, 0)
#define LED_2_ON GPIOSetValue(0, 10, 0)
#define LED_4_ON GPIOSetValue(1, 9, 0)
#define LED_3_ON GPIOSetValue(3, 4, 0)
#define LED_5_ON GPIOSetValue(2, 7, 0)

#define LED_0_OFF GPIOSetValue(0, 8, 1)
#define LED_1_OFF GPIOSetValue(0, 9, 1)
#define LED_2_OFF GPIOSetValue(0, 10, 1)
#define LED_4_OFF GPIOSetValue(1, 9, 1)
#define LED_3_OFF GPIOSetValue(3, 4, 1)
#define LED_5_OFF GPIOSetValue(2, 7, 1)*/


//DO INLINE!
inline void turboPinHigh(){ //turbopin is 1_13 (later 1_14 mby if necessary)
	LPC_GPIO->SET[1] |= (0x2000);
	//LPC_GPIO->SET[1] = (0x2000);
	//return;
}

inline void turboPinLow(){
	//LPC_GPIO->CLR[portNum] |= (1<<bitPosi);
	LPC_GPIO->CLR[1] |= (0x2000);
	//LPC_GPIO->CLR[1] = (0x2000);
	//return;
}

void delay_us(int us){
	LPC_CT16B0->TC = 0;
	LPC_CT16B0->PC = 0;
	//maybe I can remove these?

	//RELEVANT TO CLOCK, if 12MHz clock, set to 11. If 48MHz clock, set to 47
	if(SystemCoreClock == 48000000) LPC_CT16B0->PR = 47;
	else LPC_CT16B0->PR = 11; //16,000,000 / 16 = 1us. Incremented every PCLK.

	LPC_CT16B0->MCR = 0x5; //stop on MR0 match. Interrupt, no reset


	LPC_CT16B0->MR0 = us;
	while ( !(LPC_CT16B0->MR0 == us) ); // Wait until updated
	LPC_CT16B0->TCR = 0x1; //enable
	while(LPC_CT16B0->TCR & 0x1); //wait while timer is running.
	return;
}

void delay(int ms){
	while(ms>=65){
		delay_us(65000);
		ms-=65;
	}
	delay_us(ms*1000);
	return;
}


/*
void setup48MHzInternalClock(){
		//MAKES LPC GO SOVERYFAST

	//this function is outdated and equivalent to:
	//if( setupClocking(0, 48000000) ) while (1); //failed to set clock. Lock MCU



	LPC_SYSCON->PDAWAKECFG &= (~(1<<7)); //pll powered up after wake (this is a desperate attempt)

		 //set main clock to IRC
		 LPC_SYSCON->MAINCLKSEL  = 0;
		 LPC_SYSCON->MAINCLKUEN  = 0;
		 LPC_SYSCON->MAINCLKUEN  = 1;


		LPC_SYSCON->SYSPLLCLKSEL = 0x0; //set sys pll source IRC
		LPC_SYSCON->SYSPLLCLKUEN	= 0x0; //update clock source (step 1/2)
		LPC_SYSCON->SYSPLLCLKUEN	= 0x1; //update clock source (step 2/2)


		LPC_SYSCON->PDRUNCFG |= (1<<7); //pll should be powered down here
		//while(!(LPC_SYSCON->PDRUNCFG & (1<<7))); //verify that PLL is powered down.

		LPC_SYSCON->SYSPLLCTRL = 0x23; //unless I fucked up, if no work, try 0x60 for 1to1 ratio
		//LPC_SYSCON->SYSPLLCTRL = 0x60; //1to1
		//while(!(LPC_SYSCON->SYSPLLCTRL ==  (0x23)));
		//now might be reasonable to POWER UP THE FUCKING PLL
		LPC_SYSCON->PDRUNCFG &= (~(1<<7));
		//while((LPC_SYSCON->PDRUNCFG & (1<<7))); //verify that PLL is powered.

		GPIOSetValue(1, 13, 0);
		GPIOSetValue(1, 14, 0);
		delay(1);
		while((!(LPC_SYSCON->SYSPLLSTAT)&0x01)); //while PLL not locked, we wait.
		GPIOSetValue(1, 13, 1);
		GPIOSetValue(1, 14, 1);

		LPC_SYSCON->MAINCLKSEL = 0x3; //set sys pll output as main clock input
		LPC_SYSCON->MAINCLKUEN = 0x0; //update clock source, 1/2
		LPC_SYSCON->MAINCLKUEN = 0x1; //update clock source, 2/2


		SystemCoreClock = 48000000; //because without this UART has like 0 idea wtf is going on
}*/

void setupClocks(){
	GPIOSetValue(1, 13, 0);
	GPIOSetValue(1, 14, 1);

	LPC_SYSCON->SYSAHBCLKDIV = 1;

	//LPC_SYSCON->PDRUNCFG |= (1<<5); //power down xtal osc
	//LPC_SYSCON->PDRUNCFG &= (~(1<<5)); //power up xtal osc
	LPC_SYSCON->SYSOSCCTRL |= 0x01;

	LPC_SYSCON->PDRUNCFG &= (~(1<<4)); //power up ADC
	LPC_SYSCON->PDRUNCFG &= (~(1<<6)); //power up WDT osc
	//page 30 of usermanual
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<5); //enable clock to I2C
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<7); //enable clock to CT16B0
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<8); //enable clock to CT16B1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<9); //enable clock to CT32B0
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<10); //enable clock to CT32B1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<12); //enable clock to USART
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<13); //enable clock to ADC
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<15); //enable clock to WDT
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<18); //enable clock to SSP1
//	LPC_SYSCON->SSP1CLKDIV = 1; //SSP1 clock divider
	//LPC_SYSCON->SSP1CLKDIV = 8; //SSP1 clock divider
	LPC_SYSCON->SSP1CLKDIV = 64; //SSP1 clock divider
	LPC_SYSCON->PRESETCTRL |= (1 << 1); //remove reset from I2C
	LPC_SYSCON->PRESETCTRL |= (1 << 2); //remove reset from SSP1

	LPC_SYSCON->UARTCLKDIV = 1; //USART clock divider


	/*GPIOSetValue(1, 13, 0);
	GPIOSetValue(1, 14, 1);*/
	return;
}

bool setupClocking(uint32_t xtalInHz, uint32_t targetMCUclockInHz){

	uint32_t MSEL = 0;
	uint32_t PSEL = 0;
	bool xtalUsed;
	uint32_t pllCtrlReg = 0;

	if(xtalInHz == 0){
		xtalUsed = 0;
		xtalInHz = 12000000; //use IRC value, 12 MHz
	} else xtalUsed = 1;


	//setupClocks(); currently done in main, should be done here in future



	MSEL = targetMCUclockInHz / xtalInHz;
	if ((MSEL * xtalInHz) != targetMCUclockInHz) return 1; //something fractional, not possible

	MSEL = MSEL - 1; //adjust M to MSEL

	//only 4 possible values for PSEL
	if		( ((2*targetMCUclockInHz)	>= 156000000) 	&& ((2*targetMCUclockInHz)	<= 320000000) )	 PSEL = 0;
	else if	( ((4*targetMCUclockInHz)	>= 156000000)	&& ((4*targetMCUclockInHz)	<= 320000000) )	 PSEL = 1;
	else if	( ((8*targetMCUclockInHz)	>= 156000000)	&& ((8*targetMCUclockInHz)	<= 320000000) )	 PSEL = 2;
	else if	( ((16*targetMCUclockInHz)	>= 156000000)	&& ((16*targetMCUclockInHz)	<= 320000000) )	 PSEL = 3;
	else return 1; //cannot find suitable PSEL value
	//at this point, PSEL value is set in the final format

	//set main clock to IRC, cause we are changing it and want it to run meanwhile
	 LPC_SYSCON->MAINCLKSEL  = 0;
	 LPC_SYSCON->MAINCLKUEN  = 0;
	 LPC_SYSCON->MAINCLKUEN  = 1;

	 //TODO: check that we run on IRC?

	if (xtalUsed == 0){
		//no xtal.
		LPC_SYSCON->SYSPLLCLKSEL 	= 0x0; //set sys pll source IRC
	}
	else {
		//have xtal.
		LPC_SYSCON->SYSPLLCLKSEL 	= 0x1; //set sys pll source XTAL
		if(LPC_SYSCON->PDRUNCFG & (1<<5)){
				LPC_SYSCON->PDRUNCFG &= (~(1<<5)); //power up xtal osc
		}
		if(LPC_SYSCON->SYSOSCCTRL & (1<<0)){
				LPC_SYSCON->SYSOSCCTRL &= (~(1<<0)); //disable xtal oscillator bypass
		}
		if((LPC_SYSCON->SYSOSCCTRL & (1<<1)) && (xtalInHz > 20000000))
						LPC_SYSCON->SYSOSCCTRL |= (~(1<<1)); //set freq range 15-25MHz
		else if ((!(LPC_SYSCON->SYSOSCCTRL & (1<<1))) && (xtalInHz < 20000000))
			LPC_SYSCON->SYSOSCCTRL &= (~(1<<1)); //set freq range 1-20MHz

	}

	LPC_SYSCON->SYSPLLCLKUEN	= 0x0; //update clock source (step 1/2)
	LPC_SYSCON->SYSPLLCLKUEN	= 0x1; //update clock source (step 2/2)


	LPC_SYSCON->PDRUNCFG |= (1<<7); //pll should be powered down here
	//while(!(LPC_SYSCON->PDRUNCFG & (1<<7))); //verify that PLL is powered down.

	pllCtrlReg |= (MSEL		);
	pllCtrlReg |= (PSEL << 5);

	LPC_SYSCON->SYSPLLCTRL = pllCtrlReg;

	//while(!(LPC_SYSCON->SYSPLLCTRL ==  (0x23)));
	//now might be reasonable to POWER UP THE FUCKING PLL
	LPC_SYSCON->PDRUNCFG &= (~(1<<7));
	//while((LPC_SYSCON->PDRUNCFG & (1<<7))); //verify that PLL is powered.

	delay(1);
	while((!(LPC_SYSCON->SYSPLLSTAT)&0x01)); //while PLL not locked, we wait.
	GPIOSetValue(1, 13, 1);
	GPIOSetValue(1, 14, 1);

	LPC_SYSCON->MAINCLKSEL = 0x3; //set sys pll output as main clock input
	LPC_SYSCON->MAINCLKUEN = 0x0; //update clock source, 1/2
	LPC_SYSCON->MAINCLKUEN = 0x1; //update clock source, 2/2

	SystemCoreClock = targetMCUclockInHz; //because without this UART has like 0 idea wtf is going on






	return 0; //all cool
}





int flash_service_routine(){
	while(1){
		debugOutput("FLASH ERASE @ ");
		//debugOutput();


		debugOutput("Flash write go @ ");
		//debugOutput();
		debugOutput("; ");
		//debugOutput();
		debugOutput(" bytes \n\r");

		debugOutput("Flash write end\n\r");

		debugOutput("Read @ \n\r");
	}
	return 0;
}

int debugOutput(char *message){
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


bool esp8266_LPCToESP(esp8266_instance *instance){
	char response;

	//delay(1);
	//bitbangUARTmessage("esp01addrL2E: ");
	//bitbangUARThex(instance,0,0);
	//bitbangUARTmessage("\r\n");

	//if there are chars waiting in buffer (not returning <0 (e.g. -1))
	response = esp8266_charFromBufferToUart(instance);
	if (response < 0) return 1;

	//char placeholder = 'X';
	//placeholder = response & 0xFF;

	//send it to UART (if UART not busy?)
	//todo: REPLACE IT WITH "IF UART FULL" if possible
	while ((!LPC_USART->LSR & (1<<5))){  //THRE: Transmitter Holding Register Empty.
				                                        // 0: THR contains valid data, 1: THR is empty
			//just block it. It will send out data soon unless something very wrong
	}
	LPC_USART->THR = (response & 0xFF);
	delay(1);
	return 0;
}

bool esp8266_ESPToLPC(esp8266_instance *instance){

	//delay(1);
	volatile extern int l11uxx_uart_rx_buffer_current_index;
	extern char *l11uxx_uart_rx_buffer;

	//bitbangUARTmessage("esp01addrE2L: ");
	//bitbangUARThex(instance,0,0);
	//bitbangUARTmessage("\r\n");

	//this should occur anyway cause 1-byte trigger for interrupt
	//l11uxx_uart_sendToBuffer(); //throw from HW buffer to SW buffer

	//char debugtemp = *(&l11uxx_uart_rx_buffer+0);

	//if data available in UART buffer
	//if ((*(l11uxx_uart_rx_buffer)) == 0)
		if(l11uxx_uart_rx_buffer_current_index == 0)
			return 1; //no data, return





	//bitbangUARTmessage("UARTIndexPreClear: ");
	//bitbangUARTint(l11uxx_uart_rx_buffer_current_index,0,0);
	//bitbangUARTmessage("\r\n");




	//stop interrupts
	NVIC_DisableIRQ(UART_IRQn);

	int maxIndex = l11uxx_uart_rx_buffer_current_index;
	//copy data out and prep for new data
	char temporaryBuffer[l11uxx_uart_rx_buffer_current_index+2]; //+2 for extra space just in case

	memcpy(temporaryBuffer, &l11uxx_uart_rx_buffer, l11uxx_uart_rx_buffer_current_index);
	//strncpy(temporaryBuffer, (&l11uxx_uart_rx_buffer), l11uxx_uart_rx_buffer_current_index); //both appear to work equally well??

	//l11uxx_uart_clearRxBuffer(); <- careful, it fucks with UART interrupts!
	//l11uxx_uart_rx_buffer_current_index=0;
	//l11uxx_uart_rx_buffer=0;
	l11uxx_uart_clearRxBuffer_withoutReadout();

	//re-enable interrupts
	NVIC_EnableIRQ(UART_IRQn);


	int currentIndex = 0;

	//add the null terminator where it should be.
	//temporaryBuffer[currentIndex] = 0;

	//bitbangUARTmessage("UARTIndexPostClear: ");
	//bitbangUARTint(l11uxx_uart_rx_buffer_current_index,0,0);
	//bitbangUARTmessage("\r\n");

	//handle data, send it to ESP buffer
	while (currentIndex < (maxIndex + 0)){ //+1 to make sure nullterminator also comes over
		if(esp8266_charFromUartToBuffer(instance, temporaryBuffer[currentIndex]) == 0)
			currentIndex++;
		else return 1; //something wrong
	}

	return 0;
}


bool hd44780lcd_handler(hd44780_instance *instance){
	uint8_t LCDMSN, LCDRS, LCDRW, LCDE, LCDdatabyte;

	bool dataInvalid = 1;

	dataInvalid = hd44780_getFromTxBuffer(instance, &LCDMSN, &LCDRS, &LCDRW, &LCDE);



	//PCF-to-LCD pinout
	//PCF P0 = LCD RS (pin 4)
	//PCF P1 = LCD RW (pin 5)
	//PCF P2 = LCD E  (pin 6)
	//PCF P3 = LCD BL (E2, pin 15)
	//PCF P4 = LCD D4
	//PCF P5 = LCD D5
	//PCF P6 = LCD D6
	//PCF P7 = LCD D7



	while(dataInvalid == 0){

		//haha delays for I2C are ridic, cause the slow part is HD44780 from 70's or w/e, not PCF

		//got a packet to send
		l11uxx_i2c_sendStart();
		//delay(1);
		LCDdatabyte = 0;
		l11uxx_i2c_sendAddr(instance->I2C_addr, 0);

		//l11uxx_i2c_sendStop();

//		bitbangUARTmessage("E: ");
//		bitbangUARThex(((uint8_t)(LCDE)),0,0);
//		if (LCDE){
//			bitbangUARTmessage("; RS: ");
//			bitbangUARThex(((uint8_t)(LCDRS)),0,0);
//			bitbangUARTmessage("; frombuffer: ");
//			bitbangUARTbin(((uint8_t)(LCDMSN)),0,4);
//			bitbangUARTmessage("\r\n");
//		} else {
//			bitbangUARTmessage("\r\n");
//		}


		//delay(1);
		LCDdatabyte |= ((LCDMSN & 0x0F) << 4);
		LCDdatabyte |= ((LCDE   & 0x01) << instance->I2C_pinE_offset);
		LCDdatabyte |= ((LCDRW  & 0x01) << 1);
		LCDdatabyte |= ((LCDRS  & 0x01) << 0);
		//delay(1);
		l11uxx_i2c_sendByte(LCDdatabyte);
		//delay(1);
		l11uxx_i2c_sendStop();
		delay(1); // but one delay is still necessary cause otherwise PCF tries to go faster than LCD
		dataInvalid = hd44780_getFromTxBuffer(instance, &LCDMSN, &LCDRS, &LCDRW, &LCDE); //this was at top of while loop but it seemed illogical. How did it even work.
	}

	return dataInvalid; //if 0, all cool
}

bool printSmallPercent(hd44780_instance *instance, uint8_t percentValue){
	if (percentValue < 10){
		hd44780_lcdcursor(instance, 38, 1);
	} else if (percentValue < 100){
		hd44780_lcdcursor(instance, 37, 1);
	} else {
		hd44780_lcdcursor(instance, 36, 1);
	}
	char temporaryString[4];
	itoa(percentValue, temporaryString, 10);

	hd44780_printtext(instance, temporaryString);
	hd44780_printtext(instance, "%");
	return 0;
}

bool ili9341_handler(ili9341_instance *instance){
	uint8_t LCDdata, LCDDC, LCDCSchange;

	bool dataInvalid = 1;

	dataInvalid = ILI9341_getFromTxBuffer(instance, &LCDdata, &LCDDC, &LCDCSchange);

	while(dataInvalid == 0){
		//got a packet to send
		if(LCDCSchange){
			while (LPC_SSP1->SR & (0x1 << 4)); //wait while SPI busy //ATTN, locks processor!
			GPIOSetValue(1, 29, ((~LCDdata) & 0x01));
		} else {
			GPIOSetValue(1, 31, (LCDDC & 0x01));
			l11uxx_spi_sendByte(1, LCDdata);
		}
		delay(1);
		dataInvalid = ILI9341_getFromTxBuffer(instance, &LCDdata, &LCDDC, &LCDCSchange);
	}

	return dataInvalid; //if 0, all cool
}

// nrf24l01 hardware specific functions START

bool nrf24l01_spiSend(uint8_t *dataPacket){
	l11uxx_spi_sendByte(1, dataPacket);
	return 0;
}

bool nrf24l01_spiRxFlush(){
	l11uxx_spi_flushRxBuffer(1);
	return 0;
}

bool nrf24l01_spiGet(uint8_t *dataPacket){
	//nrf24l01_flushSPIRx();
	*dataPacket = l11uxx_spi_receiveByte(1);
	return 0;
}

bool nrf24l01_CSN_enable(){
	delay(10);
	GPIOSetValue(1, 25, 0);
	delay(10);
	return 0;
}

bool nrf24l01_CSN_disable(){
	delay(10);
	// TODO: ADD CHECK THAT SPI HAS FINISHED BEFORE RELEASING CS
	GPIOSetValue(1, 25, 1);
	delay(10);
	return 0;
}

bool nrf24l01_CE_enable(){
	GPIOSetValue(1, 19, 1);
	return 0;
}

bool nrf24l01_CE_disable(){
	GPIOSetValue(1, 19, 0);
	return 0;
}



// nrf24l01 hardware specific functions END


// lcd_hx1230 hardware specific functions START
bool hx1230_spiSend(uint16_t *dataPacket){
	l11uxx_spi_sendByte(1, dataPacket);
	return 0;
}
bool hx1230_CS_enable(){
	delay(10);
	GPIOSetValue(1, 29, 0);
	delay(10);
	return 0;
}

bool hx1230_CS_disable(){
	delay(10);
	// TODO: ADD CHECK THAT SPI HAS FINISHED BEFORE RELEASING CS
	GPIOSetValue(1, 29, 1);
	delay(10);
	return 0;
}

bool hx1230_reset_enable(){
	delay(10);
	GPIOSetValue(1, 28, 0);
	delay(10);
	return 0;
}

bool hx1230_reset_disable(){
	delay(10);
	GPIOSetValue(1, 28, 1);
	delay(10);
	return 0;
}
// lcd_hx1230 hardware specific functions END


int main(void) {




	int i=0, j=0;
	int debug=0;
	volatile char temporaryString1[300], temporaryString2[40];


	//printf("Hello, this is app.");
 	GPIOSetDir(0, 2, 0); //set input
	GPIOSetDir(0, 7, 0); //set input
	GPIOSetDir(0, 20, 1);
	GPIOSetValue(0, 20, 1); //turn off adjpsu
	GPIOSetDir(1, 13, 1);
	GPIOSetDir(1, 14, 1);
	GPIOSetDir(1, 27, 1);
	GPIOSetDir(0, 8, 1);
	GPIOSetValue(0, 8, 0);

	GPIOSetDir(0, 17, 1); //I use it for debug
	GPIOSetValue(0, 17, 0);


	setupClocks();

	//setup48MHzInternalClock(); //gotta go fast
	//if( setupClocking(16000000, 48000000) ) while (1); //failed to set clock. Lock MCU
	if( setupClocking(0, 48000000) ) while (1); //failed to set clock. Lock MCU


	//l11uxx_spi_pinSetup(1, 38, 26, 13);
	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0);
	//HW_test_lcd_5110_welcome();

	hx1230_instance hx1230_0;
	hx1230_0.sendSPIpacket = &hx1230_spiSend;
	hx1230_0.enableCS = &hx1230_CS_enable;
	hx1230_0.disableCS = &hx1230_CS_disable;
	hx1230_0.enableReset = &hx1230_reset_enable;
	hx1230_0.disableReset = &hx1230_reset_disable;
	GPIOSetDir(1, 29, 1); // CS of nokiaLCD/hx1230
	GPIOSetDir(1, 28, 1); // reset of ^
	//l11uxx_spi_init(1, 9, 0, 1, 1, 0, 0, 0); //works for hx1230
	//HW_test_lcd_hx1230(&hx1230_0);



	//buffertester_8(); //does not return

	l11uxx_spi_pinSetup(1, 38, 26, 13);
	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 2); //works well for 320x240rgblcd & ext flash & nokiaLCD(not correct tho)
	//l11uxx_spi_init(1, 8, 0, 0, 1, 0, 0, 0);
	l11uxx_spi_init(1, 8, 0, 0, 0, 0, 0, 0); //works for NRF (and rgb lcd?), a specific 9113wifi & nokiaLCD
	//l11uxx_spi_init(1, 8, 0, 1, 0, 0, 0, 0);
	//l11uxx_spi_init(int SPINumber, int bits, int FRF, int CPOL, int CPHA, int SCR, int MS, int CPSDVSR)

	l11uxx_uart_pinSetup(47, 46); //set up to CH340 //careful, esp is set afterwards
	l11uxx_uart_init(9600); //upping speed later mby

	HW_test_lcd_5110_with_uptime();

	nrf24l01_instance nrf24Radio01;
	nrf24Radio01.sendSPIpacket = &nrf24l01_spiSend;
	nrf24Radio01.getSPIpacket = &nrf24l01_spiGet;
	nrf24Radio01.flushSPIrxBuffer = &nrf24l01_spiRxFlush;
	nrf24Radio01.enableCSN = &nrf24l01_CSN_enable;
	nrf24Radio01.disableCSN = &nrf24l01_CSN_disable;
	nrf24Radio01.enableCE = &nrf24l01_CE_enable;
	nrf24Radio01.disableCE = &nrf24l01_CE_disable;

	GPIOSetDir(1, 25, 1); //nRF24L01 CSN
	GPIOSetDir(1, 19, 1); //nRF24L01 CE
	nrf24l01_init_generic(&nrf24Radio01);

	//TxRoutine and RxRoutine do not return
	//if(!(GPIOGetValue(0, 2))) txRoutine(&nrf24Radio01);
	if(!(GPIOGetValue(0, 7)))  rxRoutine(&nrf24Radio01);
	else txRoutine(&nrf24Radio01);


	bitbangUARTmessage("\r\n\r\n");

	/*
	//here starts hustle with circularbuffer16
	uint16_t testBufferData[50];
	uint8_t testBuffer8Data[400+2]; //nb, change this in init too
	circularBuffer_16bit testBuffer;
	circularBuffer_8bit testBuffer8;
	circularBuffer16_init(&testBuffer, 50, &testBufferData);
	circularBuffer8_init(&testBuffer8, 400, &testBuffer8Data);
	i = 0;
	while (i < 40){
		circularBuffer16_put (&testBuffer, i); //put in characters
		i++;
	}
	while (circularBuffer16_get(&testBuffer, &j) == 0){ //empty buffer
		bitbangUARTint(j,0, 3);
		bitbangUARTmessage("\r\n");
	}
	while (i < 80){
		circularBuffer16_put (&testBuffer, i); //put in characters
		i++;
	}
	while (circularBuffer16_get(&testBuffer, &j) == 0){ //empty buffer
			bitbangUARTint(j,0, 3);
			bitbangUARTmessage("\r\n");
	}

	while (i < 120){
		circularBuffer16_put (&testBuffer, i); //put in characters
		i++;
	}
	while (circularBuffer16_get(&testBuffer, &j) == 0){ //empty buffer
		bitbangUARTint(j,0, 3);
		bitbangUARTmessage("\r\n");
	}

	while (circularBuffer8_get(&testBuffer8, &j) == 0); //empty buffer
	circularBuffer8_put_string ((&testBuffer8), (",.-,.-,.-,.-,.-,.-,.-,")); //insert 22 char to make sure data breaks
	//circularBuffer8_put_string ((&testBuffer8), (",.-,.-,.-,.-,.-,.-,.-,.-,.-")); //insert 27 char to make sure data breaks
	while (circularBuffer8_get(&testBuffer8, &j) == 0); //empty buffer

	circularBuffer16_put_string ((&testBuffer), ("DATA:123456789;")); //put in characters
	//circularBuffer8_put_string ((&testBuffer8), ("DATA:123456789;NOISE")); //put in characters
	i = 0;
	while (i< 30){
		circularBuffer8_put_string ((&testBuffer8), ("0123456789\r\n")); //put in characters
		i++;
	}

	bitbangUARTmessage("\r\n8-BUFFER");
	bitbangUARTmessage("\r\n");
	while (circularBuffer8_get(&testBuffer8, &j) == 0){ //empty buffer
			bitbangUARTmessage(&j);
			//bitbangUARTmessage("\r\n");
	}
	bitbangUARTmessage("\r\n");
*/

	//here ends circularbuffer16

	//here starts findstring
	/*
	//findBetweenTwoStrings("DATA:123456789;", "DATA:", ";", &temporaryString1);
	temporaryString1[4] = 55;
	temporaryString1[5] = 0;
	//findBetweenTwoStrings((testBuffer8.Buffer+testBuffer8.BufferReadIndex), "DATA:", ";", &temporaryString1);

	findBetweenTwoStrings_circularBuffer(&testBuffer8, "DATA:\0", ";\0", &temporaryString1);

	while (circularBuffer8_get(&testBuffer8, &j) == 0){ //empty buffer
		bitbangUARTmessage(&j);
		bitbangUARTmessage("\r\n");
	}
*/
	//here ends findstring


	//here starts hustle with ili9341
/*
	GPIOSetDir(1, 27, 1); //BL as output
	GPIOSetDir(1, 28, 1); //reset as output
	GPIOSetDir(1, 29, 1); //CS as output
	GPIOSetDir(1, 31, 1); //D/C as output
	GPIOSetValue(1, 29, 1); //set CS to idle
	GPIOSetValue(1, 27, 1); //BL on (note that it is '1' cause direct-bypass bugfix)
	delay(10);
	GPIOSetValue(1, 28, 0); //do reset just in case
	delay(300);
	GPIOSetValue(1, 28, 1); //wake from reset
	delay(300);

	ili9341_instance iliLCD01;
	iliLCD01.handlerFunction = &ili9341_handler;

	//ILI9341_init(&iliLCD01, 240, 320);
	//ILI9341_setMemoryAccess(&iliLCD01, 0, 1, 0); //draws pokey facing right, bottom at pins, text readable

	ILI9341_init(&iliLCD01, 320, 240);
	ILI9341_setMemoryAccess(&iliLCD01, 0, 0, 1); //draws pokey facing right, right at pins, text readable


	//RAINBOW!
	signed int r=255, g=0, b=0, step = 0;
	while (i < iliLCD01.xResolution){
		if(step == 0){
			g+=16;
			if(g >= 255) step++;
		}
		else	if(step == 1){
			r-=16;
			if(r <= 0) step++;
		}
		else	if(step == 2){
			b+=16;
			if(b >= 255) step++;
		}
		else	if(step == 3){
			g-=16;
			if(g <= 0) step++;
		}
		else	if(step == 4){
			r+=16;
			if(r >= 255) step++;
		}
		else	if(step == 5){
			b-=16;
			if(b <= 0) step=0;
		}

		if(r > 255) r = 255;
		if(g > 255) g = 255;
		if(b > 255) b = 255;
		if(r < 0) r = 0;
		if(g < 0) g = 0;
		if(b < 0) b = 0;
		ILI9341_drawFastVLine(&iliLCD01, i, 0, iliLCD01.yResolution, ILI9341_Color65k(r, g, b));
		i++;
	}



	ILI9341_fillScreen(&iliLCD01, ILI9341_Color65k(0, 255, 255));

	ILI9341_drawPixel(&iliLCD01, 10, 10, 0xFFFF);
	ILI9341_drawPixel(&iliLCD01, 11, 10, 0xFFFF);
	ILI9341_drawPixel(&iliLCD01, 10, 11, 0xFFFF);
	ILI9341_drawPixel(&iliLCD01, 11, 11, 0xFFFF);

	ILI9341_drawPixel(&iliLCD01, 20, 10, 0xF00F);
	ILI9341_drawPixel(&iliLCD01, 21, 10, 0xF00F);
	ILI9341_drawPixel(&iliLCD01, 20, 11, 0xF00F);
	ILI9341_drawPixel(&iliLCD01, 21, 11, 0xF00F);

	//ILI9341_drawPixel(&iliLCD01, 500, 500, 0xFFFF);



	ILI9341_drawFastVLine(&iliLCD01, 100, 100, 80, 0x00FF); //do a blue line somewhere
	ILI9341_drawFastHLine(&iliLCD01, 100, 100, 80, 0xFF00); //do another line

	ILI9341_printString_bg(&iliLCD01, 100, 100, ILI9341_Color65k(255, 0, 0), ILI9341_Color65k(0, 255, 0), "Kitties B cute");

	ILI9341_invert(&iliLCD01, 1);
	delay(1000);
	ILI9341_invert(&iliLCD01, 0);
	delay(1000);

	drawPokey(&iliLCD01, 100, 10);

	i=0;
		while(i<50){
			while(j<41){
				//ILI9341_drawPixel(j+22, i+10, 0xFFFF);
				ILI9341_drawPixel(&iliLCD01, j+22, i+10, 0xFFFF);
				j++;
				ili9341_handler(&iliLCD01);
			}
			i++;
			j=0;
		}






	//iliLCD01.handlerFunction(); //this is broken, do not use
	ili9341_handler(&iliLCD01);
	while(1); //I don't wanna continue
*/

	uint8_t espRxBufferData[RX_BUFFER_SIZE+2]; //nb, change this in init too //not sure whether I need that +2. Hopefully not
	circularBuffer_8bit espRxBuffer;
	circularBuffer8_init(&espRxBuffer, RX_BUFFER_SIZE, &espRxBufferData);

	uint8_t espTxBufferData[TX_BUFFER_SIZE+2]; //nb, change this in init too //not sure whether I need that +2. Hopefully not
	circularBuffer_8bit espTxBuffer;
	circularBuffer8_init(&espTxBuffer, TX_BUFFER_SIZE, &espTxBufferData);

	uint8_t espRxPacketBufferData[RX_PACKET_CONTENT_MAX_SIZE*RX_PACKET_MAX_COUNT+2]; //nb, change this in init too //not sure whether I need that +2. Hopefully not
	circularBuffer_8bit espRxPacketBuffer;
	circularBuffer8_init(&espRxPacketBuffer, RX_PACKET_CONTENT_MAX_SIZE*RX_PACKET_MAX_COUNT, &espRxPacketBufferData);

	esp8266_instance esp01;
	esp01.sendToESPbuffer = &espTxBuffer;
	esp01.receivedFromESPbuffer = &espRxBuffer;
	esp01.rxPacketBuffer = &espRxPacketBuffer;
	esp01.getCharFromESP = &esp8266_ESPToLPC;
	esp01.sendCharToESP = &esp8266_LPCToESP;
	esp8266_initalize(&esp01);
	bitbangUARTmessage("esp01addr: ");
	bitbangUARThex(&esp01,0,0);
	bitbangUARTmessage("\r\n");

	l11uxx_uart_pinSetup_unset(47, 46); //cause bootloader may have done trix
	l11uxx_uart_pinSetup(36, 37); //set up to ESP8266
	//l11uxx_uart_Send("UART HELLO!\r\n");



	l11uxx_i2c_pinSetup(15, 16);
	l11uxx_i2c_init();

	hd44780_instance i2cLCD01up;
	hd44780_instance i2cLCD01dn;

	i2cLCD01up.handlerFunction = &hd44780lcd_handler;
	i2cLCD01dn.handlerFunction = &hd44780lcd_handler;

	i2cLCD01up.I2C_addr = (uint8_t)(0x27);
	hd44780_init(&i2cLCD01up, 40, 2, 0);
	//i2cLCD01dn.I2C_pinE_offset = (uint8_t)(2); //this should not be necessary but I am desperate


	i2cLCD01dn.I2C_addr = (uint8_t)(0x27);
	hd44780_init(&i2cLCD01dn, 40, 2, 0);
	i2cLCD01dn.I2C_pinE_offset = (uint8_t)(3); //cause using BL pin as E

	//hd44780_I2CLCD_proofOfConcept(&i2cLCD01up); //does not return //this function is for when you start to blame LCD is shit

	hd44780lcd_handler(&i2cLCD01up);
	hd44780lcd_handler(&i2cLCD01dn);


	//HWTestmigrate starts here
	int lcdcursortempY=0;
	int lcdcursortempX=0;
	char *ptrForStrstr=0;



	espToLCD(&esp01, &i2cLCD01up, &i2cLCD01dn); //does not return


	//HW_test_getFlashID();
	//HW_test_uartToSPIconverter(1); //never returns (careful, it didn't find the function for some reason when I commented it out. Might need to create header
	//HW_test_lowerpower(500);

		lcd_5110_init();
			delay(100);
			lcd_5110_clear_framebuffer();
		debugOutput("ESP test\n\r");
		//l11uxx_uart_pinSetup(47, 46); //set up to CH340
			l11uxx_uart_pinSetup_unset(47, 46); //cause bootloader may have done trix
			l11uxx_uart_pinSetup(36, 37); //set up to ESP8266










			//THIS HERE NEEDS FAKED ESP
			/*l11uxx_uart_init(9600);
			//check for data
			bitbangUARTmessage("WAITING 2 DATAPACKETS\r\n");
			while(esp01.rxPacketCount < 2)esp8266_receiveHandler(&esp01); //wait until some data is get
			bitbangUARTmessage("PRINTING 3 DATAPACKETS\r\n");
			esp8266_getData(&esp01, temporaryString1);
			bitbangUARTmessage(temporaryString1);
			debugOutput("\n\r");
			esp8266_getData(&esp01, temporaryString1);
			bitbangUARTmessage(temporaryString1);
			debugOutput("\n\r");
			esp8266_getData(&esp01, temporaryString1);
			bitbangUARTmessage(temporaryString1);
			debugOutput("\n\r");*/


			//check baud for esp8266
			printSmallPercent(&i2cLCD01dn, 0*100/9);
			l11uxx_uart_init(9600);
			debugOutput("9600?\n\r");
			if(esp8266_isAlive(&esp01) == 0){
				//no response, trying smth else
				l11uxx_uart_init(115200);
				debugOutput("115200?\n\r");
				if(esp8266_isAlive(&esp01) == 0){
					//no response, trying smth else

					l11uxx_uart_init(19200);
					debugOutput("19200?\n\r");

					if(esp8266_isAlive(&esp01) == 0) debugOutput("ESP comm fail!\n\r");; //idk, massive fail
				}
			}
		printSmallPercent(&i2cLCD01dn, 1*100/9);

		//esp8266_sendCommandAndWaitOK("AT+UART?");
		esp8266_isAlive(&esp01);
		printSmallPercent(&i2cLCD01dn, 2*100/9);
		//esp8266_SWreset(&esp01);
		//l11uxx_uart_init(115200); //because SWreset
		//esp8266_isAlive(&esp01);

		esp8266_setUARTMode(&esp01, 9600, 8, 3, 0, 0);
		printSmallPercent(&i2cLCD01dn, 3*100/9);
		l11uxx_uart_init(9600);
		printSmallPercent(&i2cLCD01dn, 4*100/9);
		//esp8266_setUARTMode(&esp01, 115200, 8, 3, 0, 0);
		//l11uxx_uart_init(115200);

		//esp8266_SWreset();
		//delay(1000);
		//delay(100);
		esp8266_isAlive(&esp01);
		printSmallPercent(&i2cLCD01dn, 5*100/9);

		esp8266_setMode(&esp01, 1);
		printSmallPercent(&i2cLCD01dn, 6*100/9);
		esp8266_setCipmux(&esp01, 1); //multiple connections, yay
		printSmallPercent(&i2cLCD01dn, 7*100/9);


		while( esp8266_joinAP(&esp01, WIFI_SSID, WIFI_PASSWD) != 0) bitbangUARTmessage("Trying wificonnect again\n\r");;

		printSmallPercent(&i2cLCD01dn, 8*100/9);

		bitbangUARThex(temporaryString1,3,8);
		bitbangUARTmessage("Cipstatus response request\n\r");
		esp8266_sendCommandAndReadResponse(&esp01, "AT+CIPSTATUS", temporaryString1); //this line gets response nicely to string
		bitbangUARTmessage("Cipstatus response occurred\n\r");
		bitbangUARThex(temporaryString1,3,8);
		bitbangUARTmessage(temporaryString1);
		//debug = (int)(temporaryString1);
		printSmallPercent(&i2cLCD01dn, 9*100/9);
		esp8266_getOwnIP(&esp01, temporaryString1);
		bitbangUARTmessage(temporaryString1);

		hd44780_clear(&i2cLCD01up);
		hd44780_clear(&i2cLCD01dn);
		hd44780_lcdcursor(&i2cLCD01up, 18, 1);
		hd44780_printtext(&i2cLCD01up, "IP:");
		hd44780_lcdcursor(&i2cLCD01dn, 8, 0);
		hd44780_printtext(&i2cLCD01dn, temporaryString1);
		hd44780_printtext(&i2cLCD01dn, ":6666");

		esp8266_openConnection(&esp01, 0, "UDP", "192.168.173.1", 6666);
		while(1){
			//get packet and handle LCD accordingly
			while(esp01.rxPacketCount < 1)esp8266_receiveHandler(&esp01); //wait until some data is get
			//esp8266_sendData(&esp01, 0, 10, "PACKET GET");
			esp8266_getData(&esp01, temporaryString1, &i, &j);
			bitbangUARTmessage(temporaryString1);
			ptrForStrstr = 0;
			ptrForStrstr = strstr(temporaryString1, "LCDCLR");
			if(ptrForStrstr){
				//packet contains LCD clear request
				hd44780_clear(&i2cLCD01up);
				hd44780_clear(&i2cLCD01dn);
			}
			ptrForStrstr = 0;
			ptrForStrstr = strstr(temporaryString1, "LCDCRS:");
			if(ptrForStrstr){
				//packet contains LCD cursor location, e.g. "LCDCRS:01,03";
				strcpy(temporaryString2, ptrForStrstr+7);
				lcdcursortempX = atoi(temporaryString2);
				strcpy(temporaryString2, ptrForStrstr+7+3);
				lcdcursortempY = atoi(temporaryString2);
				//lcdcursortempY =
				if(lcdcursortempY<=1) hd44780_lcdcursor(&i2cLCD01up, lcdcursortempX, lcdcursortempY);
				else hd44780_lcdcursor(&i2cLCD01dn, lcdcursortempX, (lcdcursortempY-2));
			}
			ptrForStrstr = 0;
			ptrForStrstr = strstr(temporaryString1, "LCDTXT:");
			if(ptrForStrstr){
				//packet contains LCD text data
				if (lcdcursortempY <= 1) hd44780_printtext(&i2cLCD01up, (ptrForStrstr+7));
				else hd44780_printtext(&i2cLCD01dn, (ptrForStrstr+7));
			}
		}

		//HWtestmigrate ends here





		debugOutput("\n\r");
		esp8266_getOwnMAC(&esp01, temporaryString1);
		//delay(2000);

		//esp8266_connection_instance homeLink;
		//esp8266_connLayer_init(&esp01, &homeLink, "UDP", "192.168.1.166", 6666);



		//attempt to get UDP connection somewhere. NECESSARY FOR RECEIVE ONLY TOO
		//esp8266_openConnection(&esp01, 0, "UDP", "192.168.1.166", 6666);

		esp8266_openConnection(&esp01, 1, "TCP", "192.168.173.1", 6667);
		esp8266_sendData(&esp01, 0, 9, "DATA PLS.");
		esp8266_sendData(&esp01, 1, 10, "TCP HELLO!");
		//check for data
		bitbangUARTmessage("WAITING 2 DATAPACKETS\r\n");
		while(esp01.rxPacketCount < 2)esp8266_receiveHandler(&esp01); //wait until some data is get
		bitbangUARTmessage("PRINTING 3 DATAPACKETS\r\n");
		esp8266_getData(&esp01, temporaryString1, &i, &j);
		bitbangUARTmessage(temporaryString1);
		debugOutput("\n\r");
		esp8266_getData(&esp01, temporaryString1, &i, &j);
		bitbangUARTmessage(temporaryString1);
		debugOutput("\n\r");
		esp8266_getData(&esp01, temporaryString1, &i, &j);
		bitbangUARTmessage(temporaryString1);
		debugOutput("\n\r");

		esp8266_sendData(&esp01, 0, 9, "GOT DATA!");

		esp8266_leaveAP(&esp01);
		esp8266_sendCommandAndReadResponse(&esp01, "AT+CIPSTATUS", temporaryString1);
		delay(2000);
		esp8266_sendCommandAndReadResponse(&esp01, "AT+RST", temporaryString1);
		delay(3000);

		//l11uxx_uart_spewBuffer();
		bitbangUARTmessage("--PROGRAM FINISHED--\n\r");
	while(1); //I don't want to continue.

	HW_test_lcd_5110_welcome();



	//debugOutput("\r\nAyy lmao!\n\r");
	debugOutput("L1UDEV001A starting.\n\n\r");
	delay(500);
	debugOutput("Console output enabled.\n\r");
	delay(500);
	debugOutput("Brought to you by\n\r");
	delay(500);
	debugOutput("Jens Dender\n\r");
	delay(500);
	debugOutput("Productions.\n\r");








	HW_test_uart0_loopback(); //no return
	flash_service_routine();
	//l11uxx_adc_init(char adcNumber, char freerunning, char clkdiv, char bits)
	l11uxx_adc_pinSetup(32);
	l11uxx_adc_init(0, 1,  10, 10);

	//while(1) HW_test_ADC();



	debugOutput("LCD activity\n\r");
	 //HW_test_lcd_5110();
	HW_test_lcd_5110_with_uptime(); //no return
	//HW_test_ILI9341();
	debugOutput("LCD activity done\n\r");



		while(1); //I don't want to continue.


		GPIOSetValue(1, 13, 1);
		GPIOSetValue(1, 14, 0);



























	//guitarpants pin 1_24 - 32B0 (pin 21), mat0

	l11uxx_timer_pinSetup(21);
	l11uxx_timer_initPWM(32, 0, 0, 0);
	LPC_CT32B0->MCR = (0x01<<10); //no interrupt, reset on MR3 -----//stop on MR0 match. Interrupt, no reset
	//l11uxx_timer_setPWM(32, 0, 3, 1023); //limit to 10bit?
	l11uxx_timer_setPWM(32, 0, 3, 255); //limit to 8bit
	//LPC_CT32B0->MR3 = 1023; //limit to 10bit?
	LPC_CT32B0->TC = 0;
	l11uxx_timer_setPWM(32, 0, 0, 0x1FF);








	while(1){
		/*l11uxx_timer_setPWM(32, 0, 0, 0x00);
		GPIOSetValue(1, 14, 0);
		delay(500);
		l11uxx_timer_setPWM(32, 0, 0, 0x1FF);
		GPIOSetValue(1, 14, 1);
		delay(500);
		l11uxx_timer_setPWM(32, 0, 0, 0x3FF);
		GPIOSetValue(1, 14, 0);
		delay(500);
		l11uxx_timer_setPWM(32, 0, 0, 0x1FF);
		GPIOSetValue(1, 14, 1);
		delay(500);*/
		//delay(100);
		LPC_GPIO->CLR[1] |= (0x2000);
		l11uxx_timer_setPWM(32, 0, 0, ((l11uxx_adc_getValue(0))>>2)&0xFF);
		//delay(100);
		LPC_GPIO->SET[1] |= (0x2000);
		l11uxx_timer_setPWM(32, 0, 0, ((l11uxx_adc_getValue(0))>>2)&0xFF);


		/*GPIOSetValue(1, 14, 1);
		l11uxx_timer_setPWM(32, 0, 0, l11uxx_adc_getValue(0));

		GPIOSetValue(1, 14, 0);
		//A1
		GPIOSetValue(1, 14, 1);
		GPIOSetValue(1, 14, 0);
		//A2
		GPIOSetValue(1, 14, 1);

		turboPinLow();
		//B1
		turboPinHigh();
		turboPinLow();
		//B2
		turboPinHigh();

		LPC_GPIO->CLR[1] |= (0x2000);
		//C1
		LPC_GPIO->SET[1] |= (0x2000);
		LPC_GPIO->CLR[1] |= (0x2000);
		//C2
		LPC_GPIO->SET[1] |= (0x2000);

		//ohhoo, v6rdusm2rk ei t88ta!
		LPC_GPIO->CLR[1] = (0x2000);
		//D1
		LPC_GPIO->SET[1] = (0x2000);
		LPC_GPIO->CLR[1] = (0x2000);
		//D2
		LPC_GPIO->SET[1] = (0x2000);

		l11uxx_timer_setPWM(32, 0, 0, l11uxx_adc_getValue(0));
		GPIOSetValue(1, 14, 0);*/
	}





}
