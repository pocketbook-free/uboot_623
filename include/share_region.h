/* The size of share region is 4KB */

#define CRC32_RESIDUE	0xdebb20e3UL /* ANSI X3.66 residue*/

#define	PAGE_SIZE	0x1000

#define	SHARE_REGION_SIZE	PAGE_SIZE
#define SHARE_REGION_BASE	(0x75000000 - SHARE_REGION_SIZE)	
#define	RESERVED_SIZE	(SHARE_REGION_SIZE - sizeof(struct share_region_flags) - sizeof(unsigned long))


struct share_region_flags
{
	unsigned int uboot_flag;
	unsigned int kernel_flag;
};

struct share_region_t 
{
	struct share_region_flags flags;
	unsigned char reserved[RESERVED_SIZE];
	unsigned long checksum;
};

typedef struct share_region_t share_region_t;

//struct  share_region_t *const share_region = (struct share_region_t *)SHARE_REGION_BASE;
/* Values of boot flag */
enum{
	BOOT_NORMAL = 0x0,
	BOOT_SYSTEM_UPGRADE,
	BOOT_OTA,
};

int read_share_region();
int share_region_handle();
//void set_share_region(int flag);
//int get_update_flag();
void clear_flags();
