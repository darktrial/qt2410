//#include "../../inc/config.h"
#include "skbuff.h"
#include "eth.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"
#include "utility.h"
#include "sys.h"
//#include "../../inc/board.h"

extern systemInfo *globalSysInfo;

bool startTFTP=false;
char TftpLoadEnd;
char TftpPutBegin;
char TftpPutMark;

//#include "params.h"

#define	LOCAL_IP_ADDR	((192UL<<24)|(168<<16)|(15<<8)|100)

unsigned long tftp_download_len;
unsigned long tftp_download_addr;



int StartTFTPServer()
{
	unsigned char eth_addr[ETH_ALEN];	
	unsigned char *s;
	u_int give_ip;
	
	startTFTP=true;
	//give_ip = LOCAL_IP_ADDR;
	give_ip=ntohl(globalSysInfo->IPAddr);
	s = (unsigned char *)&give_ip;
	
	printf("Mini TFTP Server 1.0 (IP : %d.%d.%d.%d PORT: %d)\n\r", s[3], s[2], s[1], s[0], TFTP);		
	printf("Type tftp -i %d.%d.%d.%d put filename at the host PC\n\r", s[3], s[2], s[1], s[0]);

	//eth_init();//We do initial at boot stage
	//if(eth_lnk_stat())
	//	return 0;
		
	eth_get_addr(eth_addr);		
//	arp_init();
	ip_init(give_ip);
	udp_init();
		
	arp_add_entry(eth_addr, give_ip);	

	TftpLoadEnd  = 0;
	TftpPutMark  = 0;
	TftpPutBegin = 0;
	
	tftp_download_addr =globalSysInfo->ApplicationLoadAddress;
	tftp_download_len = 0;
	
	while (!TftpLoadEnd) 
	{		
		if(TftpPutBegin) 
		{
			printf("Starting the TFTP download...\n\r");
			TftpPutBegin = 0;
		}
		if(TftpPutMark) 
		{
			printf(".");
			TftpPutMark = 0;
		}
	}

	if(TftpLoadEnd) {
		printf("\n\rdownload 0x%x bytes to 0x%08x\n\r", tftp_download_len, tftp_download_addr);
//		printf("\nPress any key to continue...\n");
//		getch();
		startTFTP=false;
		return tftp_download_len;
	}
	startTFTP=false;
	return 0;
}




