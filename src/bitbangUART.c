/*
 * bitbangUART.c
 *
 *  Created on: 26.01.2017
 *      Author: Denry
 */



void bitbangUARTbyte(int letter){ //blasts out data @ pin 25 (P2_10) at staggering baudrate of 9600 bdps.

	static int firstTime=1;
	const int uartdelayus=91;
	const int bitbangUARTport=0;
	const int bitbangUARTbit=17;

	//88 gave 10000 ????
	//92 gave 9433
	//96 gave 9256
	//100 gave 8928
	//3300 worked for 300baud

	if(firstTime){
		GPIOSetDir(bitbangUARTport, bitbangUARTbit, 1); //set output
		GPIOSetValue(bitbangUARTport, bitbangUARTbit, 1); //set high, IDLE
		delay(20); //allow freak firsttimepacket to timeout
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
