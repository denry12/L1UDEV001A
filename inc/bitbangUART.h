/*
 * bitbangUART.h
 *
 *  Created on: 20 Apr 2017
 *      Author: denry
 */

#ifndef BITBANGUART_H_
#define BITBANGUART_H_

void bitbangUARTbyte(int letter);
void bitbangUARTmessage(char *text);
void bitbangUARTloadingbar(int current, int max);
void bitbangUARTint(int intValue, int newline, int minSpaces);
void bitbangUARThex(int intValue, int newline, int minSpaces);
void bitbangUARTbin(int intValue, int newline, int minSpaces);


#endif /* BITBANGUART_H_ */
