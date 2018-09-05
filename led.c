#include "sys.h"
void led_on()
{
	rGPBDAT=0x00000000;//control IRDA led to light on
}

void led_off()
{
	rGPBDAT=0x00000002;
}