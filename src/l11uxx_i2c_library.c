/*
 * l11uxx_i2c_library.c
 *
 *  Created on: 27 Apr 2017
 *      Author: denry
 */

#ifdef __USE_CMSIS
#include "LPC11Uxx.h"
#endif

#include <stdint.h>
#include <stdbool.h>

void l11uxx_i2c_pinSetup(int sclPin, int sdaPin){ //uses physical pin numbers

	//if(I2CNumber == 0){
		if (sclPin == 15) LPC_IOCON->PIO0_4 |= 0x1; //p0_4

		if (sdaPin == 16) LPC_IOCON->PIO0_5 |= 0x1; //p0_5
	//}
	//I don't think this MCU even has any other.
	//else if(I2CNumber == 1) while (1); //lol I am too lazy to make it, I will prolly notice at some point
	return;
}
void l11uxx_i2c_pinSetup_unset(int pin){ //uses physical pin numbers

	if (pin == 15) LPC_IOCON->PIO0_4 &= (~0x07); //p0_4
	if (pin == 16) LPC_IOCON->PIO0_5 &= (~0x07); //p0_5

	return;
}

void l11uxx_i2c_sendStart(){
	LPC_I2C->CONCLR = (1<<3); //clear SI bit
	LPC_I2C->CONCLR = (1<<5); //clear STA bit ???
	LPC_I2C->CONSET = (1<<5); //I2C start enable
	while(!(LPC_I2C->CONSET & (1<<3))); //after START is sent, SI bit is set (wait here until it is)

	return;
}

void l11uxx_i2c_sendStop(){
	//LPC_I2C->CONCLR = (1<<5); //clear START bit
	//LPC_I2C->CONSET = (1<<4);

	LPC_I2C->CONCLR = (1<<3); //clear SI bit
	LPC_I2C->CONCLR = (1<<5); //clear STA bit ???
	LPC_I2C->CONSET = (1<<4); //I2C stop enable

	return;
}

void l11uxx_i2c_sendByte(uint8_t *data){
	if(LPC_I2C->CONSET & (1<<5)) LPC_I2C->CONCLR = (1<<5); //clear STA bit
	while(!(LPC_I2C->CONSET & (1<<3))); //after START is sent, SI bit is set (wait here until it is)
	LPC_I2C->DAT = data;
	LPC_I2C->CONCLR = (1<<3); //clear SI bit
	while(! (LPC_I2C->CONSET & (1<<3)) ); //after START is sent, SI bit is set (wait here until it is)
	//LPC_I2C->CONCLR = (1<<3); //clear SI bit
	//and check states etc. Undone yet. See user manual page 310/523
	return;
}

void l11uxx_i2c_sendAddr(uint8_t addr, uint8_t read){
	addr &= 0x7F; //clamp it.
	//LPC_I2C->CONCLR = (1<<5); //clear START bit
	//while(!(LPC_I2C->CONSET & (1<<3))); //after START is sent, SI bit is set (wait here until it is)
	l11uxx_i2c_sendByte((addr << 1) | read); //load address and write bit

	return;
}

void l11uxx_i2c_receiveByte(){
	while(1); //this is undone, as you can see
	return;
}

void l11uxx_i2c_setspeed(uint16_t SCLH, uint16_t SCLL){
	//both at least 4
	if(SCLH < 4) SCLH = 4;
	if(SCLL < 4) SCLL = 4;

	LPC_I2C->SCLH = SCLH;
	LPC_I2C->SCLL = SCLL;
	return;
}

void l11uxx_i2c_init(){ //TODO: add speed
	//int temporaryValue=0;

	//LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5); //enable clock to I2C //done on device setup?
	//LPC_SYSCON->PRESETCTRL |= (1 << 1); //remove reset from I2C //done on device setup?

	l11uxx_i2c_setspeed(2000, 2000);
	LPC_I2C->CONCLR = 0xFF; //clear ctrl register


	LPC_I2C->CONSET = (1<<6); //I2C interface enable (0x40)
	//LPC_I2C->CONSET = (1<<5); //I2C start enable
	//LPC_I2C->CONSET = (1<<4); //I2C stop enable
	//LPC_I2C->CONCLR = (1<<3); //clear SI bit (0x04)

	return;
}
