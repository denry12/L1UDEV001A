/*
 * l11uxx_internal_flash_lib.c
 *
 *  Created on: 08.12.2016
 *      Author: Denry
 */

//http://www.nxp.com/documents/user_manual/UM10462.pdf
//see starting from page 408
//find flash sectors on page 395

#define IAP_STATUS_CMD_SUCCESS 0
#define IAP_STATUS_SRC_ADDR_ERROR 2
#define IAP_STATUS_DST_ADDR_ERROR 3
//#define IAP_STATUS_ADDR_ERROR
#define IAP_STATUS_COUNT_ERROR 6
#define IAP_STATUS_INVALID_SECTOR 7
#define IAP_STATUS_SECTOR_NOT_PREP_FOR_WRITE 9
#define IAP_STATUS_COMPARE_ERROR 10
#define IAP_STATUS_BUSY 11
#define IAP_LOCATION 0x1fff1ff1
unsigned int command_param[5];
unsigned int status_result[4];

typedef void (*IAP)(unsigned int [],unsigned int[]);
//IAP iap_entry;
//iap_entry=(IAP) IAP_LOCATION;
IAP iap_entry=(IAP) IAP_LOCATION;

//iap_entry (command_param,status_result); //use to call event

//EXAMPLE HOW TO RESERVE RAM
//reserve CAN ROM-RAM 0x1000 0050 - 0x1000 00B9
//__BSS(RESERVED) char CAN_driver_memory[0xC0];
//is reserving 0xC0 bytes as shown in MAP file:
//^  obv no work

const char *firmwareVersion = (char*)0x1000; //lel see peaks olema 4kB mitte 8b (when shared with checksum and other datas!) (nt 0x0000 9000 to 0x0000 9FFF (works from LPC11U34 to 37))
//unsigned char firmwareChecksum[4]@0x2000;


int l11uxx_internal_flash_read(int srcFlashAddr){
	//this function is by idiot denry for idiot denry
	int data=0;
	data = *(&srcFlashAddr);
	return data;
}

int l11uxx_internal_flash_prepForWrite(int startSectorNumber, int endSectorNumber){ //To prepare a single sector use the same "Start" and "End" sector numbers.
	command_param[0] = 50;
	command_param[1] = startSectorNumber;
	command_param[2] = endSectorNumber;
	iap_entry (command_param,status_result);
	while(status_result[0] == IAP_STATUS_BUSY);
	if(status_result[0] == IAP_STATUS_CMD_SUCCESS) return 0; //everything wented better than expedition
	return 1; //o no
}

int l11uxx_internal_flash_RAMToFlash(int destFlashAddr, int srcRamAddr, int numOfBytes, int sysClkInKhz){
	//numofbytes should be 256 | 512 | 1024 | 4096
	if(!((numOfBytes == 256) || (numOfBytes == 512) || (numOfBytes == 1024) || (numOfBytes == 4096))) return 1; //why bother, it will fail

	//destination addr "should be 256 byte boundary"

	//source addr "should be a word boundary"


	command_param[0] = 51;
	command_param[1] = destFlashAddr;
	command_param[2] = srcRamAddr;
	command_param[3] = numOfBytes;
	command_param[4] = sysClkInKhz;
	iap_entry (command_param,status_result);

	while(status_result[0] == IAP_STATUS_BUSY);
	if(status_result[0] == IAP_STATUS_CMD_SUCCESS) return 0; //everything wented better than expedition
	return 1; //o no

}

int l11uxx_internal_flash_erase(int startSectorNumber, int endSectorNumber, int sysClkInKhz){ //To erase a single sector use the same "Start" and "End" sector numbers.
	command_param[0] = 52;
	command_param[1] = startSectorNumber;
	command_param[2] = endSectorNumber;
	command_param[2] = sysClkInKhz;
	iap_entry (command_param,status_result);
	while(status_result[0] == IAP_STATUS_BUSY);
	if(status_result[0] == IAP_STATUS_CMD_SUCCESS) return 0; //everything wented better than expedition
	return 1; //o no
}


int l11uxx_internal_flash_compare(int destAddr, int srcAddr, int numOfBytes){
	//addresses should be word boundaries
	//number of bytes should be a multiple of 4
	command_param[0] = 56;
	command_param[1] = destAddr;
	command_param[2] = srcAddr;
	command_param[2] = numOfBytes;
	iap_entry (command_param,status_result);
	while(status_result[0] == IAP_STATUS_BUSY);
	if(status_result[0] == IAP_STATUS_CMD_SUCCESS) return 0; //everything wented better than expedition
	return 1; //o no
}

/*
 *
Prepare sector(s) for write operation 	50 (decimal) Table 383
Copy RAM to flash 						51 (decimal) Table 384
Erase sector(s) 52 (decimal) Table 385
Blank check sector(s) 53 (decimal) Table 386
Read Part ID 54 (decimal) Table 387
Read Boot code version 55 (decimal) Table 388
Compare 56 (decimal) Table 389
Reinvoke ISP 57 (decimal) Table 390
Read UID 58 (decimal) Table 391
Erase page 59 (decimal) Table 392
EEPROM Write 61(decimal) Table 393
EEPROM Read 62(decimal) Table 394
 */

//enable writing to flash

//set flash address


//write byte?
