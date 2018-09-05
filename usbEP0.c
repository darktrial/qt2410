#include "sys.h"
#include "usb.h"

extern u_int ep0State;
extern bool isUSBSet;
extern struct USB_SETUP_DATA descSetup;
extern struct USB_DEVICE_DESCRIPTOR descDev;
extern struct USB_CONFIGURATION_DESCRIPTOR descConf;
extern struct USB_INTERFACE_DESCRIPTOR descIf;
extern struct USB_ENDPOINT_DESCRIPTOR descEndpt0;
extern struct USB_ENDPOINT_DESCRIPTOR descEndpt1;


#define CLR_EP0_OUT_PKT_RDY() 		rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| EP0_SERVICED_OUT_PKT_RDY )	 
#define CLR_EP0_OUTPKTRDY_DATAEND() rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| (EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END) )	 
					
#define SET_EP0_IN_PKT_RDY() 		rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| (EP0_IN_PKT_READY) )	 
#define SET_EP0_INPKTRDY_DATAEND() 	rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| (EP0_IN_PKT_READY|EP0_DATA_END) )	 
					
#define CLR_EP0_SETUP_END() 		rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)| (EP0_SERVICED_SETUP_END) )

#define CLR_EP0_SENT_STALL() 		rEP0_CSR=( ep0_csr & (~EP0_WR_BITS)& (~EP0_SENT_STALL) )

#define FLUSH_EP0_FIFO() 			{register i;while(rOUT_FIFO_CNT1_REG)i=rEP0_FIFO;}


static const u_char descStr0[]=
{
	4,STRING_TYPE,LANGID_US_L,LANGID_US_H,  //codes representing languages
};

static const u_char descStr1[]=
{  //Manufacturer  
        (0x14+2),STRING_TYPE, 
        'S',0x0,'y',0x0,'s',0x0,'t',0x0,'e',0x0,'m',0x0,' ',0x0,'M',0x0,
        'C',0x0,'U',0x0,
};
    
static const u_char descStr2[]=
{  //Product  
        (0x2a+2),STRING_TYPE, 
        'S',0x0,'E',0x0,'C',0x0,' ',0x0,'S',0x0,'3',0x0,'C',0x0,'2',0x0,
        '4',0x0,'1',0x0,'0',0x0,'X',0x0,' ',0x0,'T',0x0,'e',0x0,'s',0x0,
        't',0x0,' ',0x0,'B',0x0,'/',0x0,'D',0x0
};


void Ep0Handler(void)
{
    static int ep0SubState;
//    int i;
    u_char ep0_csr;

    rINDEX_REG=0;
    ep0_csr=rEP0_CSR;
    
//    DbgOut("<0:%x]",ep0_csr);

    //DATAEND interrupt(ep0_csr==0x0) will be ignored 
    //because ep0State==EP0_STATE_INIT when the DATAEND interrupt is issued.
    
    if(ep0_csr & EP0_SETUP_END)
    {   
    	 // Host may end GET_DESCRIPTOR operation without completing the IN data stage.
    	 // If host does that, SETUP_END bit will be set.
    	 // OUT_PKT_RDY has to be also cleared because status stage sets OUT_PKT_RDY to 1.
//   	DbgOut("[SETUPEND]");
		CLR_EP0_SETUP_END();
		if(ep0_csr & EP0_OUT_PKT_READY) 
		{
		    FLUSH_EP0_FIFO(); //(???)
	    	//I think this isn't needed because EP0 flush is done automatically.   
	    	CLR_EP0_OUT_PKT_RDY();
		}
	
		ep0State=EP0_STATE_INIT;
		return;
    }	

    //I think that EP0_SENT_STALL will not be set to 1.
    if(ep0_csr & EP0_SENT_STALL)
    {   
//		DbgOut("[STALL]");
	   	CLR_EP0_SENT_STALL();
		if(ep0_csr & EP0_OUT_PKT_READY) 
		{
		    CLR_EP0_OUT_PKT_RDY();
		}
	
		ep0State=EP0_STATE_INIT;
		return;
    }

    if((ep0_csr & EP0_OUT_PKT_READY) && (ep0State==EP0_STATE_INIT))
    {	
		RdPktEp0((u_char *)&descSetup,EP0_PKT_SIZE);
    
		switch(descSetup.bRequest)
    	{
    	case GET_DESCRIPTOR:
            switch(descSetup.bValueH)        
            {
            case DEVICE_TYPE:
// 	    		DbgOut("[GDD]");
	 	    	CLR_EP0_OUT_PKT_RDY();
		    	ep0State=EP0_STATE_GD_DEV_0;	        
	    		break;	
	    		
		    case CONFIGURATION_TYPE:
// 			   	DbgOut("[GDC]");
 	    		CLR_EP0_OUT_PKT_RDY();
	 	    	if((descSetup.bLengthL+(descSetup.bLengthH<<8))>0x9)
 	    	  //bLengthH should be used for bLength=0x209 at WIN2K.    	
		    	    ep0State=EP0_STATE_GD_CFG_0; //for WIN98,WIN2K
				else	    	    
	  			    ep0State=EP0_STATE_GD_CFG_ONLY_0; //for WIN2K
			    break;
			    
	   	    case STRING_TYPE:
// 		    	DbgOut("[GDS]");
 	    		CLR_EP0_OUT_PKT_RDY();
	    		switch(descSetup.bValueL)
	    		{
	    	    case 0:
	    	    	ep0State=EP0_STATE_GD_STR_I0;
	    	    	break;
	    	    case 1:
       	    		ep0State=EP0_STATE_GD_STR_I1;
	    	    	break;
	    	    case 2:	
	    	    	ep0State=EP0_STATE_GD_STR_I2;
	    	    	break;
	    	    default:
//	    			DbgOut("[UE:STRI?]");
	    			break;
	    		}
		    	ep0SubState=0;
		    	break;
		    	
	    	case INTERFACE_TYPE:
// 	    		DbgOut("[GDI]");
	 	    	CLR_EP0_OUT_PKT_RDY();
		    	ep0State=EP0_STATE_GD_IF_ONLY_0; //for WIN98
	    		break;
	    		
		    case ENDPOINT_TYPE:	    	
// 		    	DbgOut("[GDE]");
 	    		CLR_EP0_OUT_PKT_RDY();
 	    		switch(descSetup.bValueL&0xf)
		    	{
		    	case 0:
	    		    ep0State=EP0_STATE_GD_EP0_ONLY_0;
	    		    break;
		    	case 1:
					ep0State=EP0_STATE_GD_EP1_ONLY_0;
	    		    break;
	    		default:
//	    	    	DbgOut("[UE:GDE?]");
		    	    break;
		    	}
	    		break;
		    default:
//		    	DbgOut("[UE:GD?]");
	    		break;
		    }	
   		    break;
   		    
    	case SET_ADDRESS:
//			DbgOut("[SA:%d]",descSetup.bValueL);
            rFUNC_ADDR_REG=descSetup.bValueL | 0x80;
		    CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
            ep0State=EP0_STATE_INIT;
            break;    	
            
		case SET_CONFIGURATION:
//			DbgOut("[SC]");
            CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
            ep0State=EP0_STATE_INIT;
            isUSBSet=true; 
    	    break;
    	    
  		default:
//			DbgOut("[UE:SETUP=%x]",descSetup.bRequest);
    	    CLR_EP0_OUTPKTRDY_DATAEND(); //Because of no data control transfers.
		    ep0State=EP0_STATE_INIT;
	    	break;    	
        }
    }
    
    switch(ep0State)
    {	
	case EP0_STATE_INIT:
	    break; 

	//=== GET_DESCRIPTOR:DEVICE ===
   	case EP0_STATE_GD_DEV_0:
//		DbgOut("[GDD0]");
		WrPktEp0((u_char *)&descDev+0,8); //EP0_PKT_SIZE
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_DEV_1;
		break;
            
	case EP0_STATE_GD_DEV_1:
//		DbgOut("[GDD1]");
		WrPktEp0((u_char *)&descDev+0x8,8); 
		SET_EP0_IN_PKT_RDY();            
		ep0State=EP0_STATE_GD_DEV_2;
		break;

	case EP0_STATE_GD_DEV_2:
//		DbgOut("[GDD2]");
		WrPktEp0((u_char *)&descDev+0x10,2);   //8+8+2=0x12
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;
		break;   

        //=== GET_DESCRIPTOR:CONFIGURATION+INTERFACE+ENDPOINT0+ENDPOINT1 ===
        //Windows98 gets these 4 descriptors all together by issuing only a request.
        //Windows2000 gets each descriptor seperately.
	case EP0_STATE_GD_CFG_0:
//		DbgOut("[GDC0]");
		WrPktEp0((u_char *)&descConf+0,8); //EP0_PKT_SIZE
		SET_EP0_IN_PKT_RDY();
		ep0State=EP0_STATE_GD_CFG_1;
		break;
    
	case EP0_STATE_GD_CFG_1:
//		DbgOut("[GDC1]");
		WrPktEp0((u_char *)&descConf+8,1); 
		WrPktEp0((u_char *)&descIf+0,7); 
		SET_EP0_IN_PKT_RDY();
        ep0State=EP0_STATE_GD_CFG_2;
        break;

  	case EP0_STATE_GD_CFG_2:
//		DbgOut("[GDC2]");
        WrPktEp0((u_char *)&descIf+7,2); 
        WrPktEp0((u_char *)&descEndpt0+0,6); 
        SET_EP0_IN_PKT_RDY();
        ep0State=EP0_STATE_GD_CFG_3;
        break;

   	case EP0_STATE_GD_CFG_3:
//		DbgOut("[GDC3]");
        WrPktEp0((u_char *)&descEndpt0+6,1); 
        WrPktEp0((u_char *)&descEndpt1+0,7); 
        SET_EP0_IN_PKT_RDY();
        ep0State=EP0_STATE_GD_CFG_4;            
        break;

   	case EP0_STATE_GD_CFG_4:
//		DbgOut("[GDC4]");
		//zero length data packit 
		SET_EP0_INPKTRDY_DATAEND();
		ep0State=EP0_STATE_INIT;            
        break;

    //=== GET_DESCRIPTOR:CONFIGURATION ONLY===
   	case EP0_STATE_GD_CFG_ONLY_0:
//		DbgOut("[GDCO0]");
        WrPktEp0((u_char *)&descConf+0,8); //EP0_PKT_SIZE
        SET_EP0_IN_PKT_RDY();
        ep0State=EP0_STATE_GD_CFG_ONLY_1;
        break;
    
	case EP0_STATE_GD_CFG_ONLY_1:
//		DbgOut("[GDCO1]");
        WrPktEp0((u_char *)&descConf+8,1); 
        SET_EP0_INPKTRDY_DATAEND();
        ep0State=EP0_STATE_INIT;            
        break;

    //=== GET_DESCRIPTOR:INTERFACE ONLY===
   	case EP0_STATE_GD_IF_ONLY_0:
//		DbgOut("[GDI0]");
        WrPktEp0((u_char *)&descIf+0,8); 
        SET_EP0_IN_PKT_RDY();
        ep0State=EP0_STATE_GD_IF_ONLY_1;
        break;
        
   	case EP0_STATE_GD_IF_ONLY_1:
//		DbgOut("[GDI1]");
        WrPktEp0((u_char *)&descIf+8,1); 
        SET_EP0_INPKTRDY_DATAEND();
        ep0State=EP0_STATE_INIT;            
        break;

    //=== GET_DESCRIPTOR:ENDPOINT 0 ONLY===
   	case EP0_STATE_GD_EP0_ONLY_0:
//		DbgOut("[GDE00]");
        WrPktEp0((u_char *)&descEndpt0+0,7); 
        SET_EP0_INPKTRDY_DATAEND();
        ep0State=EP0_STATE_INIT;            
        break;
            
    //=== GET_DESCRIPTOR:ENDPOINT 1 ONLY===
   	case EP0_STATE_GD_EP1_ONLY_0:
//		DbgOut("[GDE10]");
        WrPktEp0((u_char *)&descEndpt1+0,7); 
        SET_EP0_INPKTRDY_DATAEND();
        ep0State=EP0_STATE_INIT;            
        break;
 
    //=== GET_DESCRIPTOR:STRING ===
   	case EP0_STATE_GD_STR_I0:
//		DbgOut("[GDS0_0]");
	    WrPktEp0((u_char *)descStr0, 4 );  
	    SET_EP0_INPKTRDY_DATAEND();
	    ep0State=EP0_STATE_INIT;     
	    ep0SubState=0;
	    break;

	case EP0_STATE_GD_STR_I1:
//		DbgOut("[GDS1_%d]",ep0SubState);
		if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr1) )
		{
			WrPktEp0((u_char *)descStr1+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_STR_I1;
            ep0SubState++;
		}
	    else
	    {
	    	WrPktEp0((u_char *)descStr1+(ep0SubState*EP0_PKT_SIZE),
	    		 sizeof(descStr1)-(ep0SubState*EP0_PKT_SIZE)); 
			SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT;     
			ep0SubState=0;
	    }
	    break;

	case EP0_STATE_GD_STR_I2:
//		DbgOut("[GDS2_%d]",ep0SubState);
        if( (ep0SubState*EP0_PKT_SIZE+EP0_PKT_SIZE)<sizeof(descStr2) )
        {
        	WrPktEp0((u_char *)descStr2+(ep0SubState*EP0_PKT_SIZE),EP0_PKT_SIZE); 
            SET_EP0_IN_PKT_RDY();
            ep0State=EP0_STATE_GD_STR_I2;
            ep0SubState++;
		}
	    else
	    {
//			DbgOut("[E]");
	    	WrPktEp0((u_char *)descStr2+(ep0SubState*EP0_PKT_SIZE),
	    		 sizeof(descStr2)-(ep0SubState*EP0_PKT_SIZE)); 
			SET_EP0_INPKTRDY_DATAEND();
			ep0State=EP0_STATE_INIT;     
			ep0SubState=0;
	    }
		break;
		
	default:
//		DbgOut("UE:G?D");
    	break;
    }
}