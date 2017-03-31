/*
 * L1UDEV001A_HW_tests.c
 *
 *  Created on: 18.01.2017
 *      Author: Denry
 *
 *  Basically all crap that works in "main.c" (L1UDEV001A.c)
 *  is thrown here to keep the "main.c" clean. Might be
 *  helpful when trying to get stuff working. All
 *  "test functions" shall have a small mention of their
 *  prerequisites (e.g. "set spi/gpio to w/e")
 */

#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif

#include <stdlib.h>

void HW_test_debugmessage(char text[]){
	//comment this out if you do not want to
	//use UART in HW tests where it is not critical
	//l11uxx_uart_Send(text);
	return;
}

void HW_test_uart0_loopback(){
	//config uart pins, setup uart speed
	//for example:
	//l11uxx_uart_init(115200);
	//l11uxx_uart_pinSetup(47, 46); //set up to CH340
	l11uxx_uart_Send("UART loopback is go\n\r");
	while(1){
		while ((LPC_USART->LSR & 0x01)){ //while data available
			LPC_USART->THR = LPC_USART->RBR; //send back whatever you find
		}
	}

	return; //it never returns...
}


//int HW_test_getFlashID(){ //incredibly unfinished
//	//SPI pins, for example
//	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0); //works well for 320x240rgblcd & ext flash & nokiaLCD
//	const int flashIDbytes = 10;
//	char flashID[flashIDbytes];
//	//flash25P10_init();
//	//flashID[0]=flash25P10_readStatusRegister();
//
//
//
//	//return &flashID;
//	return 0;
//}

//void HW_test_adjpsu(){
//	//sets
//
//}


//void HW_test_flashDump(){ //not sure where to dump, uart?
//
//}

void HW_test_ADC(){
	//reads back all ADC pin values and spews them to debugmessage
	//ADC must be set up
		//l11uxx_adc_init(char adcNumber, char freerunning, char clkdiv, char bits)
	//for example:
	//l11uxx_adc_pinSetup(32);
	//l11uxx_adc_init(0, 1,  10, 10);
	//and so forth for all channels

	char tempstring[100];
	char tempstring2[100];
	int i;
	int adcval;

	//forget about this init thing, imma do it here
	char ADCpin[8] = {32, 33, 34, 35, 40, 30, 42};
	for(i=0; i<=7; i++) l11uxx_adc_pinSetup(ADCpin[i]);
	for(i=0; i<=7; i++) l11uxx_adc_init(i, 1,  10, 10);

	strcpy(tempstring, "ADC values: ");
	//strcat(tempstring, "0");
	for(i=0; i<=7; i++){
		adcval = l11uxx_adc_getValue(i);

		itoa(adcval, tempstring2, 10);
		//itoa(123, tempstring2, 10);
		if(adcval<1000) strcat(tempstring, "0");
				if(adcval<100) strcat(tempstring, "0");
				if(adcval<10) strcat(tempstring, "0");
		strcat(tempstring, tempstring2);
		strcat(tempstring, "; ");
	}
	strcat(tempstring, " (0; ...; 7)\n\r");
	HW_test_debugmessage(tempstring);
	return;
}

//void HW_test_flash(){
//	//dumps onboardflash contents
//
//}

void HW_test_lcd_5110(){
	//spi needs to be set up. One working example:
	//l11uxx_spi_pinSetup(1, 38, 26, 13);
	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0);

	//lcd5110 testsection start
	lcd_5110_init();
	HW_test_debugmessage(" 5110 init done\n\r");
	delay(100);
	lcd_5110_clear_framebuffer();
	HW_test_debugmessage(" 5110 framebuffer clear\n\r");
	lcd_5110_printString(0,0, "luv <3");
	lcd_5110_printString(2,2, "Kitties");
	lcd_5110_printString(6,3, "b");
	lcd_5110_printString(4,4, "cute!");
	//lcd_5110_printString(0,0, "EEEEEE");
	HW_test_debugmessage(" 5110 framebuffer setup\n\r");
	lcd_5110_redraw();
	HW_test_debugmessage(" 5110 redrawn\n\r");

	//symbolism start
	//you might want to comment out "symbolism" when
	//doing some public demo
	lcd_5110_setAddressX(0);
	lcd_5110_setAddressY(0);
	lcd_5110_arc(41,10,10,200,160,1);
	lcd_5110_arc(26,40,7,0,360,1);
	lcd_5110_arc(56,40,7,0,360,1);
	lcd_5110_line_polar(41, 10, 10, 3, 0, 1);
	//lcd_5110_line(30,0,30,23,1); //fix this line thing
	lcd_5110_line_polar(35, 18, 0, 20, 180, 1);
	lcd_5110_line_polar(47, 18, 0, 20, 180, 1);
	lcd_5110_redraw();
	//symbolism end
	//lcd5110 testsection end

		return;
}


void HW_test_lcd_5110_welcome(){
	//spi needs to be set up. One working example:
	//l11uxx_spi_pinSetup(1, 38, 26, 13);
	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0);

	int i;
	//lcd5110 testsection start
	GPIOSetValue(1, 27, 1); //BL off
	GPIOSetDir(1, 27, 1);
	GPIOSetValue(1, 27, 1); //BL off
	lcd_5110_init();
	lcd_5110_clear_framebuffer();
	lcd_5110_redraw();
	delay(1000);

	GPIOSetValue(1, 13, 0);
	delay(500);
	GPIOSetValue(1, 14, 0);
	delay(500);
	//fade backlight in
	for(i=0; i<1000; i++){
		delay_us(i);
		GPIOSetValue(1, 27, 1); //BL off
		delay_us(1000-i);
		GPIOSetValue(1, 27, 0); //BL on


	}
	delay(500);

	GPIOSetValue(1, 27, 0); //BL on

	//blink backlight off twice
	delay(100);
	GPIOSetValue(1, 27, 1); //BL off
	delay(20);
	GPIOSetValue(1, 27, 0); //BL on
	delay(100);
	GPIOSetValue(1, 27, 1); //BL off
	delay(20);
	GPIOSetValue(1, 27, 0); //BL on
	delay(200);
	//enable other leds

	//Power: Online
	//L1UDEV001A starting
	//...........
	//
	//random loadings
	//blank

	//Welcome to the future.
	//	-JDP
	lcd_5110_printAsConsole("Power: Online\n\r", 1);
	lcd_5110_redraw();
	delay(500);
	lcd_5110_printAsConsole("L1UDEV boot.\n\r", 1);
	delay(500);
	for(i=0; i<13; i++){
		lcd_5110_printAsConsole(".", 1);
		delay(100);

	}

	lcd_5110_printAsConsole("\n\r", 1);
	lcd_5110_printAsConsole("ROM check:", 1);
	delay(500);
	lcd_5110_printAsConsole(" OK\n\r", 1);
	delay(300);
	lcd_5110_printAsConsole("Setting \n\rnodes:", 1);
	delay(250);
	l11uxx_uart_pinSetup(36, 37); //set up to ESP8266
	l11uxx_uart_init(115200);
	l11uxx_uart_Send("AT+RST\n\r");
	delay(250);
	l11uxx_uart_Send("AT+RST\n\r");
	lcd_5110_printAsConsole(" OK\n\r", 1);
	delay(300);
	lcd_5110_printAsConsole("Initializing \n\rtables:", 1);
	delay(500);
	lcd_5110_printAsConsole(" OK\n\r", 1);
	delay(300);

	lcd_5110_printAsConsole("\n\r", 1);
	lcd_5110_printAsConsole("    |", 0);
	lcd_5110_printString(9, 5, "|");
	for(i=0; i<4; i++){
			lcd_5110_printAsConsole("=", 1);
			delay(700);

		}

	delay(500);
	lcd_5110_clear_framebuffer();
	lcd_5110_redraw();
	delay(1000);


	lcd_5110_consolePosition_jump(0,0);
	lcd_5110_printAsConsole("Welcome.\n\r", 1);
	delay(1000);
	lcd_5110_printAsConsole("This is ", 1);
	lcd_5110_printAsConsole("\n\r", 0);
	lcd_5110_printAsConsole("the future.\n\r", 1);
	delay(500);
	lcd_5110_printAsConsole("          -JDP\n\r", 1);

	delay(1000);

	lcd_5110_printString(0, 5, ">");
	lcd_5110_redraw();
	while(1){
		lcd_5110_printString(1, 5, "_");
		lcd_5110_redraw();
		delay(500);
		lcd_5110_printString(1, 5, "    ");
		lcd_5110_redraw();
		delay(500);
	}
		return;
}

void HW_test_lcd_5110_with_uptime(){
	//spi needs to be set up. One working example:
	//l11uxx_spi_pinSetup(1, 38, 26, 13);
	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0);
	HW_test_lcd_5110();

	char temporaryString1[40], temporaryString2[40];
	int i=0;
	unsigned long uptime=0;
	while(1){
		//if(GPIOGetValue(0, 2))GPIOSetValue(1, 14, 0);
		//else if(GPIOGetValue(0, 7))GPIOSetValue(1, 14, 0);
		//else GPIOSetValue(1, 14, 1);
		i++;
		if(i>1){
			//	GPIOSetValue(1, 13, 1);
			i=0;
			uptime++;
		} //else GPIOSetValue(1, 13, 0);
		delay(250);
		GPIOSetValue(1, 27, 0);
		delay(250);
		GPIOSetValue(1, 27, 1);

		strcpy(temporaryString1, "");
		if(uptime>4294967294)uptime=0;
		if(uptime<1000000000) strcat(temporaryString1, "0");
		if(uptime<100000000) strcat(temporaryString1, "0");
		if(uptime<10000000) strcat(temporaryString1, "0");
		if(uptime<1000000) strcat(temporaryString1, "0");
		if(uptime<100000) strcat(temporaryString1, "0");
		if(uptime<10000) strcat(temporaryString1, "0");
		if(uptime<1000) strcat(temporaryString1, "0");
		if(uptime<100) strcat(temporaryString1, "0");
		if(uptime<10) strcat(temporaryString1, "0");
		uitoa(uptime, temporaryString2,10); //lol itoa makes it to signed int, uitoa makes to unsigned
		strcat(temporaryString1, temporaryString2);
		lcd_5110_printString(2,0, temporaryString1);
		lcd_5110_redraw();
	}
	return;
}


void HW_test_ILI9341(){
	//enjoy, this takes like 9 weeks for it to finish
	//spi needs to be set up. One working example:
	//l11uxx_spi_pinSetup(1, 38, 26, 13);
	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0);
	//spi needs to be powered and clocked obv (1x division @ 48mhz is no probs)

	//note for troubleshooting: if LCD remains black
	//verify that lcd BL bugfix is done on L1UDEV001A

	int i, j;

	//ILI9341 testsection start
	ILI9341_GPIO_init();
	ILI9341_begin();

	ILI9341_drawPixel(10, 10, 0); //black????
	ILI9341_drawPixel(12, 12, 0xFF);
	ILI9341_drawPixel(14, 14, 0xFFFF); //hellawhite

		i=0;
		while(i<5){
			while(j<10){
				//ILI9341_drawPixel(j+22, i+10, 0xFFFF);
				ILI9341_drawPixel(j+22, i+10, 0x00FF);
				j++;
			}
			i++;
			j=0;
		}

		ILI9341_drawFastVLine(10, 150, 100, 0x0FF0);
		ILI9341_drawFastVLine(11, 150, 100, 0x0FF0);
		ILI9341_drawFastVLine(12, 150, 100, 0x0FF0);
		ILI9341_drawFastVLine(13, 150, 100, 0x0FF0);
		ILI9341_drawFastVLine(14, 150, 100, 0x0FF0);

		//ILI9341_fillScreen(ILI9341_Color565(0,0,255));
		//ILI9341_fillScreen(ILI9341_Color565(0,0,0));



		ILI9341_fillRect(0, 0, 20, 20, ILI9341_Color565(255,0,0));
		ILI9341_fillRect(10, 0, 10, 15, ILI9341_Color565(0,255,0));
		ILI9341_fillRect(0, 0, 10, 10, ILI9341_Color565(0,0,255));
		const unsigned char navyPasta[] = {"What the fuck did you just fucking say about me, you little bitch? I’ll have you know I graduated top of my class in the Navy Seals, and I’ve been involved in numerous secret raids on Al-Quaeda, and I have over 300 confirmed kills. I am trained in gorilla warfare and I’m the top sniper in the entire US armed forces. You are nothing to me but just another target. I will wipe you the fuck out with precision the likes of which has never been seen before on this Earth, mark my fucking words. You think you can get away with saying that shit to me over the Internet? Think again, fucker. As we speak I am contacting my secret network of spies across the USA and your IP is being traced right now so you better prepare for the storm, maggot. The storm that wipes out the pathetic little thing you call your life. You’re fucking dead, kid. I can be anywhere, anytime, and I can kill you in over seven hundred ways, and that’s just with my bare hands. Not only am I extensively trained in unarmed combat, but I have access to the entire arsenal of the United States Marine Corps and I will use it to its full extent to wipe your miserable ass off the face of the continent, you little shit. If only you could have known what unholy retribution your little “clever” comment was about to bring down upon you, maybe you would have held your fucking tongue. But you couldn’t, you didn’t, and now you’re paying the price, you goddamn idiot. I will shit fury all over you and you will drown in it. You’re fucking dead, kiddo."};

		for(i=0; i<40; i++){
			ILI9341_printString_bg(0, i*8, ILI9341_Color565(0,0,0), ILI9341_Color565(255,255,255), &navyPasta[40*i]);
			ILI9341_drawFastHLine(0, i*8+7, 240, ILI9341_Color565(255,255,255));
		}

		ILI9341_fillRect(100, 100, 50, 50, ILI9341_Color565(255,0,0));
		ILI9341_fillRect(100, 150, 50, 50, ILI9341_Color565(0,255,0));
		ILI9341_fillRect(100, 200, 50, 50, ILI9341_Color565(0,0,255));
		drawPokey(160, 120);
	//ILI9341 testsection end
		return;
}

void HW_test_uartToSPIconverter(int SPInumber){
	//setup SPI & uart first, e.g.
	//l11uxx_spi_pinSetup(1, 38, 26, 13);
	//l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0); //works well for 320x240rgblcd & ext flash & nokiaLCD
	//l11uxx_uart_pinSetup(47, 46); //set up to CH340
	//l11uxx_uart_init(9600);


	//send commands like "CS1H 0x00DA 0x00AF CS1L \r\n" over UART. All that is read back, is directly spewed out (also as hex)

	l11uxx_uart_clearRxBuffer();
	char *okResponse = 0;
	int rxBufferLocalWaypoint = 0;
	extern char l11uxx_uart_rx_buffer;
	volatile extern int l11uxx_uart_rx_buffer_current_index;
	int stringHandlingDone = 0;
	int dataValue = 0;
	int conversionBuffer[40];

	const int indicate_transfer 	= 1; 	// Blinks some gpio while transferring

	const int CS0_PORT 	= 1; 	// SPI1_SSEL
	const int CS0_PIN 	= 19; 	// SPI1_SSEL
	const int CS1_PORT 	= 1; 	// Flash CS
	const int CS1_PIN 	= 25; 	// Flash CS

	GPIOSetDir(CS0_PORT, CS0_PIN, 1);
	GPIOSetDir(CS1_PORT, CS1_PIN, 1);
	GPIOSetValue(CS0_PORT, CS0_PIN, 1);
	GPIOSetValue(CS1_PORT, CS1_PIN, 1);
	if(indicate_transfer) GPIOSetDir(1, 27, 1);
	if(indicate_transfer) GPIOSetValue(1, 27, 1); //BL off

	while(1){
		okResponse = 0;
		stringHandlingDone = 0;
		rxBufferLocalWaypoint = 0;
		l11uxx_uart_clearRxBuffer();
		while(okResponse == 0){
			while ((LPC_USART->LSR & 0x01)) l11uxx_uart_sendToBuffer(); //anything in HW buffer, slap it to SW buffer
			okResponse = strstr(&l11uxx_uart_rx_buffer, "\n"); //finish with \r\n
		}
		if(indicate_transfer) GPIOSetValue(1, 27, 0); //BL on
		while (stringHandlingDone == 0){
			/*if((*(&l11uxx_uart_rx_buffer_current_index+rxBufferLocalWaypoint)) == "CS0H") GPIOSetValue(CS0_PORT, CS0_PIN, 1);
			else if((*(&l11uxx_uart_rx_buffer_current_index+rxBufferLocalWaypoint)) == "CS0L") GPIOSetValue(CS0_PORT, CS0_PIN, 0);
			else if((*(&l11uxx_uart_rx_buffer_current_index+rxBufferLocalWaypoint)) == "CS1H") GPIOSetValue(CS1_PORT, CS1_PIN, 1);
			else if((*(&l11uxx_uart_rx_buffer_current_index+rxBufferLocalWaypoint)) == "CS1L") GPIOSetValue(CS1_PORT, CS1_PIN, 0);
			else if((*(&l11uxx_uart_rx_buffer_current_index+rxBufferLocalWaypoint)) == "0x"){
				l11uxx_spi_sendByte(SPInumber, strtol((*(&l11uxx_uart_rx_buffer_current_index+rxBufferLocalWaypoint)), NULL, 16));
				//while ((LPC_SSP0->SR & (0x1 << 2))); //wait until data available
				dataValue = LPC_SSP0->DR;

				l11uxx_uart_Send("0x");
				itoa(dataValue, conversionBuffer, 16);
				l11uxx_uart_Send(conversionBuffer);
			}
			else if((*(&l11uxx_uart_rx_buffer_current_index+rxBufferLocalWaypoint)) == '\n') stringHandlingDone = 1;
			else if((*(&l11uxx_uart_rx_buffer_current_index+rxBufferLocalWaypoint)) == 0) stringHandlingDone = 1;*/

			if(  strncmp(((&l11uxx_uart_rx_buffer+rxBufferLocalWaypoint)),"CS0H", 4) == 0  ) GPIOSetValue(CS0_PORT, CS0_PIN, 1);
			else if(  strncmp(((&l11uxx_uart_rx_buffer+rxBufferLocalWaypoint)),"CS0L", 4) == 0  ) GPIOSetValue(CS0_PORT, CS0_PIN, 0);
			else if(  strncmp(((&l11uxx_uart_rx_buffer+rxBufferLocalWaypoint)),"CS1H", 4) == 0  ){
				GPIOSetValue(CS1_PORT, CS1_PIN, 1);
				//l11uxx_uart_Send("FLH");
			}
			else if(  strncmp(((&l11uxx_uart_rx_buffer+rxBufferLocalWaypoint)),"CS1L", 4) == 0  ){
				GPIOSetValue(CS1_PORT, CS1_PIN, 0);
				//l11uxx_uart_Send("FLL");
			}

			else if(  strncmp(((&l11uxx_uart_rx_buffer+rxBufferLocalWaypoint)),"0x", 2) == 0  ) {
				dataValue = strtol( ((&l11uxx_uart_rx_buffer+rxBufferLocalWaypoint)), NULL, 16);
				l11uxx_spi_sendByte(SPInumber, dataValue);

				if(SPInumber == 0){
//					while ((LPC_SSP0->SR & (0x1 << 2))); //wait until data available
					dataValue = LPC_SSP0->DR;
				} else {
//					while ((LPC_SSP1->SR & (0x1 << 2))); //wait until data available
					dataValue = LPC_SSP1->DR;
				}
				l11uxx_uart_Send("0x");
				itoa(dataValue, conversionBuffer, 16);
				l11uxx_uart_Send(conversionBuffer);
				l11uxx_uart_Send(" ");
			}



			else if(strncmp(((&l11uxx_uart_rx_buffer+rxBufferLocalWaypoint)),"\n", 1) == 0  ) stringHandlingDone = 1;
			else if((*(&l11uxx_uart_rx_buffer+rxBufferLocalWaypoint)) == 0) stringHandlingDone = 1;
			//else //unknown situation

//			strcpy(conversionBuffer, (((&l11uxx_uart_rx_buffer))));
//			l11uxx_uart_Send(conversionBuffer);

			rxBufferLocalWaypoint++;

		}
		l11uxx_uart_Send("\r\n");
		if(indicate_transfer) GPIOSetValue(1, 27, 1); //BL off

	}



	return; //never returns
}
