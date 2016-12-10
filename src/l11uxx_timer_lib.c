/*
 * l11uxx_timer_lib.c
 *
 *  Created on: 19.10.2016
 *      Author: Denry
 */

#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif


void l11uxx_timer_pinSetup_unset(int pin){ //uses physical pin numbers

	switch (pin){
		case 4: LPC_IOCON->PIO0_1 &= (~0x07); break; //p0_1
		case 27: LPC_IOCON->PIO0_8 &= (~0x07); break; //p0_8
		case 28: LPC_IOCON->PIO0_9 &= (~0x07); break; //p0_9
		case 29: LPC_IOCON->SWCLK_PIO0_10 &= (~0x07); break; //p0_10 //swclk
		case 32: LPC_IOCON->TDI_PIO0_11 &= (~0x07); break; //p0_11 //tdi
		case 34: LPC_IOCON->TDO_PIO0_13 &= (~0x07); break; //p0_13 //tdo
		case 35: LPC_IOCON->TRST_PIO0_14 &= (~0x07); break; //p0_14 //trst
		case 39: LPC_IOCON->SWDIO_PIO0_15 &= (~0x07); break; //p0_15 //swdio
		case 40: LPC_IOCON->PIO0_16 &= (~0x07); break; //p0_16
		case 46: LPC_IOCON->PIO0_18 &= (~0x07); break; //p0_18
		case 47: LPC_IOCON->PIO0_19 &= (~0x07); break; //p0_19
		case 17: LPC_IOCON->PIO0_21 &= (~0x07); break; //p0_21
		case 30: LPC_IOCON->PIO0_22 &= (~0x07); break; //p0_22

		case 36: LPC_IOCON->PIO1_13 &= (~0x07); break; //p1_13
		case 37: LPC_IOCON->PIO1_14 &= (~0x07); break; //p1_14
		case 43: LPC_IOCON->PIO1_15 &= (~0x07); break; //p1_15
		case 18: LPC_IOCON->PIO1_23 &= (~0x07); break; //p1_23
		case 21: LPC_IOCON->PIO1_24 &= (~0x07); break; //p1_24
		case 11: LPC_IOCON->PIO1_26 &= (~0x07); break; //p1_26
		case 12: LPC_IOCON->PIO1_27 &= (~0x07); break; //p1_27
	}
	return;
}


void l11uxx_timer_pinSetup(int pin){ //uses physical pin numbers

	l11uxx_timer_pinSetup_unset(pin);

	switch (pin) {
		case 4: LPC_IOCON->PIO0_1 |= 0x1; break; //p0_1
		case 27: LPC_IOCON->PIO0_8 |= 0x2; break; //p0_8
		case 28: LPC_IOCON->PIO0_9 |= 0x2; break; //p0_9
		case 29: LPC_IOCON->SWCLK_PIO0_10 |= 0x3; break; //p0_10 //swclk
		case 32: LPC_IOCON->TDI_PIO0_11 |= 0x3; break; //p0_11 //tdi
		case 34: LPC_IOCON->TDO_PIO0_13 |= 0x3; break; //p0_13 //tdo
		case 35: LPC_IOCON->TRST_PIO0_14 |= 0x3; break; //p0_14 //trst
		case 39: LPC_IOCON->SWDIO_PIO0_15 |= 0x3; break; //p0_15 //swdio
		case 40: LPC_IOCON->PIO0_16 |= 0x2; break; //p0_16
		case 46: LPC_IOCON->PIO0_18 |= 0x2; break; //p0_18
		case 47: LPC_IOCON->PIO0_19 |= 0x2; break; //p0_19
		case 17: LPC_IOCON->PIO0_21 |= 0x1; break; //p0_21
		case 30: LPC_IOCON->PIO0_22 |= 0x2; break; //p0_22

		case 36: LPC_IOCON->PIO1_13 |= 0x2; break; //p1_13
		case 37: LPC_IOCON->PIO1_14 |= 0x2; break; //p1_14
		case 43: LPC_IOCON->PIO1_15 |= 0x2; break; //p1_15
		case 18: LPC_IOCON->PIO1_23 |= 0x1; break; //p1_23
		case 21: LPC_IOCON->PIO1_24 |= 0x1; break; //p1_24
		case 11: LPC_IOCON->PIO1_26 |= 0x1; break; //p1_26
		case 12: LPC_IOCON->PIO1_27 |= 0x1; break; //p1_27
	}
	return;
}




void l11uxx_timer_initPWM(int timerType, int timerNumber, int prescaler, int matchNumber){
	//timerType is 16 or 32, timernumber is following B (eg 16B1)

	//todo: put these in right places in this function
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<8); //enable clock to CT16B1
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<10); //enable clock to CT32B1


	if (timerType==16){
		switch (timerNumber){
		case 0:
			LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<7); //enable clock to CT16B0
			LPC_CT16B0->TC = 0;
			LPC_CT16B0->PC = 0;
			LPC_CT16B0->PR = prescaler; //16,000,000 / 16 = 1us. Incremented every PCLK.
			//LPC_CT16B0->MCR = (0x01<<10); //no interrupt, reset on MR3 -----//stop on MR0 match. Interrupt, no reset
			LPC_CT16B0->MCR = 0; //no interrupt, no reset, no stop
			//LPC_CT16B0->MR3 = maxValue;
			//while ( !(LPC_CT16B0->MR0 == us) ); // Wait until updated
			LPC_CT16B0->TCR = 0x1; //enable
			LPC_CT16B0->PWMC |= (0x01 << matchNumber);
			//while(LPC_CT16B0->TCR & 0x1); //wait while timer is running.
			break;
		}
	} else {
		switch (timerNumber){
				case 0:
					LPC_SYSCON->SYSAHBCLKCTRL |= (0x01<<9); //enable clock to CT32B0
					LPC_CT32B0->TC = 0;
					LPC_CT32B0->PC = 0;
					LPC_CT32B0->PR = prescaler; //16,000,000 / 16 = 1us. Incremented every PCLK.
					LPC_CT32B0->MCR = 0; //no interrupt, no reset, no stop
					//LPC_CT32B0->MR3 = maxValue;
					//while ( !(LPC_CT16B0->MR0 == us) ); // Wait until updated
					LPC_CT32B0->TCR = 0x1; //enable
					LPC_CT32B0->PWMC |= (0x01 << matchNumber);
					//while(LPC_CT16B0->TCR & 0x1); //wait while timer is running.
					break;
				}
	}
	return;
}

void l11uxx_timer_setPWM(int timerType, int timerNumber, int matchNumber, unsigned int matchValue){
	//matchNumber is MRx x, matchValue is MRx value
	if (timerType==16){
		switch (timerNumber){
				case 0:
					switch (matchNumber){
						case 0: LPC_CT16B0->MR0 = matchValue;	break;
						case 1: LPC_CT16B0->MR1 = matchValue;	break;
						case 2: LPC_CT16B0->MR2 = matchValue;	break;
						case 3: LPC_CT16B0->MR3 = matchValue;	break;
					//while ( !(LPC_CT16B0->MR0 == us) ); // Wait until updated
					break;
				}
		}
	} else {
		switch (timerNumber){
			case 0:
				switch (matchNumber){
					case 0: LPC_CT32B0->MR0 = matchValue;	break;
					case 1: LPC_CT32B0->MR1 = matchValue;	break;
					case 2: LPC_CT32B0->MR2 = matchValue;	break;
					case 3: LPC_CT32B0->MR3 = matchValue;	break;
					//while ( !(LPC_CT16B0->MR0 == us) ); // Wait until updated
				break;
				}
		}
	}
	return;
}
