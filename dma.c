#include "sys.h"
#include "dma.h"

static volatile int dmaDone;
#define	_NONCACHE_STARTADDRESS	0x32000000




void DMA_M2M(int ch,int srcAddr,int dstAddr,int tc,int dsz,int burst)
{
    int i,time;
    volatile unsigned int memSum0=0,memSum1=0;
    DMA *pDMA;
    int length;
    
    length=tc*(burst ? 4:1)*((dsz==0)+(dsz==1)*2+(dsz==2)*4);
    
    printf("[DMA%d MEM2MEM Test]\r\n",ch);

    switch(ch)
    {
    case 0:
    	pDMA = (void *)0x4b000000;//channel0 base register
    	break;
    case 1:
    	pDMA = (void *)0x4b000040;//channel1 base register
    	break;
    case 2: 
    	pDMA = (void *)0x4b000080;//channel2 base register
		break;
    case 3: 
       	pDMA = (void *)0x4b0000c0;//channel3 base register
        break;
    }
                                                                                                                            
    printf("DMA%d %8xh->%8xh,size=%xh(tc=%xh),dsz=%d,burst=%d\r\n",ch,
    		srcAddr,dstAddr,length,tc,dsz,burst);

    printf("Initialize the src.\r\n");
    
    for(i=srcAddr;i<(srcAddr+length);i+=4)
    {
    	*((unsigned int *)i)=i^0x55aa5aa5;
    	memSum0+=i^0x55aa5aa5;
    }

    printf("DMA%d start\r\n",ch);
    
    dmaDone=0;
    
    pDMA->DISRC=srcAddr;
    pDMA->DISRCC=(0<<1)|(0<<0); // inc,AHB
    pDMA->DIDST=dstAddr;
    pDMA->DIDSTC=(0<<1)|(0<<0); // inc,AHB
    pDMA->DCON=(1UL<<31)|(1<<30)|(1<<29)|(burst<<28)|(1<<27)|(0<<23)|(1<<22)|(dsz<<20)|(tc);
    //HS,AHB,TC interrupt,whole, SW request mode,relaod off
    pDMA->DMASKTRIG=(1<<1)|1; //DMA on, SW_TRIG
     
    while(dmaDone==0);
    
	printf("DMA%d end\r\n", ch);
	    
    for(i=dstAddr;i<dstAddr+length;i+=4)
    {
    	memSum1+=*((unsigned int *)i)=i^0x55aa5aa5;
    }
    
    printf("memSum0=%x,memSum1=%x\r\n",memSum0,memSum1);
    if(memSum0==memSum1)
    	printf("DMA test result--------O.K.\r\n");
    else 
    	printf("DMA test result--------ERROR!!!\r\n");

}

void Test_DMA(void)
{
    //DMA Ch 0
    //      channel,startAddress,dstAddress,size,dsz,burst
    DMA_M2M(0,_NONCACHE_STARTADDRESS,_NONCACHE_STARTADDRESS+0x80000,0x80000,0,0); //byte,single
    DMA_M2M(0,_NONCACHE_STARTADDRESS,_NONCACHE_STARTADDRESS+0x80000,0x40000,1,0); //halfword,single
    DMA_M2M(0,_NONCACHE_STARTADDRESS,_NONCACHE_STARTADDRESS+0x80000,0x20000,2,0); //word,single
    DMA_M2M(0,_NONCACHE_STARTADDRESS,_NONCACHE_STARTADDRESS+0x80000,0x20000,0,1); //byte,burst
    DMA_M2M(0,_NONCACHE_STARTADDRESS,_NONCACHE_STARTADDRESS+0x80000,0x10000,1,1); //halfword,burst
    DMA_M2M(0,_NONCACHE_STARTADDRESS,_NONCACHE_STARTADDRESS+0x80000, 0x8000,2,1); //word,burst
}

void Dma0Done(void)
{
    dmaDone=1;
}

void Dma1Done(void)
{
    dmaDone=1;
}

void Dma2Done(void)
{
    dmaDone=1;
}

void Dma3Done(void)
{
    dmaDone=1;
}