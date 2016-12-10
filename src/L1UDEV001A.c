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
	LPC_CT16B0->PR = 11; //16,000,000 / 16 = 1us. Incremented every PCLK.
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



void setupClocks(){
	GPIOSetValue(1, 13, 0);
	GPIOSetValue(1, 14, 1);

	//LPC_SYSCON->PDRUNCFG |= (1<<5); //power down xtal osc
	LPC_SYSCON->SYSOSCCTRL |= 0x01;

	LPC_SYSCON->PDRUNCFG &= (~(1<<4)); //power up ADC
	//page 30 of usermanual
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<7); //enable clock to CT16B0
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<8); //enable clock to CT16B1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<9); //enable clock to CT32B0
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<10); //enable clock to CT32B1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<18); //enable clock to SSP1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<12); //enable clock to USART
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<13); //enable clock to ADC
//	LPC_SYSCON->SSP1CLKDIV = 1; //SSP1 clock divider
	LPC_SYSCON->SSP1CLKDIV = 8; //SSP1 clock divider
	LPC_SYSCON->PRESETCTRL |= (1 << 2); //remove reset from SSP1


	LPC_SYSCON->UARTCLKDIV = 1; //USART clock divider







	//DO NOT FORGET TO UNCOMMENT THIS IF YOU WANT FAST MCU
	//MAKES LPC GO SOVERYFAST
	/*LPC_SYSCON->PDAWAKECFG &= (~(1<<7)); //pll powered up after wake (this is a desperate attempt)

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
*/


	/*GPIOSetValue(1, 13, 0);
	GPIOSetValue(1, 14, 1);*/
	return;
}




void rxRoutine(){
	l11uxx_uart_Send("RxRoutine is go!\n\r");
	int i=0;
	int j = 0;
	int visualAssistance = 0;
	int statusReg=0;
	char temporaryString1[40], temporaryString2[40];
	int temporaryInt=0;

	/*delay(100);
			nrf24l01_pwrupEnable();
			delay(100);
			*/


			nrf24l01_writeRegister_withFrame (0x00, 0x1F); //config, rx
			delay(100);

	nrf24l01_getConfigData();
	while(1){

			//i = nrf24l01_bytesOfPayload_p0();
			GPIOSetValue(1, 14, 1);
			statusReg=nrf24l01_readRegister_withFrame(0x07); //read status register
			//while (!(statusReg & 0x40)){
			while ((nrf24l01_isRxFIFOEmpty())){
				nrf24l01_CEEnable(); //LED shines while listening
				GPIOSetValue(1, 13, 0);
				delay(1000);
				GPIOSetValue(1, 13, 1);
				nrf24l01_CEDisable();
				delay(10);


				statusReg=nrf24l01_readRegister_withFrame(0x07); //read status register
				//if(statusReg & 0x40){ //new data received
				//	nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x40)); //clear data received interrupt
				//	l11uxx_uart_Send("Data received interrupt!\n\r");
				//}
				if(nrf24l01_isRxFIFOEmpty()) l11uxx_uart_Send("Rx FIFO empty - 1 ");
				//else l11uxx_uart_Send("Rx FIFO has data - 1 \a"); //with bell! 0x07
				else l11uxx_uart_Send("Rx FIFO has data - 1");
				if (visualAssistance) l11uxx_uart_Send("|");
				else l11uxx_uart_Send("-");
				l11uxx_uart_Send("\n\r");
				if(visualAssistance != 0) visualAssistance = 0;
				else visualAssistance = 1;


				//i = nrf24l01_bytesOfPayload_p0();


			}
			/*if(nrf24l01_isRxFIFOEmpty()) l11uxx_uart_Send("Rx FIFO empty - 2 ");
			else l11uxx_uart_Send("Rx FIFO has data - 2 ");
			if (visualAssistance) l11uxx_uart_Send("|");
			else l11uxx_uart_Send("-");
			l11uxx_uart_Send("\n\r");
			if(visualAssistance != 0) visualAssistance = 0;
			else visualAssistance = 1;
			GPIOSetValue(1, 14, 1);

			 */
			//
			strcpy(temporaryString1, "                              ");
			l11uxx_uart_Send("Data: ");

			//temporaryInt=nrf24l01_isRxFIFOEmpty();


			//while(!(temporaryInt)){
			while(!(nrf24l01_isRxFIFOEmpty())){
				nrf24l01_CSEnable();
				nrf24l01_readRxPayload ();
				//while(!(nrf24l01_isRxFIFOEmpty())){
				temporaryString1[1] = 0;
				for(i=0; i<5; i++){
				temporaryString1[0] = nrf24l01_recvSPIByte();
				sprintf(&temporaryString2[0], "[0x%02x] ", temporaryString1[0]);
				l11uxx_uart_Send(&temporaryString1[0]);
				l11uxx_uart_Send(&temporaryString2[0]);
				}
				/*temporaryString1[1] = nrf24l01_recvSPIByte();
				temporaryString1[2] = nrf24l01_recvSPIByte();
				temporaryString1[3] = nrf24l01_recvSPIByte();
				temporaryString1[4] = nrf24l01_recvSPIByte();*/

				//temporaryString2[0] = temporaryString1[i];
				//temporaryString2[1] = 0;
				/*sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[0]);
				l11uxx_uart_Send(&temporaryString2[0]);
				sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[1]);
				l11uxx_uart_Send(&temporaryString2[0]);
				sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[2]);
				l11uxx_uart_Send(&temporaryString2[0]);
				sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[3]);
				l11uxx_uart_Send(&temporaryString2[0]);
				sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[4]);
				l11uxx_uart_Send(&temporaryString2[0]);*/

				//}
				nrf24l01_CSDisable();
				//i--;

			}
			l11uxx_uart_Send(".\n\r");

			if(statusReg & 0x40){ //new data received
								nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x40)); //clear data received interrupt
								l11uxx_uart_Send("Cleared data received interrupt\n\r");
							}
			//nrf24l01_flushRxBuffer ();

			delay(100);
			strcpy(temporaryString1, "                              ");
		}

	return;
}

void txRoutine(){
	int statusReg=0;
	int printfdebug = 0;
	l11uxx_uart_Send("TxRoutine is go!\n\r");
	if(printfdebug) printf("TxRoutine is go!\n\r");

	/*delay(100);
	//	nrf24l01_pwrupDisable();
		delay(100);

		delay(100);
			nrf24l01_primRXDisable();
			delay(100);



	delay(100);
	nrf24l01_setTxAddr(0x456789ABCD);
	delay(100);
	//nrf24l01_setRxAddr(0xACDC000000);
	nrf24l01_setRxAddr(0x456789ABCD); //autoack thing???

	delay(100);


	delay(100);
		nrf24l01_pwrupEnable();
		delay(100);*/


		nrf24l01_writeRegister_withFrame (0x00, 0x1E); //config, tx
			delay(100);

	GPIOSetValue(1, 13, 1);
	GPIOSetValue(1, 14, 1);
	nrf24l01_getConfigData();
	while(1){
		if(!(nrf24l01_isTxFIFOFull())){
			if(GPIOGetValue(0,2)){
			nrf24l01_CSEnable();
			nrf24l01_writeTxPayload ();
			nrf24l01_sendSPIByte(65);//nrf24l01_sendSPIByte("A");
			nrf24l01_sendSPIByte(121);//nrf24l01_sendSPIByte("y");
			nrf24l01_sendSPIByte(121);//nrf24l01_sendSPIByte("y");
			nrf24l01_sendSPIByte(32);//nrf24l01_sendSPIByte("!");
			nrf24l01_sendSPIByte(108);//nrf24l01_sendSPIByte("");
			nrf24l01_CSDisable();

			nrf24l01_CSEnable();
			nrf24l01_writeTxPayload ();
			nrf24l01_sendSPIByte(109);//nrf24l01_sendSPIByte("A");
			nrf24l01_sendSPIByte(97);//nrf24l01_sendSPIByte("y");
			nrf24l01_sendSPIByte(111);//nrf24l01_sendSPIByte("y");
			nrf24l01_sendSPIByte(33);//nrf24l01_sendSPIByte("!");
			nrf24l01_sendSPIByte(33);//nrf24l01_sendSPIByte("");
			nrf24l01_CSDisable();

			l11uxx_uart_Send("Crap to Tx FIFO!\n\r");
			delay(100);
			if(printfdebug) printf("Crap successfully in Tx FIFO!\n\r");
			}
			else if(printfdebug) printf("Skipped transmission\n\r");
		} else{
			if(printfdebug) printf("Failed to send!\n\r");
			nrf24l01_flushTxBuffer ();


			if(printfdebug) printf("Flushed buffer!\n\r");
		}

		nrf24l01_CEEnable();
		GPIOSetValue(1, 13, 0); //LED shines while transmitting
		delay(1); //"don't keep in Tx mode for more than 4ms at a time"???
		nrf24l01_CEDisable();

		delay(100); //cause my eye is slow.
		GPIOSetValue(1, 13, 1);

		statusReg=nrf24l01_readRegister_withFrame(0x07); //read status register
		while(statusReg & 0x10){
			if(printfdebug) printf("MAT_RT interrupt active\n\r"); //data send failed???
			//nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x10)); //clear interrupt
			while(statusReg & 0x10){
				nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x10)); //clear interrupt
				nrf24l01_CEEnable();
						GPIOSetValue(1, 13, 0); //LED shines while transmitting
						delay(1); //"don't keep in Tx mode for more than 4ms at a time"???
						nrf24l01_CEDisable();

						delay(100); //cause my eye is slow.
						GPIOSetValue(1, 13, 1);
						statusReg=nrf24l01_readRegister_withFrame(0x07); //read status register
			}
			if(printfdebug) printf("MAT_RT interrupt cleared\n\r");
		}	if(statusReg & 0x20){ //new data received
			if(printfdebug) printf("TX_DS interrupt active\n\r"); //All cool, data sent
			nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x20)); //clear interrupt
			if(printfdebug) printf("TX_DS interrupt cleared\n\r");
		}


		delay(1000);




	}
}

int main(void) {
	GPIOSetDir(0, 2, 0); //set input
	GPIOSetDir(0, 7, 0); //set input
	GPIOSetDir(1, 13, 1);
	GPIOSetDir(1, 14, 1);
	GPIOSetDir(1, 27, 1);

	setupClocks();


	l11uxx_spi_pinSetup(1, 38, 26, 13);
		l11uxx_spi_init(1, 8, 0, 1, 1, 0, 0, 0);
		//l11uxx_spi_init(1, 8, 0, 0, 1, 0, 0, 0);
	//l11uxx_spi_init(1, 8, 0, 0, 0, 0, 0, 0); //works for NRF (and rgb lcd?)
		//l11uxx_spi_init(1, 8, 0, 1, 0, 0, 0, 0);
		//l11uxx_spi_init(int SPINumber, int bits, int FRF, int CPOL, int CPHA, int SCR, int MS, int CPSDVSR)

	int i=0, j=0;
	unsigned long uptime=0;
	char temporaryString1[40], temporaryString2[40];
	GPIOSetValue(1, 13, 0);

	l11uxx_uart_init(115200); //currently it is always 115200
	l11uxx_uart_pinSetup(47, 46); //set up to CH340
	l11uxx_uart_Send("\r\nAyy lmao!\n\r");

	l11uxx_uart_Send("LCD activity\n\r");




	lcd_5110_init();
	l11uxx_uart_Send(" 5110 init done\n\r");
	delay(100);
		lcd_5110_clear_framebuffer();
		l11uxx_uart_Send(" 5110 framebuffer clear\n\r");
		lcd_5110_printString(0,0, "luv <3");
		lcd_5110_printString(2,2, "Kitties");
		lcd_5110_printString(6,3, "b");
		lcd_5110_printString(4,4, "cute!");
		//lcd_5110_printString(0,0, "EEEEEE");
		l11uxx_uart_Send(" 5110 framebuffer setup\n\r");
		lcd_5110_redraw();
		l11uxx_uart_Send(" 5110 redrawn\n\r");

	//ILI9341_GPIO_init();
	//ILI9341_begin();

	//ILI9341_drawPixel(10, 10, 0); //black????
	//ILI9341_drawPixel(12, 12, 0xFF);
	//ILI9341_drawPixel(14, 14, 0xFFFF); //hellawhite

//	i=0;
//	while(i<5){
//		while(j<10){
//			//ILI9341_drawPixel(j+22, i+10, 0xFFFF);
//			ILI9341_drawPixel(j+22, i+10, 0x00FF);
//			j++;
//		}
//		i++;
//		j=0;
//	}
//
//	ILI9341_drawFastVLine(10, 150, 100, 0x0FF0);
//	ILI9341_drawFastVLine(11, 150, 100, 0x0FF0);
//	ILI9341_drawFastVLine(12, 150, 100, 0x0FF0);
//	ILI9341_drawFastVLine(13, 150, 100, 0x0FF0);
//	ILI9341_drawFastVLine(14, 150, 100, 0x0FF0);
//
//	//ILI9341_fillScreen(ILI9341_Color565(0,0,255));
//	//ILI9341_fillScreen(ILI9341_Color565(0,0,0));
//
//
//
//	ILI9341_fillRect(0, 0, 20, 20, ILI9341_Color565(255,0,0));
//	ILI9341_fillRect(10, 0, 10, 15, ILI9341_Color565(0,255,0));
//	ILI9341_fillRect(0, 0, 10, 10, ILI9341_Color565(0,0,255));
//	const unsigned char navyPasta[] = {"What the fuck did you just fucking say about me, you little bitch? I’ll have you know I graduated top of my class in the Navy Seals, and I’ve been involved in numerous secret raids on Al-Quaeda, and I have over 300 confirmed kills. I am trained in gorilla warfare and I’m the top sniper in the entire US armed forces. You are nothing to me but just another target. I will wipe you the fuck out with precision the likes of which has never been seen before on this Earth, mark my fucking words. You think you can get away with saying that shit to me over the Internet? Think again, fucker. As we speak I am contacting my secret network of spies across the USA and your IP is being traced right now so you better prepare for the storm, maggot. The storm that wipes out the pathetic little thing you call your life. You’re fucking dead, kid. I can be anywhere, anytime, and I can kill you in over seven hundred ways, and that’s just with my bare hands. Not only am I extensively trained in unarmed combat, but I have access to the entire arsenal of the United States Marine Corps and I will use it to its full extent to wipe your miserable ass off the face of the continent, you little shit. If only you could have known what unholy retribution your little “clever” comment was about to bring down upon you, maybe you would have held your fucking tongue. But you couldn’t, you didn’t, and now you’re paying the price, you goddamn idiot. I will shit fury all over you and you will drown in it. You’re fucking dead, kiddo."};
//
//	for(i=0; i<40; i++){
//		ILI9341_printString_bg(0, i*8, ILI9341_Color565(0,0,0), ILI9341_Color565(255,255,255), &navyPasta[40*i]);
//		ILI9341_drawFastHLine(0, i*8+7, 240, ILI9341_Color565(255,255,255));
//	}
//
//
//
//	ILI9341_fillRect(100, 100, 50, 50, ILI9341_Color565(255,0,0));
//	ILI9341_fillRect(100, 150, 50, 50, ILI9341_Color565(0,255,0));
//	ILI9341_fillRect(100, 200, 50, 50, ILI9341_Color565(0,0,255));
//
//	drawPokey(160, 120);



//	lcd_5110_setAddressX(0);
//	lcd_5110_setAddressY(0);

	//symbolism
//	lcd_5110_arc(41,10,10,200,160,1);
//	lcd_5110_arc(26,40,7,0,360,1);
//	lcd_5110_arc(56,40,7,0,360,1);
//	lcd_5110_line_polar(41, 10, 10, 3, 0, 1);
//	//lcd_5110_line(30,0,30,23,1); //fix this line thing
//	lcd_5110_line_polar(35, 18, 0, 20, 180, 1);
//	lcd_5110_line_polar(47, 18, 0, 20, 180, 1);
//
//	lcd_5110_redraw();
		l11uxx_uart_Send("LCD activity done\n\r");

		GPIOSetValue(1, 13, 1);
		GPIOSetValue(1, 14, 0);





		i=0;
	    while(1){
	    	if(GPIOGetValue(0, 2))GPIOSetValue(1, 14, 0);
	    	else if(GPIOGetValue(0, 7))GPIOSetValue(1, 14, 0);
	    	else GPIOSetValue(1, 14, 1);
	    	i++;
	    	if(i>1){
	    		GPIOSetValue(1, 13, 1);
	    		i=0;
	    		uptime++;
	    	} else GPIOSetValue(1, 13, 0);
	    	delay(250);
	    	//GPIOSetValue(1, 27, 0);
	    	delay(250);
	    	//GPIOSetValue(1, 27, 1);

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







		l11uxx_uart_Send("NRF setup\n\r");
	nrf24l01_pin_init();
	delay(100);
	nrf24l01_pwrupDisable(); //lol otherwise can't write registers?
	delay(100);
	//http://gizmosnack.blogspot.com.ee/2013/04/tutorial-nrf24l01-and-avr.html
	delay(100);
	nrf24l01_writeRegister_withFrame (0x01, 0x01); //en_aa
	nrf24l01_writeRegister_withFrame (0x02, 0x01); //en_rxaddr
	nrf24l01_writeRegister_withFrame (0x03, 0x03); //setup_aw
	nrf24l01_writeRegister_withFrame (0x05, 1); //rf_ch
	nrf24l01_writeRegister_withFrame (0x06, 0x07); //rf_setup
	nrf24l01_setRxAddr(0x1212121212);
	nrf24l01_setTxAddr(0x1212121212);
	nrf24l01_writeRegister_withFrame (0x11, 5); //rx_pw_p0



	l11uxx_uart_Send("NRF setup done\n\r");

	if(!(GPIOGetValue(0, 2))) txRoutine();
	if(!(GPIOGetValue(0, 7)))  rxRoutine();

	l11uxx_uart_Send("You missed your chance for NRF activity. Good job...\n\r");










	//l11uxx_adc_init(char adcNumber, char freerunning, char clkdiv, char bits)
	l11uxx_adc_pinSetup(32);
	l11uxx_adc_init(0, 1,  10, 10);


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
