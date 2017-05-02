/*
 * nrf24l01_lib.c
 *
 *  Created on: 29.09.2016
 *      Author: Denry
 */

//------------------------
//HW specific starts here
//------------------------



void nrf24l01_CSEnable(){
	delay(10);
	GPIOSetValue(1, 25, 0);
	delay(10);
	return;
}

void nrf24l01_CSDisable(){
	delay(10);
	GPIOSetValue(1, 25, 1);
	delay(10);
	return;
}

void nrf24l01_CEEnable(){ //keeping enabled makes RF transmit/receive happen
	GPIOSetValue(1, 19, 1);
	return;
}

void nrf24l01_CEDisable(){ //disabling CE, send RF to sleep. Standby-I or power-down mode
	GPIOSetValue(1, 19, 0);
	return;
}

void nrf24l01_pin_init(){ // separate to "init HW" and "init (SW)"?
	//setup GPIO
	//setup SPI
	GPIOSetDir(1, 25, 1); //CS
	GPIOSetDir(1, 19, 1); //CE
	nrf24l01_CSDisable();
	nrf24l01_CEDisable();
	return;
}

//void nrf24l01_disableMISO(){ //to force that thing low
//	l11uxx_spi_pinSetup_unset(26);
//	GPIOSetDir(1, 21, 1);
//	GPIOSetValue(1, 21, 0); //force low
//	return;
//}
//
//void nrf24l01_enableMISO(){ //to unforce it
//	GPIOSetDir(1, 21, 0);
//	l11uxx_spi_pinSetup(1, 38, 26, 13);
//	return;
//}

void nrf24l01_sendSPIByte(int data){
	//blast data out from SPI
	l11uxx_spi_sendByte(1, data);
	return;
}

int nrf24l01_flushSPIRx(){
	l11uxx_spi_flushRxBuffer(1);
	return 0;
}

int nrf24l01_recvSPIByte(){
	int data = 0;
	nrf24l01_flushSPIRx();
	data = l11uxx_spi_receiveByte(1);
	return data;
}

void nrf24l01_debugOutput(char text[]){
	l11uxx_uart_Send(text);
	//printf(text);
	return;
}

//------------------------
//HW specific ends here
//------------------------

//SPI command layer

void nrf24l01_writeRegister (int registerAddress){
	registerAddress &= 0x1F;
	registerAddress |= 0x20;
	nrf24l01_sendSPIByte(registerAddress);
	return;
}

void nrf24l01_readRegister (int registerAddress){
	registerAddress &= 0x1F;
	nrf24l01_sendSPIByte(registerAddress);
	return;
}

int nrf24l01_readRegister_withFrame (int registerAddress){
	//registerAddress &= 0x1F; //done in that other function anyway
	char registerContents=0;
	nrf24l01_CSEnable();
	nrf24l01_readRegister(registerAddress);
	nrf24l01_flushSPIRx();
	registerContents=nrf24l01_recvSPIByte();
	nrf24l01_CSDisable();

	return registerContents;
}

void nrf24l01_writeRegister_withFrame (int registerAddress, int data){
	//registerAddress &= 0x1F; //done in that other function anyway
	nrf24l01_CSEnable();
	nrf24l01_writeRegister(registerAddress);
	nrf24l01_sendSPIByte(data);
	nrf24l01_CSDisable();

	return;
}

void nrf24l01_readRxPayload (){
	nrf24l01_sendSPIByte(0x61);
	return;
}

void nrf24l01_writeTxPayload (){
	nrf24l01_sendSPIByte(0xA0);
	return;
}

void nrf24l01_flushTxBuffer (){
	nrf24l01_CSEnable();
	nrf24l01_sendSPIByte(0xE1);
	nrf24l01_CSDisable();
	return;
}

void nrf24l01_flushRxBuffer (){
	nrf24l01_CSEnable();
	nrf24l01_sendSPIByte(0xE2);
	nrf24l01_CSDisable();
	return;
}

//some additional functions I don't care about

//register manipulation ends here

//first abstraction layer

void nrf24l01_primRXEnable(){
	int confReg=0;
	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x00); //read config register
	confReg=nrf24l01_recvSPIByte();
	nrf24l01_CSDisable();

	confReg |= (1<<0);

	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x00); //write config register
	nrf24l01_sendSPIByte(confReg);
	nrf24l01_CSDisable();
	return;
	return;
}

void nrf24l01_primRXDisable(){
	int confReg=0;
	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x00); //read config register
	confReg=nrf24l01_recvSPIByte();
	nrf24l01_CSDisable();

	confReg &= (~((1<<0)&0xFF));

	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x00); //write config register
	nrf24l01_sendSPIByte(confReg);
	nrf24l01_CSDisable();
	return;
	return;
}

void nrf24l01_pwrupEnable(){
	int confReg=0;
	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x00); //read config register
	confReg=nrf24l01_recvSPIByte();
	nrf24l01_CSDisable();

	confReg |= (1<<1);

	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x00); //write config register
	nrf24l01_sendSPIByte(confReg);
	nrf24l01_CSDisable();
	return;
}

void nrf24l01_pwrupDisable(){
	int confReg=0;
	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x00); //read config register
	confReg=nrf24l01_recvSPIByte();
	nrf24l01_CSDisable();

	confReg &= (~((1<<1)&0xFF));

	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x00); //write config register
	nrf24l01_sendSPIByte(confReg);
	nrf24l01_CSDisable();
	return;
}

void nrf24l01_setAddrWidth(int aw){ //in bytes
	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x04); //write aw register
	if(aw==3) nrf24l01_sendSPIByte(0x01);
	else if(aw==4) nrf24l01_sendSPIByte(0x02);
	else if(aw==5) nrf24l01_sendSPIByte(0x03);
	nrf24l01_CSDisable();

	return;
}

void nrf24l01_setDatarate(int rate){ //either 1 or 2
	int rfReg=0;
	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x06); //read rf register
	rfReg=nrf24l01_recvSPIByte();
	nrf24l01_CSDisable();

	if(rate == 1){
		rfReg &= (~((1<<3)&0xFF));
	}else if(rate == 2){
		rfReg |= (1<<3);
	} else return;

	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x06); //write rf register
	nrf24l01_sendSPIByte(rfReg);
	nrf24l01_CSDisable();
	return;
}

void nrf24l01_setFreq(int freq){ //sets RF_CH (frequency = 2400 + int freq MHz)
	//if (freq > 125) return;
	if ( freq > 125) freq = 125;
	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x05); //write rf_ch register
	nrf24l01_sendSPIByte(freq);
	nrf24l01_CSDisable();
	return;
}

void nrf24l01_setPower(int pwrlvl){ //sets PA level (0 ... 3, where 0 is -18dBm and 3 is 0dBm)
	//if (pwrlvl > 3) return;
	if (pwrlvl > 3) pwrlvl = 3;


	int rfReg=0;
	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x06); //read rf register
	rfReg=nrf24l01_recvSPIByte();
	nrf24l01_CSDisable();


	rfReg &= (~((3<<1)&0xFF)); //clear necessary bits
	pwrlvl = pwrlvl << 1;
	pwrlvl &= 0x06; //little protector
	rfReg |= pwrlvl;

	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x06); //write rf register
	nrf24l01_sendSPIByte(rfReg);
	nrf24l01_CSDisable();


	return;
}

void nrf24l01_lnaGain(int gain){ //tbd what means what
	int rfReg=0;
		nrf24l01_CSEnable();
		nrf24l01_readRegister(0x06); //read rf register
		rfReg=nrf24l01_recvSPIByte();
		nrf24l01_CSDisable();

		if(gain){
			rfReg &= (~((1<<0)&0xFF));
		}else {
			rfReg |= (1<<0);
		}

		nrf24l01_CSEnable();
		nrf24l01_writeRegister(0x06); //write rf register
		nrf24l01_sendSPIByte(rfReg);
		nrf24l01_CSDisable();
	return;
}


int nrf24l01_isTxFIFOFull(){
	int isFull = 0;


	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x07); //read status register
	if((nrf24l01_recvSPIByte())&0x01) isFull=1;
	nrf24l01_CSDisable();

	return isFull;
}

int nrf24l01_isRxFIFOFull(){
	int isFull = 0;


	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x17); //read status register
	if((nrf24l01_recvSPIByte())&0x02) isFull=1;
	nrf24l01_CSDisable();

	return isFull;
}

int nrf24l01_isRxFIFOEmpty(){
	int isEmpty = 0;


	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x17); //read status register
	nrf24l01_flushSPIRx();
	if((nrf24l01_recvSPIByte())&0x01) isEmpty=1;
	nrf24l01_CSDisable();

	return isEmpty;
}

//------------------------ (PFFF REMOVE THIS BS TAG)
//shockburst level starts here
//------------------------
void nrf24l01_setTxAddr(long long tx_addr){ //addr is up to 5 bytes long


	//THIS RUNS WELL

	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x10); //write tx_addr register
	nrf24l01_sendSPIByte((tx_addr)&0xFF); //LSB first
	tx_addr = tx_addr >> 8;
	nrf24l01_sendSPIByte((tx_addr)&0xFF);
	tx_addr = tx_addr >> 8;
	nrf24l01_sendSPIByte((tx_addr)&0xFF);
	tx_addr = tx_addr >> 8;
	nrf24l01_sendSPIByte((tx_addr)&0xFF);
	tx_addr = tx_addr >> 8;
	nrf24l01_sendSPIByte((tx_addr)&0xFF);


	nrf24l01_CSDisable();

	return;
}

void nrf24l01_setRxAddr(long long rx_addr){ //addr is up to 5 bytes long
	//nrf24l01_disableMISO(); //didn't help



	//char LSByte = rx_addr & 0xFF;

	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x0A); //write rx_addr register
	nrf24l01_sendSPIByte((rx_addr)&0xFF); //LSB first
	rx_addr = rx_addr >> 8;
	nrf24l01_sendSPIByte((rx_addr)&0xFF);
	rx_addr = rx_addr >> 8;
	nrf24l01_sendSPIByte((rx_addr)&0xFF);
	rx_addr = rx_addr >> 8;
	nrf24l01_sendSPIByte((rx_addr)&0xFF);
	rx_addr = rx_addr >> 8;
	nrf24l01_sendSPIByte((rx_addr)&0xFF);
	nrf24l01_CSDisable();
	//nrf24l01_enableMISO(); //didn't help

//	//and rewrite LSByte
//	nrf24l01_CSEnable();
//	nrf24l01_writeRegister(0x0A); //write rx_addr register
//	nrf24l01_sendSPIByte(0xFA); //to see where it ends up
//	nrf24l01_CSDisable();

	return;
}

void nrf24l01_writeBytesOfPayload_p0(int data){
	nrf24l01_CSEnable();
	nrf24l01_writeRegister(0x11); //read pipe payload register
	data &= 0x20;
	nrf24l01_sendSPIByte(data);
	nrf24l01_CSDisable();
	return;
}

int nrf24l01_bytesOfPayload_p0(){
	int bytesInPipe=0;
	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x11); //read pipe payload register
	nrf24l01_flushSPIRx();
	bytesInPipe=nrf24l01_recvSPIByte();
	nrf24l01_CSDisable();
	return bytesInPipe;
}



void nrf24l01_getConfigData(){
	char temporaryString1[40];
	char prim_rx=0, pwr_up=0, crco=0, en_crc=0; //config register
	char rf_ch=0, pwrlvl=0, datarate=0, aw=0;
	char temporaryRegister=0;
	char temporaryRegisterValue[10];
	temporaryRegister = nrf24l01_readRegister_withFrame (0x00); //config register
	nrf24l01_debugOutput("NRF24L01 config: \n\r");
	//if(temporaryRegister & (1<<0)) prim_rx = 1;
	//if(temporaryRegister & (1<<1)) pwr_up = 1;
	//if(temporaryRegister & (1<<2)) crco = 1;
	//if(temporaryRegister & (1<<3)) en_crc = 1;
	if(temporaryRegister & (1<<0)) nrf24l01_debugOutput("PRIM_RX = 1; ");
	else nrf24l01_debugOutput("PRIM_RX = 0; ");
	if(temporaryRegister & (1<<1)) nrf24l01_debugOutput("PWR_UP = 1; ");
	else nrf24l01_debugOutput("PWR_UP = 0; ");
	if(temporaryRegister & (1<<2)) nrf24l01_debugOutput("CRCO = 1; ");
	else nrf24l01_debugOutput("CRCO = 0; ");
	if(temporaryRegister & (1<<3)) nrf24l01_debugOutput("EN_CRC = 1; ");
	else nrf24l01_debugOutput("EN_CRC = 0; ");

	if(temporaryRegister & (1<<4)) nrf24l01_debugOutput("MASK_MAT_RT = 1; ");
	else nrf24l01_debugOutput("MASK_MAT_RT = 0; ");
	if(temporaryRegister & (1<<5)) nrf24l01_debugOutput("MASK_TX_DS = 1; ");
	else nrf24l01_debugOutput("MASK_TX_DS = 0; ");
	if(temporaryRegister & (1<<6)) nrf24l01_debugOutput("MASK_RX_DR = 1; ");
	else nrf24l01_debugOutput("MASK_RX_DR = 0; ");

	nrf24l01_debugOutput("\n\r");
	temporaryRegister = nrf24l01_readRegister_withFrame (0x01); //EN_AA
	nrf24l01_debugOutput("ENAA_P0 = ");
	if(temporaryRegister & (1<<0)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ENAA_P1 = ");
	if(temporaryRegister & (1<<1)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ENAA_P2 = ");
	if(temporaryRegister & (1<<2)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ENAA_P3 = ");
	if(temporaryRegister & (1<<3)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ENAA_P4 = ");
	if(temporaryRegister & (1<<4)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ENAA_P5 = ");
	if(temporaryRegister & (1<<5)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("\n\r");
	temporaryRegister = nrf24l01_readRegister_withFrame (0x02); //EN_RXADDR
	nrf24l01_debugOutput("ERX_P0 = ");
	if(temporaryRegister & (1<<0)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ERX_P1 = ");
	if(temporaryRegister & (1<<1)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ERX_P2 = ");
	if(temporaryRegister & (1<<2)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ERX_P3 = ");
	if(temporaryRegister & (1<<3)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ERX_P4 = ");
	if(temporaryRegister & (1<<4)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("ERX_P5 = ");
	if(temporaryRegister & (1<<5)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");



	nrf24l01_debugOutput("\n\r");
	temporaryRegister = nrf24l01_readRegister_withFrame (0x03); //SETUP_AW
	nrf24l01_debugOutput("AW = ");
	if(temporaryRegister & ((1<<0) || (1<<1))) nrf24l01_debugOutput("5 bytes");
	else if(temporaryRegister & ((1<<1))) nrf24l01_debugOutput("4 bytes");
	else if(temporaryRegister & ((1<<0))) nrf24l01_debugOutput("3 bytes");
	nrf24l01_debugOutput("; ");

	//idgaf about setup_retr

	nrf24l01_debugOutput("\n\r");
	temporaryRegister = nrf24l01_readRegister_withFrame (0x05); //RF_CH
	nrf24l01_debugOutput("RF_CH = ");
	if(temporaryRegister < 100) nrf24l01_debugOutput("24");
	else if(temporaryRegister < 200 ){
		nrf24l01_debugOutput("25");
		temporaryRegister -= 100;
	}
	else nrf24l01_debugOutput("FUGG PEKKA :DDDDD");
	uitoa(temporaryRegister, temporaryString1,10);
	nrf24l01_debugOutput(temporaryString1);
	nrf24l01_debugOutput("; ");


	nrf24l01_debugOutput("\n\r");
	temporaryRegister = nrf24l01_readRegister_withFrame (0x06); //RF_SETUP
	nrf24l01_debugOutput("LNA_HCURR = ");
	if(temporaryRegister & (1<<0)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("LNA_HCURR = ");
	if(temporaryRegister & ((1<<0) || (1<<1))) nrf24l01_debugOutput("+0dBm");
	else if(temporaryRegister & ((1<<1))) nrf24l01_debugOutput("-6dBm");
	else if(temporaryRegister & ((1<<0))) nrf24l01_debugOutput("-12dBm");
	else nrf24l01_debugOutput("-18dBm");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("RF_DR = ");
	if(temporaryRegister & (1<<0)) nrf24l01_debugOutput("2Mbps");
	else nrf24l01_debugOutput("1Mbps");
	nrf24l01_debugOutput("; ");






	nrf24l01_debugOutput("\n\r");
	temporaryRegister = nrf24l01_readRegister_withFrame (0x07); //STATUS
	nrf24l01_debugOutput("TX_FULL = ");
	if(temporaryRegister & (1<<0)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("RX_P_NO = ");
	if(temporaryRegister & (1<<3)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	if(temporaryRegister & (1<<2)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	if(temporaryRegister & (1<<1)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");


	nrf24l01_debugOutput("MAX_RT = ");
	if(temporaryRegister & (1<<4)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("TX_DS = ");
	if(temporaryRegister & (1<<5)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");

	nrf24l01_debugOutput("RX_DR = ");
	if(temporaryRegister & (1<<5)) nrf24l01_debugOutput("1");
	else nrf24l01_debugOutput("0");
	nrf24l01_debugOutput("; ");


	nrf24l01_debugOutput("\n\r");
	nrf24l01_debugOutput("RX_ADDR_P0 = ");
	nrf24l01_CSEnable();
	nrf24l01_readRegister(0x0A);
	nrf24l01_debugOutput("0x");
	nrf24l01_flushSPIRx();
	temporaryRegisterValue[0]=nrf24l01_recvSPIByte();
	temporaryRegisterValue[1]=nrf24l01_recvSPIByte();
	temporaryRegisterValue[2]=nrf24l01_recvSPIByte();
	temporaryRegisterValue[3]=nrf24l01_recvSPIByte();
	temporaryRegisterValue[4]=nrf24l01_recvSPIByte();
	nrf24l01_flushSPIRx();
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[4]);
	nrf24l01_debugOutput(temporaryString1);
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[3]);
	nrf24l01_debugOutput(temporaryString1);
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[2]);
	nrf24l01_debugOutput(temporaryString1);
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[1]);
	nrf24l01_debugOutput(temporaryString1);
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[0]);
	nrf24l01_debugOutput(temporaryString1);
	nrf24l01_CSDisable();


	//nrf24l01_debugOutput(" (Likely to be 0xDAABCDEF01)");
	nrf24l01_debugOutput(" (Likely to be  0x456789ABCD)");


	nrf24l01_debugOutput("\n\r");
	nrf24l01_debugOutput("TX_ADDR = ");
	nrf24l01_CSEnable();
	delay(10);
	nrf24l01_readRegister(0x10);
	nrf24l01_debugOutput("0x");
	nrf24l01_flushSPIRx();
	delay(10);
	temporaryRegisterValue[0]=nrf24l01_recvSPIByte();
	delay(10);
	temporaryRegisterValue[1]=nrf24l01_recvSPIByte();
	delay(10);
	temporaryRegisterValue[2]=nrf24l01_recvSPIByte();
	delay(10);
	temporaryRegisterValue[3]=nrf24l01_recvSPIByte();
	delay(10);
	temporaryRegisterValue[4]=nrf24l01_recvSPIByte();
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[4]);
	nrf24l01_debugOutput(temporaryString1);
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[3]);
	nrf24l01_debugOutput(temporaryString1);
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[2]);
	nrf24l01_debugOutput(temporaryString1);
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[1]);
	nrf24l01_debugOutput(temporaryString1);
	sprintf(&temporaryString1[0], "%02x", temporaryRegisterValue[0]);
	nrf24l01_debugOutput(temporaryString1);
	nrf24l01_CSDisable();

	nrf24l01_debugOutput(" (Likely to be 0xFEDCBA9876)");


	nrf24l01_debugOutput("\n\r");

	return;
}

//void nrf24l01_disableAck(){
//
//	return;
//}



//void nrf24l01_enableAck(){
//
//	return;
//}


//void nrf24l01_crcLength(int length){
//
//	return;
//}

//void nrf24l01_enableAutoReTransmission(int delay, int retries){
//
//	return;
//}



void rxRoutine(){
	l11uxx_uart_Send("RxRoutine is go!\n\r");
	int i=0;
	int j = 0;
	int visualAssistance = 0;
	int statusReg=0;
	char temporaryString1[40], temporaryString2[40];
	int temporaryInt=0;

	/*delay(100);
			nrf24l01_pwrupEnable();
			delay(100);
			*/


			nrf24l01_writeRegister_withFrame (0x00, 0x1F); //config, rx
			delay(100);

	nrf24l01_getConfigData();
	while(1){

			//i = nrf24l01_bytesOfPayload_p0();
			GPIOSetValue(1, 14, 1);
			statusReg=nrf24l01_readRegister_withFrame(0x07); //read status register
			//while (!(statusReg & 0x40)){
			while ((nrf24l01_isRxFIFOEmpty())){
				nrf24l01_CEEnable(); //LED shines while listening
				GPIOSetValue(1, 13, 0);
				delay(1000);
				GPIOSetValue(1, 13, 1);
				nrf24l01_CEDisable();
				delay(10);


				statusReg=nrf24l01_readRegister_withFrame(0x07); //read status register
				//if(statusReg & 0x40){ //new data received
				//	nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x40)); //clear data received interrupt
				//	l11uxx_uart_Send("Data received interrupt!\n\r");
				//}
				if(nrf24l01_isRxFIFOEmpty()) l11uxx_uart_Send("Rx FIFO empty - 1 ");
				//else l11uxx_uart_Send("Rx FIFO has data - 1 \a"); //with bell! 0x07
				else l11uxx_uart_Send("Rx FIFO has data - 1");
				if (visualAssistance) l11uxx_uart_Send("|");
				else l11uxx_uart_Send("-");
				l11uxx_uart_Send("\n\r");
				if(visualAssistance != 0) visualAssistance = 0;
				else visualAssistance = 1;


				//i = nrf24l01_bytesOfPayload_p0();


			}
			/*if(nrf24l01_isRxFIFOEmpty()) l11uxx_uart_Send("Rx FIFO empty - 2 ");
			else l11uxx_uart_Send("Rx FIFO has data - 2 ");
			if (visualAssistance) l11uxx_uart_Send("|");
			else l11uxx_uart_Send("-");
			l11uxx_uart_Send("\n\r");
			if(visualAssistance != 0) visualAssistance = 0;
			else visualAssistance = 1;
			GPIOSetValue(1, 14, 1);

			 */
			//
			strcpy(temporaryString1, "                              ");
			l11uxx_uart_Send("Data: ");

			//temporaryInt=nrf24l01_isRxFIFOEmpty();


			//while(!(temporaryInt)){
			while(!(nrf24l01_isRxFIFOEmpty())){
				nrf24l01_CSEnable();
				nrf24l01_readRxPayload ();
				//while(!(nrf24l01_isRxFIFOEmpty())){
				temporaryString1[1] = 0;
				for(i=0; i<5; i++){
				temporaryString1[0] = nrf24l01_recvSPIByte();
				sprintf(&temporaryString2[0], "[0x%02x] ", temporaryString1[0]);
				l11uxx_uart_Send(&temporaryString1[0]);
				l11uxx_uart_Send(&temporaryString2[0]);
				}
				/*temporaryString1[1] = nrf24l01_recvSPIByte();
				temporaryString1[2] = nrf24l01_recvSPIByte();
				temporaryString1[3] = nrf24l01_recvSPIByte();
				temporaryString1[4] = nrf24l01_recvSPIByte();*/

				//temporaryString2[0] = temporaryString1[i];
				//temporaryString2[1] = 0;
				/*sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[0]);
				l11uxx_uart_Send(&temporaryString2[0]);
				sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[1]);
				l11uxx_uart_Send(&temporaryString2[0]);
				sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[2]);
				l11uxx_uart_Send(&temporaryString2[0]);
				sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[3]);
				l11uxx_uart_Send(&temporaryString2[0]);
				sprintf(&temporaryString2[0], "0x%02x ", temporaryString1[4]);
				l11uxx_uart_Send(&temporaryString2[0]);*/

				//}
				nrf24l01_CSDisable();
				//i--;

			}
			l11uxx_uart_Send(".\n\r");

			if(statusReg & 0x40){ //new data received
								nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x40)); //clear data received interrupt
								l11uxx_uart_Send("Cleared data received interrupt\n\r");
							}
			//nrf24l01_flushRxBuffer ();

			delay(100);
			strcpy(temporaryString1, "                              ");
		}

	return;
}

void txRoutine(){
	int statusReg=0;
	int printfdebug = 0;
	l11uxx_uart_Send("TxRoutine is go!\n\r");
	if(printfdebug) printf("TxRoutine is go!\n\r");

	/*delay(100);
	//	nrf24l01_pwrupDisable();
		delay(100);

		delay(100);
			nrf24l01_primRXDisable();
			delay(100);



	delay(100);
	nrf24l01_setTxAddr(0x456789ABCD);
	delay(100);
	//nrf24l01_setRxAddr(0xACDC000000);
	nrf24l01_setRxAddr(0x456789ABCD); //autoack thing???

	delay(100);


	delay(100);
		nrf24l01_pwrupEnable();
		delay(100);*/


		nrf24l01_writeRegister_withFrame (0x00, 0x1E); //config, tx
			delay(100);

	GPIOSetValue(1, 13, 1);
	GPIOSetValue(1, 14, 1);
	nrf24l01_getConfigData();
	while(1){
		if(!(nrf24l01_isTxFIFOFull())){
			if(GPIOGetValue(0,2)){
			nrf24l01_CSEnable();
			nrf24l01_writeTxPayload ();
			nrf24l01_sendSPIByte(65);//nrf24l01_sendSPIByte("A");
			nrf24l01_sendSPIByte(121);//nrf24l01_sendSPIByte("y");
			nrf24l01_sendSPIByte(121);//nrf24l01_sendSPIByte("y");
			nrf24l01_sendSPIByte(32);//nrf24l01_sendSPIByte("!");
			nrf24l01_sendSPIByte(108);//nrf24l01_sendSPIByte("");
			nrf24l01_CSDisable();

			nrf24l01_CSEnable();
			nrf24l01_writeTxPayload ();
			nrf24l01_sendSPIByte(109);//nrf24l01_sendSPIByte("A");
			nrf24l01_sendSPIByte(97);//nrf24l01_sendSPIByte("y");
			nrf24l01_sendSPIByte(111);//nrf24l01_sendSPIByte("y");
			nrf24l01_sendSPIByte(33);//nrf24l01_sendSPIByte("!");
			nrf24l01_sendSPIByte(33);//nrf24l01_sendSPIByte("");
			nrf24l01_CSDisable();

			l11uxx_uart_Send("Crap to Tx FIFO!\n\r");
			delay(100);
			if(printfdebug) printf("Crap successfully in Tx FIFO!\n\r");
			}
			else if(printfdebug) printf("Skipped transmission\n\r");
		} else{
			if(printfdebug) printf("Failed to send!\n\r");
			nrf24l01_flushTxBuffer ();


			if(printfdebug) printf("Flushed buffer!\n\r");
		}

		nrf24l01_CEEnable();
		GPIOSetValue(1, 13, 0); //LED shines while transmitting
		delay(1); //"don't keep in Tx mode for more than 4ms at a time"???
		nrf24l01_CEDisable();

		delay(100); //cause my eye is slow.
		GPIOSetValue(1, 13, 1);

		statusReg=nrf24l01_readRegister_withFrame(0x07); //read status register
		while(statusReg & 0x10){
			if(printfdebug) printf("MAT_RT interrupt active\n\r"); //data send failed???
			//nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x10)); //clear interrupt
			while(statusReg & 0x10){
				nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x10)); //clear interrupt
				nrf24l01_CEEnable();
						GPIOSetValue(1, 13, 0); //LED shines while transmitting
						delay(1); //"don't keep in Tx mode for more than 4ms at a time"???
						nrf24l01_CEDisable();

						delay(100); //cause my eye is slow.
						GPIOSetValue(1, 13, 1);
						statusReg=nrf24l01_readRegister_withFrame(0x07); //read status register
			}
			if(printfdebug) printf("MAT_RT interrupt cleared\n\r");
		}	if(statusReg & 0x20){ //new data received
			if(printfdebug) printf("TX_DS interrupt active\n\r"); //All cool, data sent
			nrf24l01_writeRegister_withFrame (0x07, (statusReg|0x20)); //clear interrupt
			if(printfdebug) printf("TX_DS interrupt cleared\n\r");
		}


		delay(1000);




	}
}


void nrf24l01_poorExample(){
	//copied out of "main" cause it was cluttering for way long time
	debugOutput("NRF setup\n\r");
	nrf24l01_pin_init();
	delay(100);
	nrf24l01_pwrupDisable(); //lol otherwise can't write registers?
	delay(100);
	//http://gizmosnack.blogspot.com.ee/2013/04/tutorial-nrf24l01-and-avr.html
	delay(100);
	nrf24l01_writeRegister_withFrame (0x01, 0x01); //en_aa
	nrf24l01_writeRegister_withFrame (0x02, 0x01); //en_rxaddr
	nrf24l01_writeRegister_withFrame (0x03, 0x03); //setup_aw
	nrf24l01_writeRegister_withFrame (0x05, 1); //rf_ch
	nrf24l01_writeRegister_withFrame (0x06, 0x07); //rf_setup
	nrf24l01_setRxAddr(0x1212121212);
	nrf24l01_setTxAddr(0x1212121212);
	nrf24l01_writeRegister_withFrame (0x11, 5); //rx_pw_p0



	debugOutput("NRF setup done\n\r");

	if(!(GPIOGetValue(0, 2))) txRoutine();
	if(!(GPIOGetValue(0, 7)))  rxRoutine();

	debugOutput("You missed your chance for NRF activity. Good job...\n\r");





}
