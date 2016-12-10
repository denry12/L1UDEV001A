/*
 * l11uxx_adc_library.c
 *
 *  Created on: 20.10.2016
 *      Author: Denry
 */


#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif


void l11uxx_adc_pinSetup_unset(int pin){ //uses physical pin numbers
	switch (pin){
		case 32: LPC_IOCON->TDI_PIO0_11 &= (~0x07); LPC_IOCON->TDI_PIO0_11 |= (1<<7); break; //p0_11 //tdi
		case 33: LPC_IOCON->TMS_PIO0_12 &= (~0x07); LPC_IOCON->TMS_PIO0_12 |= (1<<7); break; //p0_12 //tms
		case 34: LPC_IOCON->TDO_PIO0_13 &= (~0x07); LPC_IOCON->TDO_PIO0_13 |= (1<<7); break; //p0_13 //tdo
		case 35: LPC_IOCON->TRST_PIO0_14 &= (~0x07); LPC_IOCON->TRST_PIO0_14 |= (1<<7); break; //p0_14 //trst
		case 39: LPC_IOCON->SWDIO_PIO0_15 &= (~0x07); LPC_IOCON->SWDIO_PIO0_15 |= (1<<7); break; //p0_15 //swdio
		case 40: LPC_IOCON->PIO0_16 &= (~0x07); LPC_IOCON->PIO0_16 |= (1<<7); break; //p0_16
		case 30: LPC_IOCON->PIO0_22 &= (~0x07); LPC_IOCON->PIO0_22 |= (1<<7); break; //p0_22
		case 42: LPC_IOCON->PIO0_23 &= (~0x07); LPC_IOCON->PIO0_23 |= (1<<7); break; //p0_23
		}



	return;
}

void l11uxx_adc_pinSetup(int pin){ //uses physical pin numbers
	l11uxx_adc_pinSetup_unset(pin);

	switch (pin) {
	case 32: LPC_IOCON->TDI_PIO0_11 |= 0x2; LPC_IOCON->TDI_PIO0_11 &= (~(1<<7)); break; //p0_11 //tdi
	case 33: LPC_IOCON->TMS_PIO0_12 |= 0x2; LPC_IOCON->TMS_PIO0_12 &= (~(1<<7)); break; //p0_12 //tms
	case 34: LPC_IOCON->TDO_PIO0_13 |= 0x2; LPC_IOCON->TDO_PIO0_13 &= (~(1<<7)); break; //p0_13 //tdo
	case 35: LPC_IOCON->TRST_PIO0_14 |= 0x2; LPC_IOCON->TRST_PIO0_14 &= (~(1<<7)); break; //p0_14 //trst
	case 39: LPC_IOCON->SWDIO_PIO0_15 |= 0x2; LPC_IOCON->SWDIO_PIO0_15 &= (~(1<<7)); break; //p0_15 //swdio
	case 40: LPC_IOCON->PIO0_16 |= 0x1; LPC_IOCON->PIO0_16 &= (~(1<<7)); break; //p0_16
	case 30: LPC_IOCON->PIO0_22 |= 0x1; LPC_IOCON->PIO0_22 &= (~(1<<7)); break; //p0_22
	case 42: LPC_IOCON->PIO0_23 |= 0x1; LPC_IOCON->PIO0_23 &= (~(1<<7)); break; //p0_23

	}

	return;
}

void l11uxx_adc_init(char adcNumber, char freerunning, char clkdiv, char bits){
//LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<13); //enable clock to ADC
//LPC_SYSCON->PDRUNCFG &= (~(1<<4)); //power up ADC
	//int clkdiv=10; //dividing with CLKDIV+1 must give 4.5MHz or less
	LPC_ADC->CR |= (1<<adcNumber);
	LPC_ADC->CR &= (~(0xFF<<8));
	LPC_ADC->CR |= (clkdiv<<8);

	LPC_ADC->CR &= (~(0x7<<17));
	bits = 10-bits;
	LPC_ADC->CR |= (bits<<17);

	if(freerunning){
		LPC_ADC->CR &= (~(0x07<<24)); //clear start bits
		LPC_ADC->INTEN &= (~(0x01<<8)); //disable global done interrupt
		LPC_ADC->CR |= (1<<16); //enable burst
	}
	else {
		LPC_ADC->CR &= (~(1<<16));
		//LPC_ADC->CR |= (1<<24);//start conversion now
	}
	//LPC_ADC->GDR; //I basically don't care about this
	return;

}

void l11uxx_adc_singleConversion(char adcNumber){

	LPC_ADC->CR &= (~(0xFF)); //clear ADC choices
	LPC_ADC->CR |= (1<<adcNumber);
	LPC_ADC->CR |= (1<<24); //start conversion now
	return;
}

int l11uxx_adc_getValue(char adcNumber){
	int value=0;

	//wait until value is available
	while(!((LPC_ADC->STAT)&(1<<adcNumber)));

	//get value
	value=(LPC_ADC->DR[adcNumber])>>6;
	value &= 0x3FF;
	return value;
}
