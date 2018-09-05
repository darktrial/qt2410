/*
 * Copyright (C) ARM Limited, 1999. All rights reserved.
 */


/*
This implements a 'retarget' layer for low-level IO.  Typically, this would 
contain your own target-dependent implementations of fputc(), ferror(), etc.
This example provides implementations of fputc(), ferror(), _sys_exit(), 
_ttywrch() and __user_initial_stackheap().
Here, semihosting SWIs are used to display text onto the console of the host debugger.
This mechanism is portable across ARMulator, Angel, Multi-ICE and EmbeddedICE.
Alternatively, to output characters from the serial port of an ARM Development (PID) Board
(see serial.c), use '#define USE_SERIAL_PORT' or compile with '-DUSE_SERIAL_PORT'.
*/

#include <stdio.h>
#include <rt_misc.h>


#ifdef __thumb
/* Thumb Semihosting SWI */
#define SemiSWI 0xAB
#else
/* ARM Semihosting SWI */
#define SemiSWI 0x123456
#endif


/* Write a character */ 
__swi(SemiSWI) void _WriteC(unsigned op, char *c);
#define WriteC(c) _WriteC (0x3,c)

/* Exit */
__swi(SemiSWI) void _Exit(unsigned op, unsigned except);
#define Exit() _Exit (0x18,0x20026)


struct __FILE { int handle;   /* Add whatever you need here */};
//FILE __stdout;


extern int ReadComPort(char *data, int len);
extern int WriteComPort(char *data, int len);

int fputc(int ch, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to a UART, or to the debugger console with SWI WriteC */
	char tempch = ch;
	char temp='\n';

   /* if (ch=='\r')
    {
    	WriteComPort(&tempch, 1);
    	WriteComPort(&temp, 1);
    }
    else*/ 
    WriteComPort(&tempch, 1);
    return ch;
    
}

int getc(FILE *f)
{
	int x;
	char tempch;
	
	if(ReadComPort(&tempch, 1)==1)
	{
		x=tempch;
		WriteComPort(&tempch, 1);
		return x;
	}
	else
		return EOF;	
}

int fgetc(FILE *f)
{
	int x;
	char tempch;
	
	while(ReadComPort(&tempch, 1)==0);
	x=tempch;
	WriteComPort(&tempch, 1);
	return x;
/*		 
	if(ReadComPort(&tempch, 1)==1)
	{
		x=tempch;
		return x;
	}
	else
		return EOF;
*/
}

int ferror(FILE *f)
{   /* Your implementation of ferror */
    return EOF;
}


void _sys_exit(int return_code)
{
    Exit();         /* for debugging */

label:  goto label; /* endless loop */
}


void _ttywrch(int ch)
{
    char tempch = ch;

	WriteComPort(&tempch, 1);
	
}

int getch()
{
	int x;
	char tempch;
	
	while(ReadComPort(&tempch, 1)==0);
	x=tempch;
	WriteComPort(&tempch, 1);
	return x;
}

int getKey()
{
	char tempch;
	if (ReadComPort(&tempch, 1)==0) return 0;
	else return 1;
}

__value_in_regs struct __initial_stackheap __user_initial_stackheap(
        unsigned R0, unsigned SP, unsigned R2, unsigned SL)
{
    struct __initial_stackheap config;
    
    config.heap_base = 0x335f8000;
    config.stack_base = SP;

    return config;
}

/*
To place heap_base directly above the ZI area, use e.g:
    extern unsigned int Image$$ZI$$Limit;
    config.heap_base = (unsigned int)&Image$$ZI$$Limit;
(or &Image$$region_name$$ZI$$Limit for scatterloaded images)

To specify the limits for the heap & stack, use e.g:
    config.heap_limit = SL;
    config.stack_limit = SL;
*/

