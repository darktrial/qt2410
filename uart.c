#include "sys.h"
#include <stdarg.h>
#include "isr.h"
// set transmit queue and recv queue
#define QMAXSIZE 0x0400
#define QMAXMASK (QMAXSIZE-1)

typedef struct
{
    char buff[QMAXSIZE];// data buffer
    int wptr;			// write ptr
    int rptr;			// read ptr
} UART_BUFFER;

UART_BUFFER RxQQ; // declare RX Queue
UART_BUFFER TxQQ; // declare TX Queue

void UART0_Init(void)
{
	int	i;

	rUFCON0 = 0x0;		//FIFO disable
	rUBRDIV0   =((int)(50000000/16./115200 + 0.5) -1);		// set baud rate P50mhz
	//rUBRDIV0   =((int)(25000000/16./115200 + 0.5) -1);		// set baud rate p25mhz
	rULCON0  = 0x3;		//Normal,No parity,1 stop,8 bit
	//rUCON0   = 0x345;	//rx=edge,tx=level,disable timeout int.,enable rx error int.,normal,interrupt or polling
	//rUMCON0 = 0x0;
	//rUCON0|= (1<<9)|(1<<8)|(0<<7)|(1<<6)|(0<<5)|(0<<4)|(1<<2)|(1);
	rUCON0=0x245;
	while(!(rUTRSTAT0 & 0x4));
	SUBSRCPND_REG=(BIT_SUB_RXD0|BIT_SUB_TXD0|BIT_SUB_ERR0);
	INTSUBMSK_REG=~(BIT_SUB_ERR0|BIT_SUB_RXD0);

	RxQQ.wptr = 0;						// reset RX Queue
	RxQQ.rptr = 0;

	for(i=0; i < QMAXSIZE; i++)
	{
		RxQQ.buff[i]	= '\0';
	}

	TxQQ.wptr = 0;						// reset	TX Queue
	TxQQ.rptr = 0;

	for(i=0; i < QMAXSIZE; i++)
	{
		TxQQ.buff[i]	= '\0';
	}
}
// UART0 RX isr
void UART0_RX_ISR(void)
{
    //if (!(rUTRSTAT0 & USTAT_ERROR)) // if error
    //{
        if (RxQQ.wptr + 1 != RxQQ.rptr)   // enough space of buffer?
        {
            RxQQ.buff[RxQQ.wptr++] = RdURXH0(); // read from urxh0

            if(RxQQ.wptr == QMAXSIZE)    // verify ptr
            {
                RxQQ.wptr = 0; /*loop back*/
            }
        }
    //}
    //printf("read int\n");
}

// UART0 TX isr
void UART0_TX_ISR(void)
{
    while (TxQQ.rptr != TxQQ.wptr)       
    {
        if (rUTRSTAT0 & 0x2)   
        //while (!(rUTRSTAT0 & 0x2));
        {
           rUTXH0 = TxQQ.buff[TxQQ.rptr++];
            if (TxQQ.rptr == QMAXSIZE)  
            {
                TxQQ.rptr=0; /*loop back*/
            }
        }
    }
}

void UARTIrq(void)
{
	
	if (SUBSRCPND_REG&BIT_SUB_RXD0)
	{	
		UART0_RX_ISR();
		SUBSRCPND_REG=(BIT_SUB_RXD0|BIT_SUB_ERR0);	// Clear Sub int pending    
    	//INTSUBMSK_REG&=~(BIT_SUB_RXD0|BIT_SUB_ERR0);    
    }
    else if (SUBSRCPND_REG&BIT_SUB_TXD0)
    {
    	UART0_TX_ISR();
    	SUBSRCPND_REG=(BIT_SUB_TXD0);	// Clear Sub int pending    
    }
    else
    {
     SUBSRCPND_REG=(BIT_SUB_RXD0|BIT_SUB_ERR0|BIT_SUB_TXD0); 
    }
    
}
//user api
int ReadComPort(char *data, int len)
{
    int i = 0;

    while (len > 0 && RxQQ.wptr != RxQQ.rptr)	
    {                              			
        --len;                             
        data[i++] = RxQQ.buff[RxQQ.rptr++]; 
        RxQQ.rptr &= QMAXMASK;              
    }

    return i;                              
}

int WriteComPort(char *data, int len)
{
   int i = 0;

    while (len > 0 && (((TxQQ.wptr + 1) & QMAXMASK) != TxQQ.rptr))   
    {                                          
        TxQQ.buff[TxQQ.wptr] = data[i++];         
        TxQQ.wptr = (TxQQ.wptr + 1) & QMAXMASK;  
        --len;                                 
    }

    // Send first byte of queue to generate TX interrupt.
    if (TxQQ.rptr != TxQQ.wptr)
    {
        while (!(rUTRSTAT0 & 0x2));
        
         rUTXH0 = TxQQ.buff[TxQQ.rptr++];
         if (TxQQ.rptr == QMAXSIZE)
         {
          	TxQQ.rptr=0; /*loop back*/
         }
    }

    return i;
}

