#include "sys.h"
#include "usb.h"
// ===================================================================
// All following commands will operate in case 
// - in_csr1 is valid.
// ===================================================================

#define SET_EP1_IN_PKT_READY()  rIN_CSR1_REG= ( in_csr1 &(~ EPI_WR_BITS)| EPI_IN_PKT_READY )	 
#define SET_EP1_SEND_STALL()	rIN_CSR1_REG= ( in_csr1 & (~EPI_WR_BITS)| EPI_SEND_STALL) )
#define CLR_EP1_SENT_STALL()	rIN_CSR1_REG= ( in_csr1 & (~EPI_WR_BITS)&(~EPI_SENT_STALL) )
#define FLUSH_EP1_FIFO() 		rIN_CSR1_REG= ( in_csr1 & (~EPI_WR_BITS)| EPI_FIFO_FLUSH) )


// ***************************
// *** VERY IMPORTANT NOTE ***
// ***************************
// Prepare the code for the packit size constraint!!!

// EP1 = IN end point. 

extern u_int UsbState;
extern u_int UsbInLength;
extern u_char *UsbTxAddr;

u_char ep1Buf[EP1_PKT_SIZE];
int transferIndex=0;


void PrepareEp1Fifo(void) 
{
    int i;
    u_char in_csr1;   
    
    rINDEX_REG=1;
    in_csr1=rIN_CSR1_REG;
    
	for(i=0;i<EP1_PKT_SIZE;i++)ep1Buf[i]=(u_char)(transferIndex+i);
//	WrPktEp1(ep1Buf,EP1_PKT_SIZE);
	WrPktEp1(UsbTxAddr, EP1_PKT_SIZE);
	UsbTxAddr   += EP1_PKT_SIZE;
   	UsbInLength -= ((UsbInLength>EP1_PKT_SIZE)?EP1_PKT_SIZE:UsbInLength);
   	if(!UsbInLength)
   		UsbState = 0;
    SET_EP1_IN_PKT_READY(); 
}


void Ep1Handler(void)
{
    u_char in_csr1;
    
    rINDEX_REG = 1;    
    in_csr1 = rIN_CSR1_REG;   

    //I think that EPI_SENT_STALL will not be set to 1.
    if(in_csr1 & EPI_SENT_STALL)
    {   
//		DbgOut("[STALL]");
	   	CLR_EP1_SENT_STALL();
   		return;
    }	

    //IN_PKT_READY is cleared
    
    //The data transfered was ep1Buf[] which was already configured  
    
	transferIndex++;		
			
	if(UsbState==0x01234567)	
		PrepareEp1Fifo();
    		    	
    
    	//IN_PKT_READY is set   
    	//This packit will be used for next IN packit.	

    return;
}   