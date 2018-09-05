#include "sys.h"

#define ReadMode  1
#define NONE 0


unsigned int GPEConsave,GPEHSave,OPMode;
volatile unsigned int iicMode,iicStatus;
bool ackReceived=false,dataReceived=false;

void IICInit()
{
	
	GPEConsave=rGPECON;
	GPEHSave=rGPEUP;
	rGPEUP  |= 0xc000;                  //Pull-up disable(High impedence disable for current from low to high)
    rGPECON &= ~0xf0000000;
    rGPECON |= 0xa0000000;              //GPE15:IICSDA , GPE14:IICSCL 
	rIICCON=0x1<<7|0x1<<6|0x1<<5;       //IICCLK=pclk/512=50MHZ/512==100KHZ for KS24C080 working in standard mode
	rIICADD  = 0x10;                    //2410 slave address = [7:1]
    rIICSTAT = 0x10; 
}
//according to KS24C080 data sheet page 2-8
//byte write operation is written as listed below
//1.write slave address and wait for ack
//2.write word address and wait for ack
//3.write data and wait for ack
void WriteIICData(u_char SlaveAddress,u_char WriteAddress,u_char data)
{
	rIICDS=SlaveAddress;
	rIICSTAT=0xF0;
	while (ackReceived==false);
	ackReceived=false;
	Delay(1);
	
	rIICDS=WriteAddress;
	rIICCON = 0xe0;
	while (ackReceived==false);
	ackReceived=false;
	Delay(1);
	
	rIICDS=data;
	rIICCON = 0xe0;
	while (ackReceived==false);
	ackReceived=false;
	rIICSTAT = 0xd0;//Master Tx condition, Stop(Write), Output Enable
	rIICCON  = 0xe0;//Resumes IIC operation
	Delay(100);
                 		
}
//according to KS24C080 data sheet page 2-13
//This eeprom can perform random address byte read operation
//in the way listed below
//1 Write op start,Write slave address, wait for ack
//2 Write word address, wait for ack
//3 Read op start, write slave address,wait for ack
//4.Wait for data with no ack
char ReadIICData(u_char SlaveAddress,u_char ReadAddress)
{
	char data;
	
	rIICDS=SlaveAddress;
	rIICSTAT=0xF0;
	while (ackReceived==false);
	ackReceived=false;
	Delay(1);
	
	rIICDS=ReadAddress;
	rIICCON = 0xe0;
	while (ackReceived==false);
	ackReceived=false;
	Delay(1);
	
	rIICDS=SlaveAddress;
	rIICSTAT=0xB0;
	rIICCON=0xe0;
	while (ackReceived==false);
	ackReceived=false;
	Delay(1);
	
	OPMode=ReadMode;
	rIICCON=0x60;//
	while (dataReceived==false);
	dataReceived=false;
	data=rIICDS;
	rIICSTAT = 0x90;//Master Rx condition Stop Read
	rIICCON  = 0xe0;//Resumes IIC operation
    Delay(100);
    OPMode=NONE; 
    
    return data;   
}

void IICInt(void)
{
	iicStatus=rIICSTAT;
	
	if ((iicStatus&0x1)==0x0) ackReceived=true;
	if (OPMode==ReadMode)
	{
		dataReceived=true;
	}
	
}

void TestIIC()
{
	char data[10];
	int i;
	
	printf("IIC initial\n\r");
	IICInit();
	printf("Write to IIC\n\r");
	for (i=0;i<9;i++)
		WriteIICData(0xA0,i,i);
	for (i=0;i<9;i++)
		data[i]=0;
	printf("Read from IIC \n\r");
	for (i=0;i<9;i++)
	{
		data[i]=ReadIICData(0xA0,i);
	}
	
	for (i=0;i<9;i++)
	{
		printf("%2x ",data[i]);
	}
	//test finished
	rGPECON=GPEConsave;
	rGPEUP=GPEHSave;
	
}