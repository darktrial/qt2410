#include "sys.h"
#include "CS89IOMode.h"
//add for skb support
#include "skbuff.h"
#include "eth.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"
#include "utility.h"

#define IOBASEAddr 0x19000300 //nGCS3:0x18000000 | 1<<24 to choose IO mode op and plus 300h for IO registers
#define IOREAD(o)					((u_short)*((volatile u_short *)(IOBASEAddr + (o))))
#define IOWRITE(o, d)				*((volatile u_short *)(IOBASEAddr + (o))) = (u_short)(d)

extern bool startTFTP;
extern systemInfo *globalSysInfo;

//according to data sheet page 75. I/O Space operation
//Specify the offset of the PacketPage Memory to visit the registers 
//inside the pktpage memory.There is a pkt page pointer which is a base pointer 
//for us to use  
u_short ReadPktPageReg(u_short offset)
{
	*((volatile u_short *)(IOBASEAddr+PKPGPTR))=offset;
	return (u_short)(*((volatile u_short *)(IOBASEAddr +PKPGDATA0)));
	
}

u_short WritePktPageReg(u_short offset,u_short data) 
{
	*((volatile u_short *)(IOBASEAddr+PKPGPTR))=offset;
	*((volatile u_short *)(IOBASEAddr+PKPGDATA0))=data;
	
}


bool ProbeCS8900A()
{
	u_short pic=0;
	
	//After reset, the CS8900A packet page pointer
    //register (IObase+0Ah) is set to 3000h. The
	//3000h value can be used as part of the
	//CS8900A signature when the system scans
	//for the CS8900A.
	
	pic=(u_short)*((volatile u_short *)(IOBASEAddr+PKPGPTR));
	if (pic!= CS8900_SIGNATURE) return false;
	else printf("CS8900A signature:%x\n\r",pic);
	//read EISA
	pic=ReadPktPageReg(PKTPG_EISA_NUMBER);
	if (pic != CS8900_EISA_NUMBER) return false;
	else printf("EISA Registration number:%x\n\r",pic);
	//read PIC
	pic=ReadPktPageReg(PKTPG_PRDCT_ID_CODE);
	pic=pic&0x00ff;
	if (pic != CS8900_PRDCT_ID) return false;
	else printf("Product ID number:%x\n\r",pic);
	
	return true;
}

bool ResetCS8900A()
{
	u_short temp;
	int event=0;
	
	WritePktPageReg(PKTPG_SELF_CTL, SELF_CTL_RESET | SELF_CTL_LOW_BITS);
	
	while (1)
	{
		Delay(100);
		temp=ReadPktPageReg(PKTPG_SELF_ST);
		if (temp & SELF_ST_INITD ) 
		{
			event=0;
			break; 
		}else ++event;
		
		if (event>100) return false;
		
	}
	printf("CS8900A is in INITD state\n\r");
	while (1)
	{
		Delay(100);
		temp=ReadPktPageReg(PKTPG_SELF_ST);
		if ((temp & SELF_ST_SIBUSY)==0 ) 
		{
			event=0;
			break; 
		}else ++event;
		
		if (event>100) return false;
	}
	printf("CS8900A finished checking EEPROM\n\r");
	
	return true;
}

void EnableCS8900AIRQ(void)
{
	u_short temp;
	
	/* If INTERRUPT_NUMBER is 0,							*/
	/*	Interrupt request will be generated from INTRQ0 pin */
	WritePktPageReg(PKTPG_INTERRUPT_NUMBER, INTERRUPT_NUMBER);
	temp = ReadPktPageReg(PKTPG_BUS_CTL) | BUS_CTL_ENABLE_IRQ;
	WritePktPageReg(PKTPG_BUS_CTL, temp);
	temp = ReadPktPageReg(PKTPG_LINE_CTL) | LINE_CTL_RX_ON | LINE_CTL_TX_ON;
	WritePktPageReg(PKTPG_LINE_CTL,temp);
}

bool InitControlReg()
{
	u_short temp;
	u_short *MAC;
	
	MAC=(u_short *)globalSysInfo->MACAddr;
	temp =ReadPktPageReg(PKTPG_LINE_CTL);
	temp|= LINE_CTL_10_BASE_T;
	temp|= LINE_CTL_MOD_BACKOFF;
	
	WritePktPageReg(PKTPG_LINE_CTL, temp);
	WritePktPageReg(PKTPG_RX_CFG, RX_CFG_RX_OK_I_E | RX_CFG_LOW_BITS);
	WritePktPageReg(PKTPG_RX_CTL, RX_CTL_RX_OK_A | RX_CTL_IND_ADDR_A | RX_CTL_BROADCAST_A | RX_CTL_LOW_BITS);
	WritePktPageReg(PKTPG_INDIVISUAL_ADDR + 0, *MAC++);
	WritePktPageReg(PKTPG_INDIVISUAL_ADDR + 2, *MAC++);
	WritePktPageReg(PKTPG_INDIVISUAL_ADDR + 4, *MAC  );
	WritePktPageReg(PKTPG_TX_CFG, TX_CFG_LOW_BITS);
	EnableCS8900AIRQ();
	printf("-=====CS8900 init finished=====-\n\r");
	return true;
}

void InitEthernet()
{
	if (ProbeCS8900A()==false) 
	{
		printf("Can not detect CS8900A\n\r");
		return;
	}
	if (ResetCS8900A()==false)
	{
		printf("Reset CS8900A failed\n\r");
	}
	InitControlReg();
}

void TransmitPacket(u_char *buffer,u_short len)
{
	int event=0,i;
	u_short data,*ptr;
	
	
	//write TxCMD register
	IOWRITE(TxCMD, TX_CMD_START_ALL | TX_CMD_LOW_BITS);
	IOWRITE(TxLength, len);
	
	//read BusST register
	
	while (1)
	{	
		data=ReadPktPageReg(PKTPG_BUS_ST);
		if (data & BUS_ST_RDY_4_TX_NOW) break;
		else ++event;
		
		if (event>100) return;
	}
	
	ptr=(u_short *)buffer;
	//transmit packet 2 bytes a round
	while (len>0)
	{
		IOWRITE(IODATA0, *ptr);
		len-=2;
		++ptr;
	}
	//printf("Transmit packet success\n\r");
}

int ReceivePacket(char *buffer)
{
	u_short len,pktlen,temp,*ptr;
	
	temp=IOREAD(IODATA0);//discard RxStatus
	len=IOREAD(IODATA0);//read frame length
	pktlen=len;
	ptr=(u_short *)buffer;
	
	while (len>0)
	{
		temp=IOREAD(IODATA0);
		if (len==1)
		{
			*((char *)ptr)=(char)temp;
			len-=1;
			continue;
		}
		
		*ptr=temp;
		len-=2;
		++ptr;
	}
	return pktlen;	
}

void RecvRoutine()
{
	//char recv[1532];
	int i,len;
	
	/*len=ReceivePacket(recv);
	printf("\n\r-===Recieve packet size:%d===-\n\r",len);
 	for (i=0;i<len;i++)
 	{
		printf(" %2x", recv[i]);
		if ((i>1)&&((i%16)==0)) printf("\n\r");  	
	}*///uncomment test code to see more detail about our packet
	struct sk_buff *skb;
	struct ethhdr *eth_hdr;		
	
	
	
	
	skb = alloc_skb(ETH_FRAME_LEN);
	len=ReceivePacket((char *)skb->data);
	skb->len=len;
	if (startTFTP==true)
	{
		eth_hdr = (struct ethhdr *)(skb->data);					
		skb_pull(skb, ETH_HLEN);
		if (ntohs(eth_hdr->h_proto) == ETH_P_ARP)
			arp_rcv_packet(skb);
		else if(ntohs(eth_hdr->h_proto) == ETH_P_IP)						
		 	ip_rcv_packet(skb);	
	}else free_skb(skb);
}

void RxPacketStatus()
{
	u_short event;
	
	event = IOREAD(ISQ);
	//while (event = IOREAD(ISQ))
	do
	{
	
		if ( ((event & ISQ_REG_NUM   )    == REG_NUM_RX_EVENT ) &&
	   	((event & RX_EVENT_RX_OK)    == RX_EVENT_RX_OK   ) &&
	   	((event & RX_EVENT_IND_ADDR) | (event & RX_CTL_BROADCAST_A))) 
			{
				RecvRoutine();
			}
		else printf("rx error\n\r");
		event = IOREAD(ISQ);	
	}while (event);//read all the interrupt event,or we will never recv any interrupt from CS8900a
}

void TestTransmitPacket()
{
	u_char buffer[512];
	
	memset(buffer,0xff,512);
	TransmitPacket(buffer,512);
}
int board_eth_get_addr(unsigned char *addr)
{
	memcpy(addr,globalSysInfo->MACAddr, ETH_ALEN);
	return 0;
}


