#include "sys.h"

/*static NandFlashChip[] = 
{
	{0xec73, SIZE_16M},
	{0xec75, SIZE_32M},
	{0xec76, SIZE_64M},
	{0xec79, SIZE_128M},
	{0, 0},
};*/
u_int nand_id;
//NAND Command
#define	READCMD0	0
#define	READCMD1	1
#define	READCMD2	0x50
#define	ERASECMD0	0x60
#define	ERASECMD1	0xd0
#define	PROGCMD0	0x80
#define	PROGCMD1	0x10
#define	QUERYCMD	0x70
#define	READIDCMD	0x90
//ECC Macro
#define NF_RSTECC()	{rNFCONF|=(1<<12);}

u_char NFReadData(void)
{
	return rNFDATA;
}

char NFStatReady()
{
	return rNFSTAT&0x1;
}

void NFWriteData(char data)
{
	rNFDATA = data;
}

void NFWriteAddress(char address)
{
	rNFADDR=address;
}

void NFWriteCommand(char cmd)
{
	rNFCMD=cmd;
}

void NFEnable()
{
	rNFCONF&=~0x800;//low active
}

void NFDisable()
{
	rNFCONF|=0x800;//high inactive
}
u_char NFRetrieveNANDStatus()
{
	u_char stat=0;
	//acording to K9F1208 spec, we could read I/O6 to identify whether it is busy or ready
	//<Command>70h
	NFEnable();
	NFWriteCommand(QUERYCMD);
	stat  = NFReadData();
	NFDisable();
	return stat;
}
u_char busyWait()
{
	u_char stat=0;
	
	NFWriteCommand(QUERYCMD);
	do 
	{
		stat  = NFReadData();
	}while (!(stat&0x40));
	return stat&0x1;
	
}
// 1block=(512+16)bytes x 32pages
// 4096block

// A[25:14][13:9]
//  block   page
int EraseBlock(u_int Address)
{
	//we have 4096 blocks so we use 12 bit to represent in 4 cycles 
	u_char stat;
	
	Address=Address<<5;//page number is 0
	NFEnable();
	NFWriteCommand(ERASECMD0);
	//NFWriteAddress(0);//column(first cycle)
	NFWriteAddress(Address&0xff);//page(second cycle A9~A16)
	NFWriteAddress((Address>>8)&0xff);//block (third cycle A17~A24)
	NFWriteAddress((Address>>16)&0xff);//A25
	NFWriteCommand(ERASECMD1);
	stat=busyWait();
	NFDisable();
	if (!stat) return SUCCESS;
	else return FAIL;
	
}
bool WritePage(u_int Address,u_char *buffer)
{
	//A0~A7 Column address
	//A9~A24 Row address
	int i;
	u_char stat;
	u_char eccBuffer[16]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	
	NF_RSTECC();
	NFEnable();
	NFWriteCommand(PROGCMD0);
	NFWriteAddress(Address&0xff);//column(first cycle)
	NFWriteAddress((Address>>8)&0xff);//page(second cycle A9~A16)(actual used A9-13)
	NFWriteAddress((Address>>16)&0xff);//block (third cycle A17~A24(A14-25))
	NFWriteAddress((Address>>24)&0xff);//A25
	for(i=0; i<512; i++)
		NFWriteData(buffer[i]);
	//ecc
	eccBuffer[0]=rNFECC0;
    eccBuffer[1]=rNFECC1;
    eccBuffer[2]=rNFECC2;
    eccBuffer[5]=0xff;		// Marking good block
    for (i=0;i<16;i++)
    	NFWriteData(eccBuffer[i]);
	NFWriteCommand(PROGCMD1);
	stat=busyWait();
	NFDisable();
	if (!stat) return SUCCESS;//printf("sucess program\n\r");
	else FAIL;//printf("Error program\n\r");
}

bool ReadPage(u_int Address,u_char *buffer)
{
	//A0~A7 Column address
	//A9~A24 Row address
	int i;
	u_char ecc[3],eccBuffer[16];
	
	NF_RSTECC();
	NFEnable();
	NFWriteCommand(READCMD0);
	NFWriteAddress(Address&0xff);//column(first cycle)
	NFWriteAddress((Address>>8)&0xff);//page(second cycle A9~A16)(actual used A9-13)
	NFWriteAddress((Address>>16)&0xff);//block (third cycle A17~A24(A14-25))
	NFWriteAddress((Address>>24)&0xff);//A25
	
	while (!NFStatReady());
	
	for (i=0;i<512;i++)
	{
		buffer[i]=NFReadData();
	}
	ecc[0]=rNFECC0;
    ecc[1]=rNFECC1;
    ecc[2]=rNFECC2;
	for (i=0;i<16;i++)
	{
		eccBuffer[i]=NFReadData();
	}
	NFDisable();
	//check ecc
	if (ecc[0]==eccBuffer[0] && ecc[1]==eccBuffer[1] && ecc[2]==eccBuffer[2])
		return SUCCESS;//printf("Read Completed!ECC Check pass\n\r");
	else return FAIL;//printf("Read Failed!ECC Check failed\n\r");
}



u_int NFRetrieveNANDInfo()
{
	u_int id=0;
	//acording to K9F1208 spec
	//<Command>90h <Addr>00h 
	NFEnable();
	NFWriteCommand(READIDCMD);
	NFWriteAddress(0);
	while (!NFStatReady());
	id  = NFReadData()<<24;
	id |= NFReadData()<<16;
	id |= NFReadData()<<8;
	id |= NFReadData();
	NFDisable();
	return id;
}


u_int CombineAddress(u_int column,u_int page,u_int block)
{
	//512 bytes per page,32 pages in 1 block, and there are 4096 blocks in  QT2410 K9F1208(64MB)
	//Column addres is always start from 0 so A0~A7=0
	//set A9~13=page address
	//set A14~A25=block address
	u_int address=0;
	
	address=column | page<<8 | block << 13;
	return address;
	
}

u_int linearAddressConvert(u_int adr)
{
	u_int column,page,block;
	
	column=adr %512;
	page=adr/512;
	block=adr/16384;
	//printf("Column %2x,page %2x,block %2x\n\r",column,page,block);
    return CombineAddress(column,page,block);
}

u_int blockAddressConvert(u_int adr)
{
	return adr/16384;
}

void NandFlashInit(void)
{
	u_int stat;
	
	//512 bytes per page,32 pages in 1 block, and there are 4096 blocks in  QT2410 K9F1208(64MB)
	//        Enable   ECC     CE      TACLS TWRPH0 TWRPH1
	rNFCONF = (1<<15)|(1<<12)|(1<<11)|(7<<8)|(7<<4)|(7);
	
	nand_id = NFRetrieveNANDInfo();
	
	printf("NAND ID:%x\n\r",nand_id);
	
	stat=NFRetrieveNANDStatus();
	
	printf("NAND STAT:%x\n\r",stat);	
}
//we perform erase->write->read process for demo only
void NANDFlashTest()
{
	int i;
	u_int stat,inaddress,address,outblock;	
	u_char buffer[512];
	
	if (nand_id!=0xec76a5c0) 
	{
		printf("NAND Test Failed!Because your falsh id doesnt match 0xEC76A5C0\n\r");
	}
	inaddress=0x2000000;//flash address is between 0x0 ~ 0x3FFFFFF
	address=linearAddressConvert(inaddress);
	outblock=blockAddressConvert(inaddress);
	
	EraseBlock(outblock);
	memset(buffer,0x0,512);
	for (i=0;i<512;i++)//set test data
		buffer[i]=i;
	WritePage(address,buffer);
	memset(buffer,0x0,512);
	ReadPage(address,buffer);
	printf("buffer content:\n\r");
	for (i=0;i<512;i++)
	{
		printf(" %2x",buffer[i]);
		if (i>0 && i%16==0) printf("\n\r");
	}
	printf("\n\rNAND Flash read/write test completed\n\r");
}

void PerformErase(u_int inaddress,u_int size)
{
	u_int i,address,outblock;
	int ret;
	
	address=linearAddressConvert(inaddress);
	outblock=blockAddressConvert(inaddress);
	
	printf("Start Erasing\n\r");
	for (i=0;i<size;i+=0x4000)//32(page)*512(bytes) per block
	{
		ret=EraseBlock(outblock);
		outblock++;
		if (ret==SUCCESS)
			printf(".");
		else printf("*");
	}
	printf("\n\rErase block success\n\r");
}

void PerformTotalErase()
{
	u_int i;
	int ret;
	
	printf("Start Erasing\n\r");
	for (i=41;i<4096;i++)
	{
		ret=EraseBlock(i);
		if (ret==SUCCESS)
			printf(".");
		else printf("*");
	}
	printf("\n\rErase Success\n\r");
}
