#include <common.h>
#include <share_region.h>
#include <asm/arch/imx_spi_pmic.h>
#include <imx_spi.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx50.h>
#include <asm/arch/mx50_pins.h>

int tmp = 0;
static share_region_t *const share_region = (share_region_t *)SHARE_REGION_BASE;

/* return the CRC */
static unsigned bg_crc32(unsigned char *buf, int len)
{
	return crc32(0L, buf, len);
}

int read_share_region()
{
	return share_region->checksum;	
}

/* CRC right return 1, else return 0 */
static int check_share_region(void)
{
	int result = 0;
	
	if (share_region)
	{
		unsigned long computed_residue, checksum;

		checksum = share_region->checksum;

		computed_residue = ~bg_crc32((unsigned char*)share_region, SHARE_REGION_SIZE);
	//	printf("the computed_residue = %lx\n", computed_residue);	
		result = CRC32_RESIDUE == computed_residue;
	}
//	printf("the result is %x\n", result);
	return result; 
	
}

static void save_share_region(void)
{
	if (share_region) {
		share_region->checksum = bg_crc32((unsigned char*)share_region, SHARE_REGION_SIZE - sizeof(unsigned long ));
//		printf("the checksum addr = %p\n", &(share_region->checksum));
//		printf("share region checksum = %lx\n", share_region->checksum);
	}
}

void clear_flags(void)
{
	share_region->flags.uboot_flag = 0;
	share_region->flags.kernel_flag = 0;
}

/***************************************
* update_flag=0, BOOT_NORMAL
* update_flag=1, BOOT_SYSTEM_UPGRADE
* update_flag=2, BOOT_OTA
****************************************/
int recovery_handle(void)
{
	unsigned int val = 0;
	unsigned int reg1 = 0;
	unsigned int reg2 = 0;
	struct spi_slave *slave;


	if (share_region->flags.uboot_flag != BOOT_NORMAL)
		goto out;

/* No power key any more*/
#if 0
	slave = spi_pmic_probe();
	val = pmic_reg(slave, 3, 0, 0);
	val = (val & 0x000004) >> 2;
#endif
	mxc_request_iomux(MX50_PIN_KEY_COL3, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_KEY_ROW0, IOMUX_CONFIG_ALT1);
	reg1 = readl(GPIO4_BASE_ADDR);	
	reg1 &= 0x00000040;
	reg2 = readl(GPIO4_BASE_ADDR);
	reg2 &= 0x00000002;
#if 1
	//printf("val = %d	reg1= %0x	reg2= %0x\n", val, reg1, reg2);
	if ((reg1 && reg2) != 1 && (val == 1)) 
		clear_flags();
	val = readl(SRC_BASE_ADDR + 0x4);
	//printf("src_val %0x\n", (val & 0x00000018)>>3);
#endif
	if (1 == (reg1 && reg2)) 
		share_region->flags.uboot_flag = BOOT_SYSTEM_UPGRADE;
out:
	save_share_region();
	printf("share_region = %d\n", share_region->flags.uboot_flag);
	return share_region->flags.uboot_flag;
}

int get_update_flag(void)
{
	return share_region->flags.uboot_flag;
}

int share_region_handle(void)
{
	if (!check_share_region()) {
		memset(share_region, 0x0, 0x1000);	
	}
	
	tmp = share_region->flags.kernel_flag;  //this can clear the flag
	memset(share_region, 0x0, 0x1000);
	share_region->flags.uboot_flag = tmp;

	return recovery_handle();
}

void set_share_region(int flag)
{
	clear_flags();
	share_region->flags.kernel_flag = flag;
	save_share_region();
}

/*
 * check boot way insert USB/Adapter or press power key
 *
 */
int check_boot_way()
{
	int ret;

	return ret;
}
