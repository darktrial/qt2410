//#include "../../inc/config.h"
#include "tftpput.h"
#include "utility.h"



extern char TftpLoadEnd;
extern char TftpPutMark;
extern char TftpPutBegin;
extern unsigned long tftp_download_len;
extern unsigned long tftp_download_addr;

static unsigned char *buf;
static int data_len;

int tftp_put_begin(void)
{	
	buf = (unsigned char *)tftp_download_addr;
	data_len = 0;
	TftpPutBegin = 1;
	
	return 0;
}

int tftp_put(unsigned char *data, int len)
{
	static int count = 0;		

	count += len;
	if (count > 32 * 1024) {
		TftpPutMark = 1;
		count = 0;
	}
	
	memcpy(buf + data_len, data, len);

/*	{
		int i;	
		if(len==0x200)
		{
			printf("\n%x:", data_len+0xf0);
			for(i=0xf0; i<0xf4; i++)
				printf("%x,",(buf+data_len)[i]);
		}
	}*/
			
	data_len += len;			
	
	return 0;
}

int update_bios(unsigned char *bios, int size)
{

	return 0;
}

int update_system_table(unsigned char *system_table, int size)
{
	return 0;
}

int update_partition_table(unsigned char *partition_table, int size)
{
	return 0;
}

int update_partition(int partition_num, unsigned char *partition_data, int size)
{
	return 0;
}

int update_firmware(unsigned char *firmware, int size)
{
	return 0;
}

int tftp_put_end(void)
{	
	TftpLoadEnd = 1;
	tftp_download_len = data_len;
	return 0;
}

