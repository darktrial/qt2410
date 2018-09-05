/****************************************************************
 NAME: usb.h
 DESC: definitions(USB data structure) for USB setup operation.
       Because It's h/w independent file, it may be used without any change in future.
       Reuse the source of S3C2400X u24xmon 
 HISTORY:
 Apr.07.2000:purnnamu: first release. 
 ****************************************************************/
//option definitions
#define USBDMA			true
#define USBDMA_DEMAND 	false	//the downloadFileSize should be (64*n)
#define BULK_PKT_SIZE	32

//************************
//       Endpoint 0      
//************************

// Standard bmRequestTyje (Direction) 
#define HOST_TO_DEVICE              (0x00)
#define DEVICE_TO_HOST              (0x80)    

// Standard bmRequestType (Type) 
#define STANDARD_TYPE               (0x00)
#define CLASS_TYPE                  (0x20)
#define VENDOR_TYPE                 (0x40)
#define RESERVED_TYPE               (0x60)

// Standard bmRequestType (Recipient) 
#define DEVICE_RECIPIENT            (0)
#define INTERFACE_RECIPIENT         (1)
#define ENDPOINT_RECIPIENT          (2)
#define OTHER_RECIPIENT             (3)

// Feature Selectors 
#define DEVICE_REMOTE_WAKEUP        (1)
#define EP_STALL                    (0)

// Standard Request Codes 
#define GET_STATUS                  (0)
#define CLEAR_FEATURE               (1)
#define SET_FEATURE                 (3)
#define SET_ADDRESS                 (5)
#define GET_DESCRIPTOR              (6)
#define SET_DESCRIPTOR              (7)
#define GET_CONFIGURATION           (8)
#define SET_CONFIGURATION           (9)
#define GET_INTERFACE               (10)
#define SET_INTERFACE               (11)
#define SYNCH_FRAME                 (12)

// Class-specific Request Codes 
#define GET_DEVICE_ID               (0)
#define GET_PORT_STATUS             (1)
#define SOFT_RESET                  (2)

// Descriptor Types
#define DEVICE_TYPE                 (1)
#define CONFIGURATION_TYPE          (2)
#define STRING_TYPE                 (3)
#define INTERFACE_TYPE              (4)
#define ENDPOINT_TYPE               (5)

//configuration descriptor: bmAttributes 
#define CONF_ATTR_DEFAULT	    	(0x80) //Spec 1.0 it was BUSPOWERED bit.
#define CONF_ATTR_REMOTE_WAKEUP     (0x20)
#define CONF_ATTR_SELFPOWERED       (0x40)

//endpoint descriptor
#define EP_ADDR_IN		    (0x80)	
#define EP_ADDR_OUT		    (0x00)

#define EP_ATTR_CONTROL		    (0x0)	
#define EP_ATTR_ISOCHRONOUS	    (0x1)
#define EP_ATTR_BULK		    (0x2)
#define EP_ATTR_INTERRUPT	    (0x3)	


//string descriptor
#define LANGID_US_L 		    (0x09)  
#define LANGID_US_H 		    (0x04)
struct USB_SETUP_DATA
{
    u_char bmRequestType;    
    u_char bRequest;         
    u_char bValueL;          
    u_char bValueH;          
    u_char bIndexL;          
    u_char bIndexH;          
    u_char bLengthL;         
    u_char bLengthH;         
};


struct USB_DEVICE_DESCRIPTOR
{
    u_char bLength;    
    u_char bDescriptorType;         
    u_char bcdUSBL;
    u_char bcdUSBH;
    u_char bDeviceClass;          
    u_char bDeviceSubClass;          
    u_char bDeviceProtocol;          
    u_char bMaxPacketSize0;         
    u_char idVendorL;
    u_char idVendorH;
    u_char idProductL;
    u_char idProductH;
    u_char bcdDeviceL;
    u_char bcdDeviceH;
    u_char iManufacturer;
    u_char iProduct;
    u_char iSerialNumber;
    u_char bNumConfigurations;
};


struct USB_CONFIGURATION_DESCRIPTOR
{
    u_char bLength;    
    u_char bDescriptorType;         
    u_char wTotalLengthL;
    u_char wTotalLengthH;
    u_char bNumInterfaces;
    u_char bConfigurationValue;
    u_char iConfiguration;
    u_char bmAttributes;
    u_char maxPower;          
};
    

struct USB_INTERFACE_DESCRIPTOR
{
    u_char bLength;    
    u_char bDescriptorType;         
    u_char bInterfaceNumber;
    u_char bAlternateSetting;
    u_char bNumEndpoints;
    u_char bInterfaceClass;
    u_char bInterfaceSubClass;
    u_char bInterfaceProtocol;
    u_char iInterface;
};


struct USB_ENDPOINT_DESCRIPTOR
{
    u_char bLength;    
    u_char bDescriptorType;         
    u_char bEndpointAddress;
    u_char bmAttributes;
    u_char wMaxPacketSizeL;
    u_char wMaxPacketSizeH;
    u_char bInterval;
};



/* Power Management Register */
#define DISABLE_SUSPEND          0x00   
#define ENABLE_SUSPEND           0x01
#define SUSPEND_MODE		 	 0x02
#define MCU_RESUME               0x04
#define ISO_UPDATE		        (1<<7)

/* MAXP Register */
#define FIFO_SIZE_0              0x00  /* 0x00 * 8 = 0  */
#define FIFO_SIZE_8              0x01  /* 0x01 * 8 = 8  */
#define FIFO_SIZE_16             0x02  /* 0x02 * 8 = 16 */
#define FIFO_SIZE_32             0x04  /* 0x04 * 8 = 32 */
#define FIFO_SIZE_64             0x08  /* 0x08 * 8 = 64 */

/* ENDPOINT0 CSR (Control Status Register) : Mapped to IN CSR1 */
#define EP0_OUT_PKT_READY        0x01  /* USB sets, MCU clears by setting SERVICED_OUT_PKT_RDY */
#define EP0_IN_PKT_READY         0x02  /* MCU sets, USB clears after sending FIFO */
#define EP0_SENT_STALL           0x04  /* USB sets */       
#define EP0_DATA_END             0x08  /* MCU sets */
#define EP0_SETUP_END            0x10  /* USB sets, MCU clears by setting SERVICED_SETUP_END */
#define EP0_SEND_STALL           0x20  /* MCU sets */
#define EP0_SERVICED_OUT_PKT_RDY 0x40  /* MCU writes 1 to clear OUT_PKT_READY */
#define EP0_SERVICED_SETUP_END   0x80  /* MCU writes 1 to clear SETUP_END        */

#define EP0_WR_BITS              0xc0  

//EP_INT_REG / EP_INT_EN_REG
#define EP0_INT                	 0x01  // Endpoint 0, Control   
#define EP1_INT                  0x02  // Endpoint 1, (Bulk-In) 
#define EP2_INT                  0x04  // Endpoint 2 
#define EP3_INT			 		 0x08  // Endpoint 3, (Bulk-Out)   
#define EP4_INT			 		 0x10  // Endpoint 4

//USB_INT_REG / USB_INT_EN_REG
#define SUSPEND_INT            	 0x01  
#define RESUME_INT               0x02  
#define RESET_INT                0x04  

//IN_CSR1
#define EPI_IN_PKT_READY         0x01  
#define EPI_UNDER_RUN		 	 0x04
#define EPI_FIFO_FLUSH		 	 0x08
#define EPI_SEND_STALL           0x10  
#define EPI_SENT_STALL           0x20  
#define EPI_CDT			 		 0x40	
#define EPI_WR_BITS              (EPI_FIFO_FLUSH|EPI_IN_PKT_READY|EPI_CDT) 
					//(EPI_FIFO_FLUSH) is preferred  (???)
//IN_CSR2
#define EPI_IN_DMA_INT_MASK	(1<<4)
#define EPI_MODE_IN			(1<<5)
#define EPI_MODE_OUT		(0<<5)
#define EPI_ISO				(1<<6)
#define EPI_BULK			(0<<6)
#define EPI_AUTO_SET		(1<<7)

//OUT_CSR1
#define EPO_OUT_PKT_READY        0x01  
#define EPO_OVER_RUN		 	 0x04  
#define EPO_DATA_ERROR		 	 0x08  
#define EPO_FIFO_FLUSH		 	 0x10
#define EPO_SEND_STALL           0x20  
#define EPO_SENT_STALL           0x40
#define EPO_CDT			 		 0x80	
#define EPO_WR_BITS              (EPO_FIFO_FLUSH|EPO_SEND_STALL|EPO_CDT)
					//(EPO_FIFO_FLUSH) is preferred (???)

//OUT_CSR2
#define EPO_OUT_DMA_INT_MASK	(1<<5)
#define EPO_ISO		 			(1<<6)
#define EPO_BULK	 			(0<<6)
#define EPO_AUTO_CLR			(1<<7)

//USB DMA control register
#define UDMA_IN_RUN_OB		(1<<7)
#define UDMA_IGNORE_TTC		(1<<7)
#define UDMA_DEMAND_MODE	(1<<3)
#define UDMA_OUT_RUN_OB		(1<<2)
#define UDMA_OUT_DMA_RUN	(1<<2)
#define UDMA_IN_DMA_RUN		(1<<1)
#define UDMA_DMA_MODE_EN	(1<<0)

#define rEP1_DMA_TTC	(rEP1_DMA_TTC_L+(rEP1_DMA_TTC_M<<8)+(rEP1_DMA_TTC_H<<16))
#define rEP2_DMA_TTC	(rEP2_DMA_TTC_L+(rEP2_DMA_TTC_M<<8)+(rEP2_DMA_TTC_H<<16))
#define rEP3_DMA_TTC	(rEP3_DMA_TTC_L+(rEP3_DMA_TTC_M<<8)+(rEP3_DMA_TTC_H<<16))
#define rEP4_DMA_TTC	(rEP4_DMA_TTC_L+(rEP4_DMA_TTC_M<<8)+(rEP4_DMA_TTC_H<<16))

#define ADDR_EP0_FIFO 		(0x520001c0) //Endpoint 0 FIFO
#define ADDR_EP1_FIFO		(0x520001c4) //Endpoint 1 FIFO
#define ADDR_EP2_FIFO		(0x520001c8) //Endpoint 2 FIFO
#define ADDR_EP3_FIFO		(0x520001cc) //Endpoint 3 FIFO
#define ADDR_EP4_FIFO		(0x520001d0) //Endpoint 4 FIFO

//If you chane the packet size, the source code should be changed!!!
#define EP0_PKT_SIZE             8	
#define EP1_PKT_SIZE             BULK_PKT_SIZE
#define EP3_PKT_SIZE             BULK_PKT_SIZE

#define EP0_STATE_INIT 			(0)  

//NOTE: The ep0State value in a same group should be added by 1.
#define EP0_STATE_GD_DEV_0	 	(10)  //10-10=0 
#define EP0_STATE_GD_DEV_1 		(11)  //11-10=1
#define EP0_STATE_GD_DEV_2 		(12)  //12-10=2

#define EP0_STATE_GD_CFG_0	 	(20)
#define EP0_STATE_GD_CFG_1 		(21)
#define EP0_STATE_GD_CFG_2 		(22)
#define EP0_STATE_GD_CFG_3 		(23)
#define EP0_STATE_GD_CFG_4 		(24)

#define EP0_STATE_GD_CFG_ONLY_0		(40)
#define EP0_STATE_GD_CFG_ONLY_1		(41)
#define EP0_STATE_GD_IF_ONLY_0 		(42)
#define EP0_STATE_GD_IF_ONLY_1 		(43)
#define EP0_STATE_GD_EP0_ONLY_0		(44)
#define EP0_STATE_GD_EP1_ONLY_0		(45)


#define EP0_STATE_GD_STR_I0	 	(30)  
#define EP0_STATE_GD_STR_I1	 	(31)  
#define EP0_STATE_GD_STR_I2	 	(32)  

#define PWR_REG_DEFAULT_VALUE (DISABLE_SUSPEND)

#define CLR_EP3_OUT_PKT_READY() rOUT_CSR1_REG= ( out_csr3 &(~ EPO_WR_BITS)&(~EPO_OUT_PKT_READY) ) 
#define SET_EP3_SEND_STALL()	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)| EPO_SEND_STALL) )
#define CLR_EP3_SENT_STALL()	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)&(~EPO_SENT_STALL) )
#define FLUSH_EP3_FIFO() 		rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)|EPO_FIFO_FLUSH) )
