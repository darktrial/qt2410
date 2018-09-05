
// INTERRUPT
//=================
#define SRCPND 0x4a000000    //Interrupt request status
#define INTMOD 0x4a000004    //Interrupt mode control
#define INTMSK 0x4a000008    //Interrupt mask control
#define PRIORITY 0x4a00000a  //IRQ priority control
#define INTPND 0x4a000010    //Interrupt request status
#define INTOFFSET 0x4a000014    //Interruot request source offset
#define SUBSRCPND 0x4a000018    //Sub source pending
#define INTSUBMSK 0x4a00001c    //Interrupt sub mask

// 巨集功能

#define REG32(addr)    (*(volatile unsigned long *)(addr))
#define REG16(addr)    (*(volatile unsigned short *)(addr))
#define REG8(addr)    (*(volatile unsigned char *)(addr))

#define SRCPND_REG REG32(SRCPND)    //Interrupt request status
#define INTMOD_REG REG32(INTMOD)    //Interrupt mode control
#define INTMSK_REG REG32(INTMSK)    //Interrupt mask control
#define PRIORITY_REG REG32(PRIORITY)    //IRQ priority control
#define INTPND_REG REG32(INTPND)   //Interrupt request status
#define INTOFFSET_REG REG32(INTOFFSET)    //Interruot request source offset
#define SUBSRCPND_REG REG32(SUBSRCPND)    //Sub source pending
#define INTSUBMSK_REG REG32(INTSUBMSK)    //Interrupt sub mask
#define rEINTMASK  (*(volatile unsigned *)0x560000a4) //External interrupt mask

#define	Enable_Int(n)           INTMSK_REG &= ~(1<<(n))
#define	Disable_Int(n)			INTMSK_REG |= (1<<(n))
#define Clear_SRCPendingBit(n)     SRCPND_REG = (1<<(n))
#define Clear_INTPendingBit(n)     INTPND_REG = (1<<(n))
#define Enable_EInt(n)          rEINTMASK&=~(1<<(n)) 
#define Disable_EInt()            rEINTMASK=0xffffff
#define Clear_EIntPendingBit(n) rEINTPEND=(1<<(n))
// 中斷向量最大值
#define	IRQ_INT_NO	32

// 定義各種中斷向量的數值
#define	nEXT0_INT		0 
#define	nEXT1_INT		1
#define	nEXT2_INT		2
#define	nEXT3_INT		3
#define nEXT4_7_INT     4 
#define nEXT8_23_INT    5 
#define nReserved       6
#define	nBATT_FLT_INT	7
#define	nTICK_INT		8
#define	nWDT_INT		9
#define	nTIMER0_INT		10
#define	nTIMER1_INT		11
#define	nTIMER2_INT		12
#define	nTIMER3_INT		13
#define	nTIMER4_INT		14
#define	nUART2_INT		15
#define	nLCD_INT		16
#define	nDMA0_INT		17
#define	nDMA1_INT		18
#define	nDMA2_INT		19
#define	nDMA3_INT		20
#define	nSDI_INT		21 
#define	nSPI0_INT		22
#define	nUART1_INT		23
#define	Reserved		24
#define	nUSBD_INT		25
#define	nUSBH_INT		26
#define	nIIC_INT		27
#define UART0_INT       28
#define	nSPI1_INT		29
#define	nRTC_INT		30
#define	nADC_INT		31

//external int definitions
#define IRQ_LAN 9

__inline void enable_IRQ(void)
{
  int tmp;
  __asm
  {
    MRS tmp, CPSR
    BIC tmp, tmp, #0x80
    MSR CPSR_c, tmp
  }
}

__inline void disable_IRQ(void)
{
  int tmp;
  __asm
  {
    MRS tmp, CPSR
    ORR tmp, tmp, #0x80
    MSR CPSR_c, tmp
  }
}

#define ClearPending(bit) {\
                rSRCPND = bit;\
                rINTPND = bit;\
                rINTPND;\
                }       
      

