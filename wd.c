#include "sys.h"

#define WDT_ENABLE		(0x01<<5)
#define WDT_INT_ENABLE	(0x01<<2)
#define WDT_RST_ENABLE	(0x01<<0)	

#define WDT_CLK_SEL		(0x3<<3)		/* 1/128 */
#define WDT_PRE_SCALER	(0xFF<<8)		/* 255    */

void WDInit()
{
	rWTCNT = 0x1000;
	rWTCON = WDT_ENABLE | WDT_RST_ENABLE | WDT_CLK_SEL | WDT_PRE_SCALER;
}

void doReset()
{
	rWTCNT = 0x100;
	rWTCON = WDT_ENABLE | WDT_RST_ENABLE | WDT_CLK_SEL | WDT_PRE_SCALER;
}

