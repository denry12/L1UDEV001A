/*
 * l11uxx_power_lib.c
 *
 *  Created on: 14 Feb 2017
 *      Author: denry
 */


#include "LPC11Uxx.h"
#include <stdint.h>
#include "core_cm0.h"

//wakeup <- write 1 to SLEEPFLAG @ PCON

//general purpose registers that hold data in deep power-down
// Commented out, cause using power-down, not deep power-down (can only be exited with wakeup pin)
//int l11uxx_power_storeData(){
//
//	return 1; //is OK
//	return 0; //very broken
//}
//
//int l11uxx_power_returnData(){
//	return 1; //is OK
//	return 0; //very broken
//}





//Try to keep wakeuptime below 7.86E+018, otherwise things will go bad probably, idk (somewhere there 64bit longlong will overflow)
int l11uxx_power_setWakeupInterrupt(int wakeuptime){ //set up WDT interrupt
	uint64_t wakeuptime_calcvalue; //longer variable for calculating

	//setup wdosc
	LPC_SYSCON->WDTOSCCTRL |= 0b00011111;//set div 64
	LPC_SYSCON->WDTOSCCTRL |= (0x1<<5);//set freq 0.6 MHz


	LPC_WWDT->MOD = 0; //all disabled and such


	//diff of "1" between TC and WARNINT gives 0.4ms
	wakeuptime_calcvalue = (wakeuptime*10000)/4267;
	LPC_WWDT->TC = ((int)(wakeuptime_calcvalue))+0xFF;



	if(LPC_WWDT->CLKSEL & (1<<31)){ //is locked, not good
		if (LPC_WWDT->CLKSEL & (1<<0)) ;//it's just as I want, continue
		else return 0; //very broken
	}
	else LPC_WWDT->CLKSEL = 1; //set wdosc
	LPC_WWDT->WARNINT = 0xFF;//set value when interrupt will fire
	LPC_WWDT->MOD|=(1<<0); //start watchdog

	LPC_WWDT->FEED = 0xAA; //step 1 of watchdog feeding
	LPC_WWDT->FEED = 0x55; //step 2 of watchdog feeding

	return 1; //is OK
	return 0; //very broken
}

int l11uxx_power_wakeupInterrupt(){ //called by wakeup interrupt
	l11uxx_power_enterActiveMode();
	return 1; //is OK
	return 0; //very broken
}



void WDT_IRQHandler(void){ //watchdog interrupt
	LPC_WWDT->MOD|=(1<<3); //clear warning interrupt flag
	l11uxx_power_wakeupInterrupt();
}

int l11uxx_power_sleepClocks(){

//	Counteract setup48MHzInternalClock(); and go to xxx clockspeed
//	go to IRC, not PLL


	 //set main clock to IRC
	 LPC_SYSCON->MAINCLKSEL  = 0;
	 LPC_SYSCON->MAINCLKUEN  = 0;
	 LPC_SYSCON->MAINCLKUEN  = 1;

//	Counteract "setupClocks()"
//	LPC_SYSCON->PDRUNCFG |= (1<<0); //power down IRCosc output
//	LPC_SYSCON->PDRUNCFG |= (1<<1); //power down IRCosc
//	LPC_SYSCON->PDRUNCFG |= (1<<2); //power down flash
//	LPC_SYSCON->PDRUNCFG |= (1<<3); //power down BOD (brownout detection)
//	LPC_SYSCON->PDRUNCFG |= (1<<4); //power down ADC
//	LPC_SYSCON->PDRUNCFG |= (1<<5); //power down xtal osc
//	LPC_SYSCON->PDRUNCFG |= (1<<6); //power down wdt osc
//	LPC_SYSCON->PDRUNCFG |= (1<<7); //power down syspll
//	LPC_SYSCON->PDRUNCFG |= (1<<8); //power down usbpll
//	LPC_SYSCON->PDRUNCFG |= (1<<10); //power down usb transceiver
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<1)); //disable clock to ROM
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<2)); //disable clock to SRAM0
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<3)); //disable clock to flashreg
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<4)); //disable clock to flasharray
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<5)); //disable clock to i2c
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<6)); //disable clock to gpio
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<7)); //disable clock to ct16b0
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<8)); //disable clock to ct16b1
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<9)); //disable clock to ct32b0
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<10)); //disable clock to ct32b1
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<11)); //disable clock to ssp0
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<12)); //disable clock to usart
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<13)); //disable clock to adc
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<14)); //disable clock to usb
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<15)); //disable clock to wwdt
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<16)); //disable clock to iocon
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<18)); //disable clock to ssp1
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<19)); //disable clock to gpio pin int
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<23)); //disable clock to gpiogroup0 interrupt
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<24)); //disable clock to gpiogroup1 interrupt
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<26)); //disable clock to RAM1
//	LPC_SYSCON->SYSAHBCLKCTRL &= (~(1<<27)); //disable clock to USB SRAM

	//enable resets? not sure if helps with current consumpt


	return 1; //is OK
	return 0; //very broken
}

int l11uxx_power_regularClocks(){

	//disable watchdog

	//set up watchdog if used typically

	setupClocks();
	//setup48MHzInternalClock(); //gotta go fast

	//this is a violent placeholder.
	if( setupClocking(0, 12000000) ) while (1); //failed to set clock. Lock MCU



	return 1; //is OK
	return 0; //very broken

}

int l11uxx_power_enterPowerDown(int wakeuptime){

	LPC_PMU->PCON |= (1<<3); //disable DEEP power-down, so it won't happen by accident
	LPC_PMU->PCON |= (0x2); //ARM WFI will enter power-down mode
	//__WFI;//use arm WFI instruction

	//LPC_SYSCON->PDSLEEPCFG |= (1<<3); //BOD power-down in deep-sleep and power-down
	//LPC_SYSCON->PDSLEEPCFG |= (1<<6); //WDT power-down in deep-sleep and power-down
	//3.9.5.2 -> if lock bit 5 in wwdt mod register (table 337) set and IRC selected for WDT, reset lock bit and select WDT osc as WDT clock

	l11uxx_power_sleepClocks(); //incl. go to IRC

	SCB->SCR |= (1<<2); //write 1 to sleepdeep bit in SCR register

	//enable wakeup interrupts in registers (table 43, 44)
		LPC_SYSCON->STARTERP1 |= (1<<12); //enable WWDT interrupt wake-up
		NVIC_EnableIRQ(WDT_IRQn); //enable wakeup interrupts in NVIC
		LPC_SYSCON->PDSLEEPCFG &= (~(1<<6)); //keep watchdog osc powered in power-down


	l11uxx_power_setWakeupInterrupt(wakeuptime);//set up WDT interrupt





	__WFI;//use arm WFI instruction
	__asm("wfi"); //use ARM WFI instruction more directly cause top one doesn't work

	return 1; //is OK
	return 0; //very broken
}


int l11uxx_power_enterActiveMode(){
	LPC_PMU->PCON = (0x00); //default
	LPC_SYSCON->STARTERP1 &= (~(1<<12)); //disable WWDT interrupt wake-up
	NVIC_DisableIRQ(WDT_IRQn); //disable wakeup interrupts in NVIC
	l11uxx_power_regularClocks();

	return 1; //is OK
	return 0; //very broken
}


