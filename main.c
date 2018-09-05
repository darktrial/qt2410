#include <stdio.h>
#include <stdlib.h>
#include "isr.h"
#include "sys.h"

#define AutoBootSeconds 5
#define SystemInformationAddress 0x3fffd00
systemInfo *sysInfo,*globalSysInfo;
u_char GlobalSysBuffer[512];
//network related
u_char CS8900Amac[6]={0x00,0x80,0x48,0x12,0x34,0x56};
char CS8900IP[20]="192.168.15.100";


extern void mmu_tlb_init(void);
extern void CleanMMUTable(void);
extern void StartMMU(unsigned int mmu_base);
extern void PortInit(void);
extern void InitIRQDevices(void);
extern void setupIRQEnv(void);
extern void Test_Adc(void);

int MyMenu()
{
	int ret;
	
	printf("\n\r-==========BIOSJY 2410==========-\n\r");
	printf("1.System Information\n\r");
	printf("2.Configurations\n\r");
	printf("3.Fdisk utility\n\r");
	printf("4.TFTP Load and Run\n\r");
	printf("5.TFTP Firmware Upgrade\n\r");
	printf("6.USB Load and Run\n\r");
	printf("7.USB Firmware Upgrade\n\r");
	printf("8.Load Linux From NAND\n\r");
	printf("9.Load Other OS From NAND\n\r");
	printf("Your Choice:");
	scanf("%d",&ret);
	return ret;
}

void ReadSystemInformation()
{
	u_int inaddress,address,block;	
	u_char buffer[512];
	
	address=linearAddressConvert(SystemInformationAddress);
	block=blockAddressConvert(SystemInformationAddress);
	ReadPage(address,buffer);
	sysInfo=(systemInfo *)buffer;
	if (strcmp(sysInfo->banner,"JoeyCheng")!=0)
	{
		EraseBlock(block);
		strcpy(sysInfo->banner,"JoeyCheng");
		memcpy(sysInfo->MACAddr,CS8900Amac,6);
		sysInfo->IPAddr=str_to_addr(CS8900IP);
		sysInfo->bootImageStart=0x0;
		sysInfo->bootImageSize=0x100000;//block 40 
		sysInfo->LinuxKernelImageStart=0x104000;//block 41
		sysInfo->LinuxKernelImageSize=0x600000;
		sysInfo->LinuxFileSystemStart=0x704000;//block 1c1
		sysInfo->LinuxFileSystemSize=0x1000000;
		sysInfo->OtherKernelImageStart=0x1704000;//block 5c1
		sysInfo->OtherKernelImageSize=0x600000;
		sysInfo->ApplicationLoadAddress=0x30008000;
		sysInfo->BootOption=3;
		strcpy(sysInfo->BootParam,"noinitrd root=/dev/mtdblock/2 console=ttySAC0,115200 mem=64M");
		WritePage(address,buffer);
	}
	memcpy(GlobalSysBuffer,buffer,512);
	globalSysInfo=(systemInfo *)GlobalSysBuffer;
}

void WriteSystemInfo()
{
	u_int inaddress,address,block;	
	u_char buffer[512],ip[20],mac[50],*temp,tempString[256],ch;
	int ret,len,index=0;
	
	address=linearAddressConvert(SystemInformationAddress);
	block=blockAddressConvert(SystemInformationAddress);
	ReadPage(address,buffer);
	sysInfo=(systemInfo *)buffer;
	
	printf("1. Set IP\n\r");
	printf("2. Set MAC\n\r");
	printf("3. Set Linux Kernel Image Start\n\r");
	printf("4. Set Linux Kernel Image Size\n\r");
	
	printf("5. Set Linux Kernel File System Start\n\r");
	printf("6. Set Linux Kernel File System Size\n\r");
	
	printf("7. Set Other Kernel Image Start\n\r");
	printf("8. Set Other Kernel Image Size\n\r");
	printf("9. Set Application Load Address\n\r");
	printf("10. Set Linux Boot parameters\n\r");
	printf("11. Set boot option\n\r");
	printf("Your Choice:");
	scanf("%d",&ret);
	printf("\n\r");
	if (ret==1)
	{
		printf("IP(Format x.x.x.x):");
		memset(ip,0x0,20); 
		scanf("%s",ip);
		sysInfo->IPAddr=str_to_addr(ip);	
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==2)
	{
		printf("MAC(Format(AABBCCDDEEFF)):"); 
		scanf("%s",mac);
		temp=(u_char *)strtohex(mac,&len);
		memcpy(CS8900Amac,temp,6);
		memcpy(sysInfo->MACAddr,temp,6);
		free(temp);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==3)
	{
		printf("Linux Kernel Address in NAND(Format hex):"); 
		scanf("%s",tempString);
		sscanf((char *)tempString,"%x",&sysInfo->LinuxKernelImageStart);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==4)
	{
		printf("Linux Kernel Size in NAND(Format hex):"); 
		scanf("%s",tempString);
		sscanf((char *)tempString,"%x",&sysInfo->LinuxKernelImageSize);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==5)
	{
		printf("Linux File System Start in NAND(Format hex):"); 
		scanf("%s",tempString);
		sscanf((char *)tempString,"%x",&sysInfo->LinuxFileSystemStart);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==6)
	{
		printf("Linux File System Size in NAND(Format hex):"); 
		scanf("%s",tempString);
		sscanf((char *)tempString,"%x",&sysInfo->LinuxFileSystemSize);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==7)
	{
		printf("Other Kernel Address in NAND(Format hex):"); 
		scanf("%s",tempString);
		sscanf((char *)tempString,"%x",&sysInfo->OtherKernelImageStart);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==8)
	{
		printf("Other Kernel Size in NAND(Format hex):"); 
		scanf("%s",tempString);
		sscanf((char *)tempString,"%x",&sysInfo->OtherKernelImageSize);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==9)
	{
		printf("Set application start address in ram(Format hex):"); 
		scanf("%s",tempString);
		sscanf((char *)tempString,"%x",&sysInfo->ApplicationLoadAddress);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==10)
	{
		printf("Set Linux Boot Parameters:"); 
		//scanf("%s",sysInfo->BootParam);
		memset(sysInfo->BootParam,0,256);
		do
		{
			ch=getch();
			if (ch==13) 
			{
				sysInfo->BootParam[index]=ch;
				break;
			}
			else
			{ 
				sysInfo->BootParam[index]=ch;
				index++;
			}
		}while (ch!=13);//13 is enter
		printf("dbg :sysinfo->param %s",sysInfo->BootParam);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else if (ret==11)
	{
		printf("Auto loading linux or other kernel(Linux:1 Other Kernel:2 NoAutoBoot:3):"); 
		scanf("%d",&sysInfo->BootOption);
		EraseBlock(block);
		memcpy(GlobalSysBuffer,buffer,512);
		WritePage(address,buffer);
	}
	else printf("Warning:Input choice is from 1 to 10\n\r");
	
}

void ShowSystemInfo()
{
	u_int inaddress,address,block;	
	u_char buffer[512];
	
	address=linearAddressConvert(SystemInformationAddress);
	block=blockAddressConvert(SystemInformationAddress);
	ReadPage(address,buffer);
	sysInfo=(systemInfo *)buffer;
	
	if (strcmp(sysInfo->banner,"JoeyCheng")!=0)
	{
		printf("System information currupted!Please check your NAND Flash status\n\r");
		return;
	}
	printf("\n\r-==========System Information==========-\n\r");
	addr_fprint(sysInfo->IPAddr);
	printf("IP Address:%s\n\r",CS8900IP);
	printf("MAC Address:0x%2x:0x%2x:0x%2x:0x%2x:0x%2x:0x%2x\n\r",sysInfo->MACAddr[0],sysInfo->MACAddr[1],sysInfo->MACAddr[2],sysInfo->MACAddr[3],sysInfo->MACAddr[4],sysInfo->MACAddr[5]);
	printf("Boot Image at NAND Flash:0x%8x\n\r",sysInfo->bootImageStart);
	printf("Boot Image size:0x%8x\n\r",sysInfo->bootImageSize);
	printf("Linux Kernel Image at NAND Flash:0x%8x\n\r",sysInfo->LinuxKernelImageStart);
	printf("Linux Kernel Image size:0x%8x\n\r",sysInfo->LinuxKernelImageSize);
	printf("Linux File System at NAND Flash:0x%8x\n\r",sysInfo->LinuxFileSystemStart);
	printf("Linux File System size:0x%8x\n\r",sysInfo->LinuxFileSystemSize);
	printf("Other Kernel at NAND Flash:0x%8x\n\r",sysInfo->OtherKernelImageStart);
	printf("Other Kernel Image size:0x%8x\n\r",sysInfo->OtherKernelImageSize);
	printf("Application Load address in ram:0x%8x\n\r",sysInfo->ApplicationLoadAddress);
	printf("Linux Boot Parameters:%s\n\r",sysInfo->BootParam);
	printf("Boot:Default to startup ");
	if (sysInfo->BootOption==LinuxStart)
		printf("Linux\n\r");
	else if (sysInfo->BootOption==OtherStart) printf("Other OS\n\r");
	else printf("No autoboot\n\r");
	memcpy(GlobalSysBuffer,buffer,512);
	globalSysInfo=(systemInfo *)GlobalSysBuffer;
}

void PerformFdisk()
{
	u_int ret,address,size;
	
	printf("\n\r-==========Fdisk NAND Flash==========-\n\r");
	printf("1.Erase by address and size\n\r");
	printf("2.Erase all(Erase 64 MB Flash except bootloader)\n\r");
	printf("Your Choice:");
	scanf("%d",&ret);
	if (ret==1)
	{
		printf("\n\rInput the addres from which you want to erase(hex):");
		scanf("%x",&address);
		printf("\n\rInput the size from which you want to erase(hex):");
		scanf("%x",&size);
		PerformErase(address,size);
	}
	else if (ret==2)
	{
		PerformTotalErase();
	}
	else printf("Warning: Choice is from 1 to 2\n\r");	
}

void TFTPLoadAndRun()
{
	StartTFTPServer();
	printf("Launching your application!!\n\r");
	disable_IRQ();
	JumpToProgram();
}

void TFTPFirmwareUpgrade()
{
	u_int len;
	int choice,result;
	u_int address,block;
	
	printf("Which area you want to upgrade\n\r");
	printf("1.Linux Kernel\n\r");
	printf("2.Linux File System\n\r");
	printf("3.Other Kernel Image\n\r");
	printf("4.Bootloader\n\r");
	printf("Choice:");
	scanf("%d",&choice);
	printf("\n\r");
	
	len=StartTFTPServer();
	result=NANDFlashUpgrade(choice,len);
	if (result==true)
	{
		printf("Update image size\n\r");
		address=linearAddressConvert(SystemInformationAddress);
		block=blockAddressConvert(SystemInformationAddress);
		EraseBlock(block);
		WritePage(address,GlobalSysBuffer);
	}
}

void USBLoadAndRun()
{
	int ret;
	
	ret=USBDownload();//DMA support
	//usbTest();//Withought DMA
	if (ret==0)
	{
		printf("Image is currupted\n\r");
		return;
	}
	printf("Launching your application!!\n\r");
	disable_IRQ();
	JumpToProgram();
}

void USBFirmwareUpgrade()
{
	u_int len;
	int choice,result;
	u_int address,block;
	
	printf("Which area you want to upgrade\n\r");
	printf("1.Linux Kernel\n\r");
	printf("2.Linux File System\n\r");
	printf("3.Other Kernel Image\n\r");
	printf("4.Bootloader\n\r");
	printf("Choice:");
	scanf("%d",&choice);
	printf("\n\r");
	
	len=USBDownload();
	if (len==0)
	{
		printf("Image is currupted\n\r");
		return;
	}
	result=NANDFlashUpgrade(choice,len);
	if (result==true)
	{
		address=linearAddressConvert(SystemInformationAddress);
		block=blockAddressConvert(SystemInformationAddress);
		EraseBlock(block);
		WritePage(address,GlobalSysBuffer);
	}
}

void LoadLinuxFromNAND()
{
	u_int address,temp;
	bool ret;
	char *buffer=(char *)globalSysInfo->ApplicationLoadAddress;
	
	temp=globalSysInfo->LinuxKernelImageStart;
	
	for (temp=globalSysInfo->LinuxKernelImageStart;
		 temp<globalSysInfo->LinuxKernelImageStart+globalSysInfo->LinuxKernelImageSize;
		 temp+=0x200)
	{
		address=linearAddressConvert(temp);
		ret=ReadPage(address,buffer);
		if (ret==SUCCESS) printf(".");
		else printf("*");
		buffer+=0x200;
	}
	printf("Launching your application!!\n\r");
	disable_IRQ();
	JumpToProgram();
}

void LoadOtherFromNAND()
{
	u_int address,temp;
	bool ret;
	char *buffer=(char *)globalSysInfo->ApplicationLoadAddress;
	
	temp=globalSysInfo->OtherKernelImageStart;
	
	for (temp=globalSysInfo->OtherKernelImageStart;
		 temp<globalSysInfo->OtherKernelImageStart+globalSysInfo->OtherKernelImageSize;
		 temp+=0x200)
	{
		address=linearAddressConvert(temp);
		ret=ReadPage(address,buffer);
		if (ret==SUCCESS) printf(".");
		else printf("*");
		buffer+=0x200;
	}
	printf("Launching your application!!\n\r");
	disable_IRQ();
	JumpToProgram();
}



int main(void)
{
 	int ret;
 	u_int MyMMUBase,i=0;
 	
 	PortInit();
 	CleanMMUTable();
 	mmu_tlb_init();
 	MyMMUBase=MMU_TABLE_BASE;//MMU_TABLE_BASE+SIZE_64M;
 	StartMMU(MyMMUBase);
 	setupIRQEnv();
 	InitIRQDevices();
 	NandFlashInit();
 	//initial system information
 	ReadSystemInformation();	
 	//initial ethernet
 	InitEthernet();
 	//initial USB
 	usbInit();
 	Delay(100);
 	
 	
 	
 	if (globalSysInfo->BootOption==LinuxStart)
 	{
 		printf("AutoBoot to Linux\n\r");
 		while (getKey()==false)
 		{			
 			Delay(1000);
 			printf("Seconds left to AutoBoot:%d\n\r",AutoBootSeconds-i);
 			i++;
 			if (i>=AutoBootSeconds) LoadLinuxFromNAND();
 		}
 	}
 	else if (globalSysInfo->BootOption==OtherStart)
 	{
 		printf("AutoBoot to Other kernel\n\r");
 		while (getKey()==false)
 		{
 			
 			Delay(1000);
 			printf("Seconds left to AutoBoot:%d\n\r",AutoBootSeconds-i);
 			i++;
 			if (i>=AutoBootSeconds) LoadOtherFromNAND();
 		}
 	}
 	else printf("No AutoBoot.Go to Menu\n\r");
 	
 	while (1)
  	{
 		ret=MyMenu();
 		
 		switch (ret)
 		{
 			case 1:
 				ShowSystemInfo();
 				break;
 			case 2:
 				WriteSystemInfo();
 				break;
 			case 3:
 				PerformFdisk();
 				break;
 			case 4:
 				TFTPLoadAndRun();
 				break;
 			case 5:
 				TFTPFirmwareUpgrade();
 				break;
 			case 6:
 				USBLoadAndRun();
 				break;
 			case 7:
 				USBFirmwareUpgrade();
 				break;
 			case 8:
 				LoadLinuxFromNAND();
 				break;
 			case 9:
 				LoadOtherFromNAND();
 				break;
 			default:
 				break;
 		}
 		
  	}
}