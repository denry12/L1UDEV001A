/*
 * l11uxx_power_lib.c
 *
 *  Created on: 14 Feb 2017
 *      Author: denry
 */


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

int l11uxx_power_setWakeupInterrupt(int wakeuptime){ //set up WDT interrupt
	//wake
}

int l11uxx_power_wakeupInterrupt(){ //called by wakeup interrupt

}



void WDT_IRQHandler(void){ //watchdog interrupt
	l11uxx_power_wakeupInterrupt();
}

int l11uxx_power_sleepClocks(){

//	Counteract setup48MHzInternalClock(); and go to xxx clockspeed
//	go to IRC, not PLL



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
	setup48MHzInternalClock(); //gotta go fast



	return 1; //is OK
	return 0; //very broken

}

int l11uxx_power_enterPowerDown(int wakeuptime){

	LPC_SYSCON->PCON |= (1<<3); //disable DEEP power-down, so it won't happen by accident
	LPC_SYSCON->PCON |= (0x2); //ARM WFI will enter power-down mode
	//LPC_SYSCON->PDSLEEPCFG |= (1<<3); //BOD power-down in deep-sleep and power-down
	//LPC_SYSCON->PDSLEEPCFG |= (1<<6); //WDT power-down in deep-sleep and power-down
	//3.9.5.2 -> if lock bit 5 in wwdt mod register (table 337) set and IRC selected for WDT, reset lock bit and select WDT osc as WDT clock

	l11uxx_power_sleepClocks(); //incl. go to IRC

	l11uxx_power_setWakeupInterrupt(wakeuptime);//set up WDT interrupt

	//enable wakeup interrupts in registers (table 43, 44)
	LPC_SYSCON->STARTERP1 |= (1<<12); //enable WWDT interrupt wake-up
	NVIC_EnableIRQ(WDT_IRQn); //enable wakeup interrupts in NVIC

	SCB->SCR |= (1<<2); //write 1 to sleepdeep bit in SCR register

	__WFI;//use arm WFI instruction

	return 1; //is OK
	return 0; //very broken
}


int l11uxx_power_enterActiveMode(){
	l11uxx_power_regularClocks();

	return 1; //is OK
	return 0; //very broken
}


