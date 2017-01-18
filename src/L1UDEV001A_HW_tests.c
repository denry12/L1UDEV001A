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

void HW_test_debugmessage(char text[]){
	//comment this out if you do not want to
	//use UART in HW tests where it is not critical
	l11uxx_uart_Send(text);
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

//void HW_test_adjpsu(){
//	//sets
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
