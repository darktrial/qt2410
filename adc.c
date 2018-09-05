#include "sys.h"

#define ADC_FREQ 2500000

#define LOOP 10000

volatile unsigned int preScaler;

extern void Delay(unsigned int ms);

int ReadAdc(int ch)
{
    int i;
    static int prevCh=-1;

    rADCCON = (1<<14)|(preScaler<<6)|(ch<<3);	//setup channel

    if(prevCh!=ch)
    {
		rADCCON = (1<<14)|(preScaler<<6)|(ch<<3);   //setup channel
		for(i=0;i<LOOP;i++);	//delay to set up the next channel
			prevCh=ch;
    }
    rADCCON|=0x1;   //start ADC

    while(rADCCON & 0x1);	//check if Enable_start is low
    while(!(rADCCON & 0x8000));	//check if EC(End of Conversion) flag is high

    return ( (int)rADCDAT0 & 0x3ff );
}

void Test_Adc(void) 
{
    int a0=0; //Initialize variables
    unsigned int rADCCON_save = rADCCON;
    
    preScaler = ADC_FREQ;
    printf("ADC conv. freq. = %dHz\r\n",preScaler);
    preScaler = 50000000/ADC_FREQ -1;               //PCLK:50.7MHz
    
    printf("PCLK/ADC_FREQ - 1 = %d\r\n",preScaler);
    
    while( getc() != ESC_KEY )
    {
	    a0=ReadAdc(0);
	    printf( "AIN0: %04d\r\n", a0 );
		Delay( 80 ) ;
    }
    
    //rADCCON=(0<<14)|(19<<6)|(7<<3)|(1<<2);  //stand by mode to reduce power consumption
    rADCCON = rADCCON_save;
    printf("\nrADCCON = 0x%x\r\n", rADCCON);
}
