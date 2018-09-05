#include "sys.h"

unsigned long *mmu_tlb_base = (unsigned long *)  MMU_TABLE_BASE;
void mmu_tlb_init()
{
	unsigned long entry_index;
	 
	
	/*SRAM*/
	/*for (sectionNumber = 0; sectionNumber < 4096; sectionNumber++) {
		pageoffset = (sectionNumber << 20);
		*(mmu_tlb_base + (pageoffset >> 20)) = pageoffset | MMU_SECDESC;
	}*/
	
	/*4k sram*/
	for(entry_index = 0x0; entry_index < 0x1000;entry_index+=0x100000) 
 	{
 		/*section table's entry:AP=0b11,domain=0,Cached,write-through mode(WT)*/
 		*(mmu_tlb_base+(entry_index>>20))=entry_index|(0x03<<10)|(0<<5)|(1<<4)|(0<<2)| 0x02;
 	}
	//for NGCS3 ethernet
	for(entry_index = 0x18000000; entry_index < 0x20000000;entry_index += 0x100000)
 	{
 		/*section table's entry:AP=0b11,domain=0,NCNB*/	
 		*(mmu_tlb_base+(entry_index>>20)) =entry_index |(0x03<<10)|(0<<5)|(1<<4)|(0<<2)| 0x02;
 	} 
	
	
     /*SDRAM*/
 	for(entry_index = 0x30000000; entry_index < 0x34000000;entry_index+=0x100000)
 	{
 		/*section table's entry:AP=0b11,domain=0,Non-Cached,write-through mode(WT)*/
 		
 		*(mmu_tlb_base+(entry_index>>20))=entry_index |(0x03<<10)|(0<<5)|(1<<4)|(1<<3)|(0<<2)|0x02;
 		//REG32( MMU_TABLE_BASE+(entry_index>>20))=entry_index |(0x03<<10)|(0<<5)|(1<<4)|(1<<3)|0x02;
 		//weired 4 byte alignment problems
 		
 	}
 	/*SFR*/
 	for(entry_index = 0x48000000; entry_index < 0x60000000;entry_index += 0x100000)
 	{
 		/*section table's entry:AP=0b11,domain=0,NCNB*/	
 		*(mmu_tlb_base+(entry_index>>20)) =entry_index |(0x03<<10)|(0<<5)|(1<<4)|(0<<2)| 0x02;
 	}
 	/*exception vector*/
 	/*section table's entry:AP=0b11,domain=0,Cached,write-through mode(WT)*/
 	//*(mmu_tlb_base+(0xffff0000>>20)) =(VECTORS_PHY_BASE) |(0x03<<10)|(0<<5)|(1<<4)|(1<<3)|0x02;
}
void CleanMMUTable()
{
	int i;
	for (i=0;i<4096;i++)
	{
		*(mmu_tlb_base+i)=0x0;
	}
}

void StartMMU(unsigned int mmu_base)
{
	__asm
	{
		mov	r1, #0
		mov	r1, #7 << 5
step1:	
		orr	r3, r1, #63 << 26
step2:	
		mcr	p15, 0, r3, c7, c14, 2
		subs	r3, r3, #1 << 26
		bcs	step2
		subs	r1, r1, #1 << 5
		bcs	step1
		mcr	p15, 0, r1, c7, c5, 0
		mcr	p15, 0, r1, c7, c10, 4
		
		mov r0, #0
		mcr p15, 0, r0, c7, c7, 0
		mcr p15, 0, r0, c7, c10, 4
		
		mov r0, #0
		mcr p15, 0, r0, c7, c7, 0
		mcr p15, 0, r0, c7, c10, 4
		mcr p15, 0, r0, c8, c7, 0
		mov r4,mmu_base
		mcr p15, 0, r4, c2, c0, 0
		mvn r0, #0
		mcr p15, 0, r0, c3, c0, 0
		mrc p15, 0, r0, c1, c0, 0
		bic	r0, r0, #0x3000
		bic	r0, r0, #0x0300
		bic	r0, r0, #0x0087
		orr r0, r0, #0x0002
		orr r0, r0, #0x0001
		mcr p15, 0, r0, c1, c0, 0
	}
}


