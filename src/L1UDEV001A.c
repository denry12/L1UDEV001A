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


//hwtests

#include "JDP_wifi_creds.h" //NB! You do not have this file. It just overwrites next two defines
#ifndef WIFI_SSID
#define WIFI_SSID "4A50DD"
#endif
#ifndef WIFI_PASSWD
#define WIFI_PASSWD "2444666668888888"
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

void setup48MHzInternalClock(){
		//MAKES LPC GO SOVERYFAST
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
}

void setupClocks(){
	GPIOSetValue(1, 13, 0);
	GPIOSetValue(1, 14, 1);

	//LPC_SYSCON->PDRUNCFG |= (1<<5); //power down xtal osc
	LPC_SYSCON->SYSOSCCTRL |= 0x01;

	LPC_SYSCON->PDRUNCFG &= (~(1<<4)); //power up ADC
	LPC_SYSCON->PDRUNCFG &= (~(1<<6)); //power up WDT osc
	//page 30 of usermanual
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<7); //enable clock to CT16B0
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<8); //enable clock to CT16B1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<9); //enable clock to CT32B0
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<10); //enable clock to CT32B1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<15); //enable clock to WDT
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<18); //enable clock to SSP1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<12); //enable clock to USART
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<13); //enable clock to ADC
//	LPC_SYSCON->SSP1CLKDIV = 1; //SSP1 clock divider
	LPC_SYSCON->SSP1CLKDIV = 8; //SSP1 clock divider
	LPC_SYSCON->PRESETCTRL |= (1 << 2); //remove reset from SSP1


	LPC_SYSCON->UARTCLKDIV = 1; //USART clock divider


	/*GPIOSetValue(1, 13, 0);
	GPIOSetValue(1, 14, 1);*/
	return;
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


bool esp8266_LPCToESP(esp8266_instance instance){
	int response;

	//if there are chars waiting in buffer (not returning <0 (e.g. -1))
	response = esp8266_charFromBufferToUart(instance);
	if (response < 0) return 1;

	//char placeholder = 'X';
	//placeholder = response & 0xFF;

	//send it to UART (if UART not busy?)
	//todo: REPLACE IT WITH "IF UART FULL" if possible
	if (!(LPC_USART->LSR & (1<<5))){  //THRE: Transmitter Holding Register Empty.
				                                        // 0: THR contains valid data, 1: THR is empty
			LPC_USART->THR = (response & 0xFF);
	}
	return 0;
}

bool esp8266_ESPToLPC(esp8266_instance instance){

	volatile extern int l11uxx_uart_rx_buffer_current_index;
	extern char *l11uxx_uart_rx_buffer;

	//if data available in UART buffer
	if ((l11uxx_uart_rx_buffer[0] == 0) && (l11uxx_uart_rx_buffer_current_index == 0))
		return 1; //no data, return


	int currentIndex, maxIndex = l11uxx_uart_rx_buffer_current_index;

	//stop interrupts
	NVIC_DisableIRQ(UART_IRQn);

	//copy data out and prep for new data
	char temporaryBuffer[l11uxx_uart_rx_buffer_current_index];
	memcpy(temporaryBuffer, l11uxx_uart_rx_buffer, l11uxx_uart_rx_buffer_current_index);
	l11uxx_uart_clearRxBuffer();

	//re-enable interrupts
	NVIC_EnableIRQ(UART_IRQn);

	//handle data, send it to ESP buffer
	while (currentIndex <= maxIndex){
		if(esp8266_charFromUartToBuffer(instance, temporaryBuffer[currentIndex]) == 0)
			currentIndex++;
	}

	return 0;
}

int main(void) {

	//printf("Hello, this is app.");
 	GPIOSetDir(0, 2, 0); //set input
	GPIOSetDir(0, 7, 0); //set input
	GPIOSetDir(0, 20, 1);
	GPIOSetValue(0, 20, 1); //turn off adjpsu
	GPIOSetDir(1, 13, 1);
	GPIOSetDir(1, 14, 1);
	GPIOSetDir(1, 27, 1);

	GPIOSetDir(0, 17, 1); //I use it for debug
	GPIOSetValue(0, 17, 0);


	setupClocks();
	setup48MHzInternalClock(); //gotta go fast


	l11uxx_spi_pinSetup(1, 38, 26, 13);
	l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0); //works well for 320x240rgblcd & ext flash & nokiaLCD
	//l11uxx_spi_init(1, 8, 0, 0, 1, 0, 0, 0);
	//l11uxx_spi_init(1, 8, 0, 0, 0, 0, 0, 0); //works for NRF (and rgb lcd?)
	//l11uxx_spi_init(1, 8, 0, 1, 0, 0, 0, 0);
	//l11uxx_spi_init(int SPINumber, int bits, int FRF, int CPOL, int CPHA, int SCR, int MS, int CPSDVSR)

	l11uxx_uart_pinSetup(47, 46); //set up to CH340 //careful, esp is set afterwards
	l11uxx_uart_init(9600); //upping speed later mby

	esp8266_instance esp01;
	esp01.getCharFromESP = &esp8266_ESPToLPC;
	esp01.sendCharToESP = &esp8266_LPCToESP;
	esp8266_initalize(&esp01);


	//esp01.getCharFromESP();
	//esp01.sendCharToESP();

	//HW_test_getFlashID();
	HW_test_uartToSPIconverter(1); //never returns

	int i=0, j=0;
	int debug=0;

	volatile char temporaryString1[300], temporaryString2[40];


	//HW_test_lowerpower(500);



		//MAD DEBUGGERY FOR ESP8266

		extern char *l11uxx_uart_rx_buffer;
		volatile extern int l11uxx_uart_rx_buffer_current_index;
		bitbangUARTmessage("\n\r------------------------------------------------\n\r");
		bitbangUARTmessage("RxBSt: 0x");
		itoa(&l11uxx_uart_rx_buffer, temporaryString1, 16);
		bitbangUARTmessage(temporaryString1);
		bitbangUARTmessage("; RxIn: 0x");
		itoa(l11uxx_uart_rx_buffer_current_index, temporaryString1, 16);
		bitbangUARTmessage(temporaryString1);
		bitbangUARTmessage("; RxBEn: 0x");
		itoa((&l11uxx_uart_rx_buffer+l11uxx_uart_rx_buffer_current_index), temporaryString1, 16);
		bitbangUARTmessage(temporaryString1);
		bitbangUARTmessage("\n\r");

		//bitbangUARTmessage("  ,  -,  -,  -,  -,  -,  -,  -\n\r");
		//bitbangUARTmessage("  ");
		//bitbangUARTint(65530, 3, 8);
		//bitbangUARThex(65530, 3, 8);
		//bitbangUARTbin(65530, 3, 32);
		//bitbangUARTmessage("  ,  -,  -,  -,  -,  -,  -,  -,  -,  -,  -,  -,  -\n\r");

	//l11uxx_uart_Send("!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r");
		lcd_5110_init();
			delay(100);
			lcd_5110_clear_framebuffer();
		debugOutput("ESP test\n\r");
		//l11uxx_uart_pinSetup(47, 46); //set up to CH340
			l11uxx_uart_pinSetup_unset(47, 46); //cause bootloader may have done trix
			l11uxx_uart_pinSetup(36, 37); //set up to ESP8266
		//l11uxx_uart_init(9600);


			//check baud for esp8266
			l11uxx_uart_init(9600);
			debugOutput("9600?\n\r");
			if(esp8266_isAlive() == 0){
				//no response, trying smth else
				l11uxx_uart_init(115200);
				debugOutput("115200?\n\r");
				if(esp8266_isAlive() == 0){
					//no response, trying smth else

					l11uxx_uart_init(19200);
					debugOutput("19200?\n\r");

					if(esp8266_isAlive() == 0) debugOutput("ESP comm fail!\n\r");; //idk, massive fail
				}
			}


		//esp8266_sendCommandAndWaitOK("AT+UART?");
		esp8266_isAlive();
		esp8266_SWreset();
		l11uxx_uart_init(115200); //because SWreset
		esp8266_isAlive();

		//esp8266_setUARTMode(9600, 8, 3, 0, 0);
		//l11uxx_uart_init(9600);
		esp8266_setUARTMode(115200, 8, 3, 0, 0);
		l11uxx_uart_init(115200);

		//esp8266_SWreset();
		//delay(1000);
		//delay(100);
		esp8266_isAlive();

		esp8266_joinAP("4A50DD","2444666668888888");

		bitbangUARThex(temporaryString1,3,8);
		bitbangUARTmessage("Cipstatus response request\n\r");
		esp8266_sendCommandAndReadResponse("AT+CIPSTATUS", temporaryString1); //this line gets response nicely to string
		bitbangUARTmessage("Cipstatus response occurred\n\r");
		bitbangUARThex(temporaryString1,3,8);
		bitbangUARTmessage(temporaryString1);
		//debug = (int)(temporaryString1);
		esp8266_getOwnIP(temporaryString1);
		bitbangUARTmessage(temporaryString1);
		debugOutput("\n\r");
		esp8266_getOwnMAC(temporaryString1);
		delay(2000);
		esp8266_leaveAP();
		esp8266_sendCommandAndReadResponse("AT+CIPSTATUS", temporaryString1);
		delay(2000);
		esp8266_sendCommandAndReadResponse("AT+RST", temporaryString1);
		delay(3000);

		l11uxx_uart_spewBuffer();

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
