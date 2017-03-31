/*
 * bitbangUART.c
 *
 *  Created on: 26.01.2017
 *      Author: Denry
 */

#include <stdbool.h>

void bitbangUARTbyte(int letter){ //blasts out data @ pin 25 (P2_10) at staggering baudrate of 9600 bdps.

	static int firstTime=1;
	//const int uartdelayus=91;// (9600@12MHz?)
	const int uartdelayus=100;// (9600@48MHz?)
	//const int bitbangUARTport=0;
	//const int bitbangUARTbit=17;

	const int bitbangUARTport=1;
	const int bitbangUARTbit=19; //SSEL

	//12mhz theory prolly:
	//88 gave 10000 ????
	//92 gave 9433
	//96 gave 9256
	//100 gave 8928
	//3300 worked for 300baud

	if(firstTime){
		GPIOSetDir(bitbangUARTport, bitbangUARTbit, 1); //set output
		GPIOSetValue(bitbangUARTport, bitbangUARTbit, 1); //set high, IDLE
		delay(10); //allow freak firsttimepacket to timeout
		firstTime=0;
	}

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, 0); // start bit
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, letter&1);
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, (letter&2)>>1);
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, (letter&4)>>2);
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, (letter&8)>>3);
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, (letter&16)>>4);
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, (letter&32)>>5);
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, (letter&64)>>6);
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, (letter&128)>>7);
	delay_us(uartdelayus);

	GPIOSetValue(bitbangUARTport, bitbangUARTbit, 1); // stop bit
	delay_us(uartdelayus);


	return;
}

void bitbangUARTmessage(char *text){
		int charNumber=0;
		while((text[charNumber] != 0) && (charNumber<200) ){ //check until 0 or sane amount of characters
			bitbangUARTbyte(text[charNumber]);
			charNumber++;
		}
		//bitbangUARTbyte(11); //\n
		//bitbangUARTbyte(13); //CR
	return;
}

void bitbangUARTloadingbar(int current, int max){
	const bool lockedSize=1;
	const char loadingBarLockedSize=20;

	if(lockedSize){
		current = current * loadingBarLockedSize;
		current = current / max;
		max = loadingBarLockedSize;
	}

	int i=0;
	bitbangUARTbyte(13); //CR
	bitbangUARTmessage("[");
		while(i<current){
			bitbangUARTmessage("=");
			i++;
		}
		while(i<max){
					bitbangUARTmessage(" ");
					i++;
		}
		bitbangUARTmessage("]");
		//bitbangUARTbyte(11); //\n
		//bitbangUARTbyte(13); //CR
	return;
}

void bitbangUARTint(int intValue, int newline, int minSpaces){
	char intString[12];
	int paddingZeroes=0;
	itoa(intValue, intString, 10);
	paddingZeroes = minSpaces - strlen(intValue) + 1;
	while(paddingZeroes>0){
		bitbangUARTmessage("0");
		paddingZeroes--;
	}
	bitbangUARTmessage(intString);
	if(newline & 1) bitbangUARTmessage("\r");
	if(newline & 2) bitbangUARTmessage("\n");
	return;
}

void bitbangUARThex(int intValue, int newline, int minSpaces){
	char intString[12];
	int paddingZeroes=0;
	itoa(intValue, intString, 16);
	bitbangUARTmessage("0x");
	paddingZeroes = minSpaces - strlen(intValue) + 2;
	while(paddingZeroes>0){
		bitbangUARTmessage("0");
		paddingZeroes--;
	}
	bitbangUARTmessage(intString);
	if(newline & 1) bitbangUARTmessage("\r");
	if(newline & 2) bitbangUARTmessage("\n");
	return;
}

void bitbangUARTbin(int intValue, int newline, int minSpaces){
	char intString[12];
	int paddingZeroes=0;
	itoa(intValue, intString, 2);
	bitbangUARTmessage("0b");
	paddingZeroes = minSpaces - strlen(intValue);
	while(paddingZeroes>0){
		bitbangUARTmessage("0");
		paddingZeroes--;
	}
	bitbangUARTmessage(intString);
	if(newline & 1) bitbangUARTmessage("\r");
	if(newline & 2) bitbangUARTmessage("\n");
	return;
}
