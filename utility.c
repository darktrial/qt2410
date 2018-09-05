#include "sys.h"

//Linux image could be packed to multi-file object
#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
typedef struct _ImageHeader
 {
	u_int	ih_magic;	/* Image Header Magic Number	*/
	u_int	ih_hcrc;	/* Image Header CRC Checksum	*/
	u_int	ih_time;	/* Image Creation Timestamp	*/
	u_int	ih_size;	/* Image Data Size		*/
	u_int	ih_load;	/* Data	 Load  Address		*/
	u_int	ih_ep;		/* Entry Point Address		*/
	u_int	ih_dcrc;	/* Image Data CRC Checksum	*/
	u_char		ih_os;		/* Operating System		*/
	u_char		ih_arch;	/* CPU architecture		*/
	u_char		ih_type;	/* Image Type			*/
	u_char		ih_comp;	/* Compression Type		*/
	u_char		ih_name[32];	/* Image Name		*/
} ImageHeader;    
ImageHeader *ih;

extern char CS8900IP[20];
extern u_char CS8900Amac[6];
extern systemInfo *globalSysInfo;
//delay for 1 milli second = 0.001 sec
void Delay(unsigned int ms)
{
	rTCFG0|=0xFF00;  //Prescaler1=255
	rTCFG1|=0x300; //Select MUX input for PWM Timer2:divider=16
	// I just compute this value by using my watch timing
	rTCNTB2=750; //50000000/16/(255+1)=81.92*10^-6 seconds
	rTCMPB2=0;   //81.92*750=0.06 but actually it runs a 1 milli seconds interval
	rTCON|=1<<13;//timer2 manual updata on
	rTCON&=~(1<<13);//timer2 manual updata off
	rTCON|=0x9<<12;//start timer2 with auto reload on and invert off
	while(ms!=0) {
		while(rTCNTO2>0);
		--ms;
	}
	rTCON&=~(0x9<<20);
} 

unsigned short ntohs(unsigned short s)
{
	return (s >> 8) | (s << 8);
}

unsigned long ntohl(unsigned long l)
{
	return  ((l >> 24) & 0x000000ff) |
		((l >>  8) & 0x0000ff00) |
		((l <<  8) & 0x00ff0000) |
		((l << 24) & 0xff000000);
}

unsigned short htons(unsigned short s)
{
	return (s >> 8) | (s << 8);
}

unsigned long htonl(unsigned long l)
{
	return ntohl(l);
}

u_int str_to_addr(const char *addr)
{
	u_int split[4];
	u_int ip;

	sscanf(addr, "%d.%d.%d.%d", &split[0], &split[1], &split[2], &split[3]);

	/* assuming sscanf worked */
	ip = (split[0] << 24) |(split[1] << 16) | (split[2] << 8) | (split[3]);

	return htonl(ip);
}

void addr_fprint(u_int x)
{
	u_char split[4];
	u_int ip;
	char temp[20];
	
	//memset(CS8900IP,0x0,20);
	ip = ntohl(x);
	split[0] = (ip & 0xff000000) >> 24;
	split[1] = (ip & 0x00ff0000) >> 16;
	split[2] = (ip & 0x0000ff00) >> 8;
	split[3] = (ip & 0x000000ff);
	sprintf(temp,"%d.%d.%d.%d", split[0], split[1], split[2], split[3]);
	strcpy(CS8900IP,temp,20);
}

char hextochar(unsigned char in) {
	if(in < 16)
		return (in<10)?in+48 : in+55;
}


unsigned char chartohex(unsigned char in) {
	in = toupper(in);
	if((in >= 'A') && (in <= 'F'))
		in = in - 'A' + 10;
	else if((in >= '0') && (in <= '9'))
		in = in - '0';
	return in;
}


unsigned char* strtohex(const char *in, int *len) {
	unsigned char *out;
	int out_size = 0;
	int i, j;

	out_size = strlen(in) / 2;
	if(out_size == 0)
		return NULL;

	out =(unsigned char *)malloc(sizeof(unsigned char) * out_size + 1);
	memset(out, 0, out_size + 1);

	for(i=0; i<out_size; i++) {
		j = 2 * i;
		out[i] = chartohex(in[j]);
		out[i] = out[i] << 4;
		out[i] = out[i] + chartohex(in[j+1]);
	}
	*len = out_size;

	return out;
}


char *hextostr(unsigned char *in, int in_size) {
	unsigned char low = 0x0F, c;
	char *out;
	int i, size;

	if(!in_size)
		return NULL;

	size = in_size * 2 + 1;
	out = (char*)malloc(sizeof(char) * size);
	memset(out, 0, size);

	for(i=0; i<in_size; i++) {
		c = in[i] >> 4;
 		*(out + 2*i) = hextochar(c);

		c = in[i] & low;
		*(out + 2*i + 1) = hextochar(c);
	}
	return out;
}

static __inline void CacheCleanInvalidateAll(void)
{
	__asm{
		mov	r1, #0		
		mov	r1, #7 << 5			  	/* 8 segments */
cache_clean_loop1:		
		orr	r3, r1, #63UL << 26	  	/* 64 entries */
cache_clean_loop2:	
		mcr	p15, 0, r3, c7, c14, 2	/* clean & invalidate D index */
		subs	r3, r3, #1 << 26
		bcs	cache_clean_loop2		/* entries 64 to 0 */
		subs	r1, r1, #1 << 5
		bcs	cache_clean_loop1		/* segments 7 to 0 */
		mcr	p15, 0, r1, c7, c5, 0	/* invalidate I cache */
		mcr	p15, 0, r1, c7, c10, 4	/* drain WB */
	}
}

static __inline void TLBInvalidateAll(void)
{
	__asm{
		mov	r0, #0
		mcr	p15, 0, r0, c7, c10, 4	/* drain WB */
		mcr	p15, 0, r0, c8, c7, 0	/* invalidate I & D TLBs */
	}
}


#define LINUX_PAGE_SHIFT	12
#define LINUX_PAGE_SIZE		(1<<LINUX_PAGE_SHIFT)
#define COMMAND_LINE_SIZE 	1024

struct param_struct {
    union {
	struct {
	    unsigned long page_size;			/*  0 */
	    unsigned long nr_pages;				/*  4 */
	    unsigned long ramdisk_size;			/*  8 */
	    unsigned long flags;				/* 12 */
#define FLAG_READONLY	1
#define FLAG_RDLOAD		4
#define FLAG_RDPROMPT	8
	    unsigned long rootdev;				/* 16 */
	    unsigned long video_num_cols;		/* 20 */
	    unsigned long video_num_rows;		/* 24 */
	    unsigned long video_x;				/* 28 */
	    unsigned long video_y;				/* 32 */
	    unsigned long memc_control_reg;		/* 36 */
	    unsigned char sounddefault;			/* 40 */
	    unsigned char adfsdrives;			/* 41 */
	    unsigned char bytes_per_char_h;		/* 42 */
	    unsigned char bytes_per_char_v;		/* 43 */
	    unsigned long pages_in_bank[4];		/* 44 */
	    unsigned long pages_in_vram;		/* 60 */
	    unsigned long initrd_start;			/* 64 */
	    unsigned long initrd_size;			/* 68 */
	    unsigned long rd_start;				/* 72 */
	    unsigned long system_rev;			/* 76 */
	    unsigned long system_serial_low;	/* 80 */
	    unsigned long system_serial_high;	/* 84 */
	    unsigned long mem_fclk_21285;       /* 88 */
	} s;
	char unused[256];
    } u1;
    union {
	char paths[8][128];
	struct {
	    unsigned long magic;
	    char n[1024 - sizeof(unsigned long)];
	} s;
    } u2;
    char commandline[COMMAND_LINE_SIZE];
};

void initLinuxParam()
{
	int i;
	
	struct param_struct *params = (struct param_struct *)0x30000100;
	char *linux_params = globalSysInfo->BootParam;
	
	for(i=0; i<(sizeof(struct param_struct)>>2); i++)
		((u_int *)params)[i] = 0;
	params->u1.s.page_size = LINUX_PAGE_SIZE;
	params->u1.s.nr_pages = (0x04000000 >> LINUX_PAGE_SHIFT);
	for(i=0; linux_params[i]; i++)
		params->commandline[i] = linux_params[i];
		
	printf("Boot Parameter:%s\n\r",linux_params);
	
	
}

bool CheckImageHeader()
{
	
	
	ih=(ImageHeader *)globalSysInfo->ApplicationLoadAddress;
	if (ntohl(ih->ih_magic) != IH_MAGIC) return true;
	else return false;
}

void JumpToProgram()
{
	void (*fp)(u_int,u_int,u_int);
	
	initLinuxParam();
	if (CheckImageHeader())
		fp = (void (*)(u_int,u_int,u_int))globalSysInfo->ApplicationLoadAddress;
	else fp = (void (*)(u_int,u_int,u_int))ntohl(ih->ih_ep);
	CacheCleanInvalidateAll();
	TLBInvalidateAll();
	__asm
	{
		mov	ip, #0
		mcr	p15, 0, ip, c13, c0, 0	/* zero PID */
		mcr	p15, 0, ip, c7, c7, 0	/* invalidate I,D caches */
		mcr	p15, 0, ip, c7, c10, 4	/* drain write buffer */
		mcr	p15, 0, ip, c8, c7, 0	/* invalidate I,D TLBs */
		mrc	p15, 0, ip, c1, c0, 0	/* get control register */
		bic	ip, ip, #0x0001			/* disable MMU */
		mcr	p15, 0, ip, c1, c0, 0	/* write control register */
	}
	Delay(100);
	(*fp)(0,193,0x30000100);//193 is the linux machine type of 2410
	return;
}

void WriteNANDFlash(u_int address,u_int size)
{
	u_int StartAddress,EndAddress,StartBlock,EndBlock,i;
	char buffer[512];
	int ret;	
	
	//First we erase the block we want to override
	StartAddress=linearAddressConvert(address);
	StartBlock=blockAddressConvert(address);
	EndAddress=linearAddressConvert(address+size);
	EndBlock=blockAddressConvert(address+size);
	
	printf("Erasing blocks...\n\r");
	for (i=StartBlock;i<=EndBlock;i++)
	{
		ret=EraseBlock(i);
		if (ret==SUCCESS)
			printf(".");
		else printf("*");
	}	
	printf("Erase finished\n\r");
	printf("Start writing image %d size at NAND %d\n\r",size,address);
	for (i=globalSysInfo->ApplicationLoadAddress;i<(globalSysInfo->ApplicationLoadAddress+size);i+=512)
	{
		memset(buffer,0,512);
		StartAddress=linearAddressConvert(address);
		memcpy(buffer,i,512);
		ret=WritePage(StartAddress,buffer);
		if (ret==SUCCESS) printf(".");
		else printf("*");
		address+=512;
	}
	printf("\n\rUpgrading Success!!\n\r");
	
	
}

bool NANDFlashUpgrade(int area,int size)
{
	if (area==LKIM)
	{
		WriteNANDFlash(globalSysInfo->LinuxKernelImageStart,size);
		globalSysInfo->LinuxKernelImageSize=size;
		return true;
	}
	else if (area==LKFS)
	{
		WriteNANDFlash(globalSysInfo->LinuxFileSystemStart,size);
		globalSysInfo->LinuxFileSystemSize=size;
		return true;
	
	}
	else if (area==OKIM)
	{
		WriteNANDFlash(globalSysInfo->OtherKernelImageStart,size);
		globalSysInfo->OtherKernelImageSize=size;
		return true;
	}
	else if (area==BLIM)
	{
		WriteNANDFlash(globalSysInfo->bootImageStart,size);
		globalSysInfo->bootImageSize=size;
		return true;
	}
	else 
	{
		printf("Wrong Area to upgrade\n\r");
		return false;
	}
}
