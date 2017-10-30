/*
 * lcd_hx1230_lib.c
 *
 *  Created on: 30. okt 2017
 *      Author: Denry
 */
#include "lcd_hx1230_lib.h"

// best info found in the example given here:
// http://www.instructables.com/id/HX1230-LCD-Simple-Arduino-NANO-Clock/

bool lcd_hx1230_sendCommand(hx1230_instance *instance, uint8_t data){
	bool response = 0;
	uint16_t packet = (data & 0xFF);
	response |= (*instance).enableCS();
	response |= (*instance).sendSPIpacket(packet);
	response |= (*instance).disableCS();
	return response;
}

bool lcd_hx1230_sendData(hx1230_instance *instance, uint8_t data){
	bool response = 0;
	uint16_t packet = (data & 0xFF) | (1<<8);
	response |= (*instance).enableCS();
	response |= (*instance).sendSPIpacket(packet);
	response |= (*instance).disableCS();
	return response;
}

bool lcd_hx1230_init(hx1230_instance *instance){
	bool response = 0;

	response |= (*instance).enableReset();
	delay(10);
	response |= (*instance).disableReset();
	delay(100);


	response |= lcd_hx1230_sendCommand(instance, 0xE2); // reset
	// delay!?
	delay(10);
	response |= lcd_hx1230_sendCommand(instance, 0xA4); // power save off
	response |= lcd_hx1230_sendCommand(instance, 0x2F); // power control set
	response |= lcd_hx1230_sendCommand(instance, 0xB0); // page address
	response |= lcd_hx1230_sendCommand(instance, 0x10); // upper 3 bits of column
	response |= lcd_hx1230_sendCommand(instance, 0x00); // lower 4 bits of column
	response |= lcd_hx1230_sendCommand(instance, 0x40); // set row 0 ???
	response |= lcd_hx1230_sendCommand(instance, 0xAF); // display on
	// delay!?
	delay(10);

	return response;
}

bool lcd_hx1230_fillchecker(hx1230_instance *instance){
	bool response = 0;
	uint16_t cycle;
	response |= lcd_hx1230_sendCommand(instance, 0x10); // upper 3 bits of column
	response |= lcd_hx1230_sendCommand(instance, 0x00); // lower 4 bits of column
	response |= lcd_hx1230_sendCommand(instance, 0x40); // set row 0 ???
	response |= lcd_hx1230_sendCommand(instance, 0xB0); // page address
	for(cycle = 0; cycle < 840; cycle++) lcd_hx1230_sendData(instance, 0b00110101);
	return response;
}
