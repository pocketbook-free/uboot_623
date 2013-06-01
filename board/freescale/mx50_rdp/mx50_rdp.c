/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2009-2011 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mx50.h>
#include <asm/arch/mx50_pins.h>
#include <asm/arch/iomux.h>
#include <asm/errno.h>

#include <waveform.h>

#ifdef CONFIG_IMX_CSPI
#include <imx_spi.h>
#include <asm/arch/imx_spi_pmic.h>
#endif

#if CONFIG_I2C_MXC
#include <i2c.h>
#endif

#ifdef CONFIG_CMD_MMC
#include <mmc.h>
#include <fsl_esdhc.h>
#endif

#ifdef CONFIG_ARCH_MMU
#include <asm/mmu.h>
#include <asm/arch/mmu.h>
#endif

#ifdef CONFIG_CMD_CLOCK
#include <asm/clock.h>
#endif

#ifdef CONFIG_MXC_EPDC
#include <lcd.h>
#endif

#ifdef CONFIG_ANDROID_RECOVERY
#include "../common/recovery.h"
#include <part.h>
#include <ext2fs.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <ubi_uboot.h>
#include <jffs2/load_kernel.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

//#define BASE_EP7_BOARD	1

static u32 system_rev;
static enum boot_device boot_dev;
unsigned int volt_recovery;

unsigned char waveform_load = 0;

unsigned char *gptWfmHeader;
unsigned char *gptWfmAddr;
unsigned char *gptWbufAddr;
unsigned char *gptLogoAddr;

#define msleep(x) udelay(1000 * x)

int  enable_charge_indicator(int value)
{	unsigned char reg;
	if(value == 0 || value ==1)
	{
	mxc_request_iomux(MX50_PIN_I2C3_SDA, IOMUX_CONFIG_ALT1);
	reg = readl(GPIO6_BASE_ADDR + 0x4);
	reg |= (1 << 23);
	writel(reg, GPIO6_BASE_ADDR + 0x4);
	if( value == 1 )
	{
	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg |= (1 << 23);
	writel(reg, GPIO6_BASE_ADDR + 0x0);
	}
	if( value == 0 )
	{
	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg &= ~(1 << 23);
	writel(reg, GPIO6_BASE_ADDR + 0x0);
	}
	}else
	{	
		printf("sorry set value error\n");
		return -1;
	}
	return 0;
}

#ifdef CONFIG_IMX_CSPI
#define MC13892_REG_ISENSE0			2
#define MC13892_REG_ISENSE1			5
#define MC13892_REG_PU_MODE     6
#define MC13892_REG_24				24
#define MC13892_REG_25				25
#define MC13892_REG_POWER_CTL0			13
#define MC13892_REG_ADC0			43
#define MC13892_REG_ADC1			44
#define MC13892_REG_ADC2			45
#define MC13892_REG_ADC3			46
#define MC13892_REG_ADC4			47
#define MC13892_REG_CHARGE0			48
#define MC13892_REG_LED			52  //Add by sunny
#define MC13892_REG_MEMA			18
#define MC13892_LED_CURRENT (2 << 9) 
#define MC13892_LED_CYCLE (32 << 3)

#define MC13892_ISENSE0_VBUSVALIDS	(1<<3)	
#define MC13892_ISENSE0_CHGDETS		(1<<6)	
#define MC13892_ISENSE0_CHGCURRS	(1<<11)
#define IsChargerPresence(a)	( ((a) & (MC13892_ISENSE0_VBUSVALIDS|MC13892_ISENSE0_CHGDETS)) ==  \
								  (MC13892_ISENSE0_VBUSVALIDS|MC13892_ISENSE0_CHGDETS) )

#define MC13892_ADC0_CHRGICON_MASK		(1<<1)
#define MC13892_ADC0_BATICON_MASK		(1<<2)
#define MC13892_ADC0_ADRESET		(1 << 8)
#define MC13892_ADC0_ADREFEN		(1 << 10)
#define MC13892_ADC0_ADREFMODE		(1 << 11)
#define MC13892_ADC0_TSMOD0		(1 << 12)
#define MC13892_ADC0_TSMOD1		(1 << 13)
#define MC13892_ADC0_TSMOD2		(1 << 14)
#define MC13892_ADC0_ADINC1		(1 << 16)
#define MC13892_ADC0_ADINC2		(1 << 17)

#define MC13892_ADC0_TSMOD_MASK		(MC13892_ADC0_TSMOD0 | \
					MC13892_ADC0_TSMOD1 | \
					MC13892_ADC0_TSMOD2)


#define MC13892_ADC1_ADEN		(1 << 0)
#define MC13892_ADC1_RAND		(1 << 1)
#define MC13892_ADC1_ADCCAL		(1 << 2)
#define MC13892_ADC1_ADSEL		(1 << 3)
#define MC13892_ADC1_ASC		(1 << 20)
#define MC13892_ADC1_ADTRIGIGN		(1 << 21)
#define MC13892_ADC1_DEFAULT_ATO	(0x080800)

#define MC13892_ADC1_ADA1_SHITT      5
#define MC13892_ADC1_ADA1_MASK       0xE0
#define MC13892_ADC1_ADA2_SHITT      8
#define MC13892_ADC1_ADA2_MASK       0x700								

#define MC13892_ADC2_ADD1_SHIFT		    2
#define MC13892_ADC2_ADD1_MASK		    0x3FF
#define MC13892_ADC2_ADD2_SHIFT		    14
#define MC13892_ADC2_ADD2_MASK		    0x3FF


#define MC13892_CHARGE0_ACKLPB			(1<<8)
#define MC13892_CHARGE0_CHGAUTOVIB		(1<<23)
#define MC13892_CHARGE0_ICHRG_SHIFT		3
#define MC13892_CHARGE0_ICHRG_MASK		0x78
#define MC13892_CHARGE0_THCHKB_SHIFT		(9)
#define	MC13892_CHARGE0_PLIM_SHIFT              (15)  
#define MC13892_CHARGE0_CHGRESTART_SHIFT	(20)
#define MC13892_CHARGE0_CHGAUTOB_SHIFT		(21)
#define MC13892_CHARGE0_PLIMDIS_SHIFT		(17)

#define VBATT_SAFE_BOOT_VOLT    350  //3.5V
#define VBATT_BAT_OK            300  //3.00V  // exit trickle charge
#define CHARGER_VOLT   45 //4.5V

void check_battery(void)
{
	struct spi_slave *slave;
	unsigned int val,volt_bat;
	unsigned int adc1,adc1_cur;
	int  i,chg_curr, ii, iDelay;
	static int show_low_batt_logo = 0;
	//unsigned int is_adapter;
	int soc;
//	puts("Check battery\n");

	/* Enable VGEN1 to enable ethernet */
	slave = spi_pmic_probe();

/*add by sunny for led*/	
	val = pmic_reg(slave, MC13892_REG_LED, 0, 0);
		val |= MC13892_LED_CURRENT;
		val |= MC13892_LED_CYCLE;
	pmic_reg(slave,  MC13892_REG_LED, val, 1);
	val = pmic_reg(slave, MC13892_REG_LED, 0, 0);	
//	printf("led on: %x\n",val);
	
	while(1)
	{
	// reset ADC
	val = pmic_reg(slave, MC13892_REG_ADC0, 0, 0);
	pmic_reg(slave, MC13892_REG_ADC0, (val|MC13892_ADC0_ADRESET), 1);
	udelay(100);
	val &= (~MC13892_ADC0_ADRESET);
	pmic_reg(slave, MC13892_REG_ADC0, val, 1);
	udelay(1000);

	// enable ADC calibration once 0 2 21 20 11 19
	adc1 = MC13892_ADC1_ADEN | MC13892_ADC1_ADCCAL | MC13892_ADC1_ADTRIGIGN | MC13892_ADC1_ASC | MC13892_ADC1_DEFAULT_ATO;
	
	for(ii=0; ii<3; ii++)
	{
		//arbitrarily set ADA2 to channel 3 
		adc1 |= (3<< MC13892_ADC1_ADA2_SHITT);
		pmic_reg(slave, MC13892_REG_ADC1, adc1, 1);

		/* Wait for conversation complete */
		for (iDelay=0; iDelay<10000; iDelay++) {
			adc1_cur = pmic_reg(slave, MC13892_REG_ADC1, 0, 0);
			if(ii==0)	// calibration loop
			{
				if ( !(adc1_cur & (MC13892_ADC1_ADCCAL|MC13892_ADC1_ASC)) )
					break;
			}
			else  // ADC conversion loop
			{
				if (!(adc1_cur & MC13892_ADC1_ASC))
					break;
			}
			udelay(100);
		}

		// no ADCCAL here
		adc1 = MC13892_ADC1_ADEN | MC13892_ADC1_ADTRIGIGN | MC13892_ADC1_ASC | MC13892_ADC1_DEFAULT_ATO;

		//read ADC2
		val = pmic_reg(slave, MC13892_REG_ADC2, 0, 0);
	}
	// always take the 2nd ADC read and ignore the 1 ADC read after calibration

	//get Battery voltage from ADD1
	volt_bat = val >> MC13892_ADC2_ADD1_SHIFT;
	volt_bat &= MC13892_ADC2_ADD1_MASK;

	volt_bat = (volt_bat+1)*480/1024*10;
	if(4200 <= volt_bat)
		volt_bat = 4200;

	printf("Battery voltage: %dmV\n",volt_bat);
	volt_recovery = volt_bat;
	soc = max17043_soc_read();
	if(100 <= soc)
		soc = 100;

	//printf("Battery capacity: %d%\n",soc);
	if(volt_bat < 3640)
//	if( soc < 4 && volt_bat < 3640 )
	{
        if(show_low_batt_logo == 0 && waveform_load == 1) {
			//printf("[EINK]show low battery logo...\n");
            setup_splash_img(2);
            show_logo(2);
            show_low_batt_logo = 1;
		}
		val = pmic_reg(slave, MC13892_REG_ISENSE0, 0, 0);
		if(IsChargerPresence(val))
		{
			// set power limit = 1200mW
			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val |= (1<< MC13892_CHARGE0_THCHKB_SHIFT);
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);
			
			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val |= (3<< MC13892_CHARGE0_PLIM_SHIFT);
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);

			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val |= (1<< MC13892_CHARGE0_CHGAUTOB_SHIFT);
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);
			

			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val |= (1<< MC13892_CHARGE0_CHGRESTART_SHIFT);
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);
			
			//puts("change charge current to 400mA ...\n");

			// set CHARGE0_CHGAUTOVIB = 1
			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val |= MC13892_CHARGE0_CHGAUTOVIB;
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);

			// set CHARGE0_ICHRG = 400mA
			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val &= ~MC13892_CHARGE0_ICHRG_MASK;
			val |= (4<<MC13892_CHARGE0_ICHRG_SHIFT);
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);

			udelay(1000);

			// READ CHARGE CURRENT

			// enable charge current reading
			val = pmic_reg(slave, MC13892_REG_ADC0, 0, 0);
			val |= MC13892_ADC0_CHRGICON_MASK;
			pmic_reg(slave, MC13892_REG_ADC0, val, 1);

			//set ADA1 to channel 1 && 
			//ADA2 to channel 4 for charge current
			adc1 = MC13892_ADC1_ADEN | MC13892_ADC1_ADTRIGIGN | MC13892_ADC1_ASC | MC13892_ADC1_DEFAULT_ATO;
			adc1 |= (4<< MC13892_ADC1_ADA2_SHITT);
			pmic_reg(slave, MC13892_REG_ADC1, adc1, 1);

			/* Wait for conversation complete */
			for (iDelay=0; iDelay<10000;iDelay++) {
				adc1_cur = pmic_reg(slave, MC13892_REG_ADC1, 0, 0);
				if (!(adc1_cur & MC13892_ADC1_ASC))
					break;

				udelay(100);
			}

			//read ADC2
			val = pmic_reg(slave, MC13892_REG_ADC2, 0, 0);

			// get charging current from ADD2
			chg_curr = (val >> MC13892_ADC2_ADD2_SHIFT) & MC13892_ADC2_ADD2_MASK;
			if((chg_curr & 0x200))
			{
				// negative value
				chg_curr = 0 - ((~chg_curr + 1) & 0x1FF);
			}
			chg_curr = (chg_curr * 5865)/1000;

			printf("charger current:%d mA\n", chg_curr);
			/* 
			if((volt_bat < 340) && (chg_curr < 100))
			{
			puts("battery is lower than 3.4V with USB, Please use adapter to charge\n");
			udelay(1000);
			val = pmic_reg(slave, MC13892_REG_POWER_CTL0, 0, 0);
			val |= 0x8;
			pmic_reg(slave,  MC13892_REG_POWER_CTL0, val, 1);

			}
			*/	
			// disable charge current measurement
			/*val = pmic_reg(slave, MC13892_REG_ADC0, 0, 0);
			val &= (~MC13892_ADC0_CHRGICON_MASK);
			pmic_reg(slave, MC13892_REG_ADC0, val, 1);

			if( chg_curr < 150 )
			{
				//charger current too low, shut down
				puts("Low charge current, MC13982 power off for tickle charge\n");
				udelay(1000);
				val = pmic_reg(slave, MC13892_REG_POWER_CTL0, 0, 0);
				val |= 0x8;
				pmic_reg(slave,  MC13892_REG_POWER_CTL0, val, 1);		
			}*/	
	
		}
		else
		{
			//no charger, shut down
			puts("battery is lower than 4% without charger, Unit power off\n");
			udelay(1000);
			val = pmic_reg(slave, MC13892_REG_POWER_CTL0, 0, 0);
			val |= 0x8;
			pmic_reg(slave,  MC13892_REG_POWER_CTL0, val, 1);

		}
	}
	else
	{
	  break;
	}
	for(i=0;i<6000;i++)
	udelay(1000);
	}//end while
	/*
  val = pmic_reg(slave, MC13892_REG_MEMA, 0, 0);
	//printf("MEMA value is : 0x%x\n", val);
	val &= 0x1;
	if(val){ 	  
		  pmic_reg(slave, MC13892_REG_MEMA, 0, 1);
		
	  	val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val |= (1<< MC13892_CHARGE0_CHGAUTOB_SHIFT);
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);
			

			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val |= (1<< MC13892_CHARGE0_CHGRESTART_SHIFT);
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);
			
			puts("power off charge, charging current is 480mA with USB and 800mA with adapter ...\n");
      
      val = pmic_reg(slave, MC13892_REG_PU_MODE, 0, 0);
      //printf("se1b value is : 0x%x\n", val);
			is_adapter = ((val >> 10) & 0x1);
      //printf("is_adapter value is : 0x%x\n", is_adapter);
      
			// set CHARGE0_CHGAUTOVIB = 1
			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val |= MC13892_CHARGE0_CHGAUTOVIB;
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);
      
			// set CHARGE0_ICHRG = 480mA
			val = pmic_reg(slave, MC13892_REG_CHARGE0, 0, 0);
			val &= ~MC13892_CHARGE0_ICHRG_MASK;
			if(is_adapter == 0)
				val |= (9 << MC13892_CHARGE0_ICHRG_SHIFT);
			else
				val |= (5 << MC13892_CHARGE0_ICHRG_SHIFT);
			pmic_reg(slave,  MC13892_REG_CHARGE0, val, 1);
			udelay(1000);
			
			//scan power key press
	    val = pmic_reg(slave, MC13892_REG_ISENSE1, 0, 0);
			val >>= 3;
			val &= 0x1;
			while(val){
				val = pmic_reg(slave, MC13892_REG_ISENSE0, 0, 0);
				if(!(IsChargerPresence(val))){
				  puts("Unit power off\n");
			    udelay(1000);
			    val = pmic_reg(slave, MC13892_REG_POWER_CTL0, 0, 0);
			    val |= 0x8;
			    pmic_reg(slave,  MC13892_REG_POWER_CTL0, val, 1);}
			  val = pmic_reg(slave, MC13892_REG_ISENSE1, 0, 0);
			  val >>= 3;
			  val &= 0x1;
			  udelay(30000);}
			do_reset(NULL, 0, 0, NULL);
  }*/
	spi_pmic_free(slave);
}
#endif	//CONFIG_IMX_CSPI

static inline void setup_boot_device(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	uint bt_mem_ctl = (soc_sbmr & 0x000000FF) >> 4 ;
	uint bt_mem_type = (soc_sbmr & 0x00000008) >> 3;

	switch (bt_mem_ctl) {
	case 0x0:
		if (bt_mem_type)
			boot_dev = ONE_NAND_BOOT;
		else
			boot_dev = WEIM_NOR_BOOT;
		break;
	case 0x2:
		if (bt_mem_type)
			boot_dev = SATA_BOOT;
		else
			boot_dev = PATA_BOOT;
		break;
	case 0x3:
		if (bt_mem_type)
			boot_dev = SPI_NOR_BOOT;
		else
			boot_dev = I2C_BOOT;
		break;
	case 0x4:
	case 0x5:
		boot_dev = SD_BOOT;
		break;
	case 0x6:
	case 0x7:
		boot_dev = MMC_BOOT;
		break;
	case 0x8 ... 0xf:
		boot_dev = NAND_BOOT;
		break;
	default:
		boot_dev = UNKNOWN_BOOT;
		break;
	}
}

enum boot_device get_boot_device(void)
{
	return boot_dev;
}

u32 get_board_rev(void)
{
	return system_rev;
}

static inline void setup_soc_rev(void)
{
	int reg = __REG(ROM_SI_REV);

	switch (reg) {
	case 0x10:
		system_rev = 0x50000 | CHIP_REV_1_0;
		break;
	case 0x11:
		system_rev = 0x50000 | CHIP_REV_1_1_1;
		break;
	default:
		system_rev = 0x50000 | CHIP_REV_1_1_1;
	}
}

static inline void set_board_rev(int rev)
{
	system_rev |= (rev & 0xF) << 8;
}

static inline void setup_board_rev(void)
{
#if defined(CONFIG_MX50_RD3)
	set_board_rev(0x3);
#endif
}

static inline void setup_arch_id(void)
{
#if defined(CONFIG_MX50_RDP) || defined(CONFIG_MX50_RD3)
	gd->bd->bi_arch_number = MACH_TYPE_MX50_RDP;
#elif defined(CONFIG_MX50_ARM2)
	gd->bd->bi_arch_number = MACH_TYPE_MX50_ARM2;
#else
#	error "Unsupported board!"
#endif
}

inline int is_soc_rev(int rev)
{
	return (system_rev & 0xFF) - rev;
}

#ifdef CONFIG_ARCH_MMU
void board_mmu_init(void)
{
	unsigned long ttb_base = PHYS_SDRAM_1 + 0x4000;
	unsigned long i;

	/*
	* Set the TTB register
	*/
	asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

	/*
	* Set the Domain Access Control Register
	*/
	i = ARM_ACCESS_DACR_DEFAULT;
	asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

	/*
	* First clear all TT entries - ie Set them to Faulting
	*/
	memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);
	/* Actual   Virtual  Size   Attributes          Function */
	/* Base     Base     MB     cached? buffered?  access permissions */
	/* xxx00000 xxx00000 */
	X_ARM_MMU_SECTION(0x000, 0x000, 0x10,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* ROM, 16M */
	X_ARM_MMU_SECTION(0x070, 0x070, 0x010,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* IRAM */
	X_ARM_MMU_SECTION(0x100, 0x100, 0x040,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* SATA */
	X_ARM_MMU_SECTION(0x180, 0x180, 0x100,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* IPUv3M */
	X_ARM_MMU_SECTION(0x200, 0x200, 0x200,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* GPU */
	X_ARM_MMU_SECTION(0x400, 0x400, 0x300,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* periperals */
	X_ARM_MMU_SECTION(0x700, 0x700, 0x400,
			ARM_CACHEABLE, ARM_BUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* CSD0 1G */
	X_ARM_MMU_SECTION(0x700, 0xB00, 0x400,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* CSD0 1G */
	X_ARM_MMU_SECTION(0xF00, 0xF00, 0x100,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* CS1 EIM control*/
	X_ARM_MMU_SECTION(0xF80, 0xF80, 0x001,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* iRam */

	/* Workaround for arm errata #709718 */
	/* Setup PRRR so device is always mapped to non-shared */
	asm volatile ("mrc p15, 0, %0, c10, c2, 0" : "=r"(i) : /*:*/);
	i &= (~(3 << 0x10));
	asm volatile ("mcr p15, 0, %0, c10, c2, 0" : : "r"(i) /*:*/);

	/* Enable MMU */
	MMU_ON();
}
#endif

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	return 0;
}

static void setup_uart(void)
{
	unsigned int reg;

#if defined(CONFIG_MX50_RD3)
	/* UART3 TXD */
	mxc_request_iomux(MX50_PIN_UART3_TXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART3_TXD, 0x1E4);
	/* Enable UART1 */
	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg |= (1 << 14);
	writel(reg, GPIO6_BASE_ADDR + 0x0);

	reg = readl(GPIO6_BASE_ADDR + 0x4);
	reg |= (1 << 14);
	writel(reg, GPIO6_BASE_ADDR + 0x4);
#endif
	/* UART1 RXD */
	mxc_request_iomux(MX50_PIN_UART1_RXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_UART1_RXD, 0x1E4);
	mxc_iomux_set_input(MUX_IN_UART1_IPP_UART_RXD_MUX_SELECT_INPUT, 0x1);

	/* UART1 TXD */
	mxc_request_iomux(MX50_PIN_UART1_TXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_UART1_TXD, 0x1E4);
}

#ifdef CONFIG_I2C_MXC
static void setup_i2c(unsigned int module_base)
{
	switch (module_base) {
	case I2C1_BASE_ADDR:
		/* i2c1 SDA */
		mxc_request_iomux(MX50_PIN_I2C1_SDA,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C1_SDA, PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);
		/* i2c1 SCL */
		mxc_request_iomux(MX50_PIN_I2C1_SCL,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C1_SCL, PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);
		break;
	case I2C2_BASE_ADDR:
		/* i2c2 SDA */
		mxc_request_iomux(MX50_PIN_I2C2_SDA,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C2_SDA,
				PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);

		/* i2c2 SCL */
		mxc_request_iomux(MX50_PIN_I2C2_SCL,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C2_SCL,
				PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);
		break;
	case I2C3_BASE_ADDR:
		/* i2c3 SDA */
		mxc_request_iomux(MX50_PIN_I2C3_SDA,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C3_SDA,
				PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);

		/* i2c3 SCL */
		mxc_request_iomux(MX50_PIN_I2C3_SCL,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C3_SCL,
				PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);
		break;
	default:
		printf("Invalid I2C base: 0x%x\n", module_base);
		break;
	}
}

#endif

#ifdef CONFIG_IMX_CSPI
s32 spi_get_cfg(struct imx_spi_dev_t *dev)
{
	switch (dev->slave.cs) {
	case 0:
		/* PMIC */
		dev->base = CSPI3_BASE_ADDR;
		dev->freq = 25000000;
		dev->ss_pol = IMX_SPI_ACTIVE_HIGH;
		dev->ss = 0;
		dev->fifo_sz = 32;
		dev->us_delay = 0;
		break;
	case 1:
		/* SPI-NOR */
		dev->base = CSPI3_BASE_ADDR;
		dev->freq = 25000000;
		dev->ss_pol = IMX_SPI_ACTIVE_LOW;
		dev->ss = 1;
		dev->fifo_sz = 32;
		dev->us_delay = 0;
		break;
	default:
		printf("Invalid Bus ID!\n");
	}

	return 0;
}

void spi_io_init(struct imx_spi_dev_t *dev)
{
	switch (dev->base) {
	case CSPI3_BASE_ADDR:
		mxc_request_iomux(MX50_PIN_CSPI_MOSI, IOMUX_CONFIG_ALT0);
		mxc_iomux_set_pad(MX50_PIN_CSPI_MOSI, 0x4);

		mxc_request_iomux(MX50_PIN_CSPI_MISO, IOMUX_CONFIG_ALT0);
		mxc_iomux_set_pad(MX50_PIN_CSPI_MISO, 0x4);

		if (dev->ss == 0) {
			/* de-select SS1 of instance: cspi */
			mxc_request_iomux(MX50_PIN_ECSPI1_MOSI,
						IOMUX_CONFIG_ALT1);

			mxc_request_iomux(MX50_PIN_CSPI_SS0, IOMUX_CONFIG_ALT0);
			mxc_iomux_set_pad(MX50_PIN_CSPI_SS0, 0xE4);
		} else if (dev->ss == 1) {
			/* de-select SS0 of instance: cspi */
			mxc_request_iomux(MX50_PIN_CSPI_SS0, IOMUX_CONFIG_ALT1);

			mxc_request_iomux(MX50_PIN_ECSPI1_MOSI,
						IOMUX_CONFIG_ALT2);
			mxc_iomux_set_pad(MX50_PIN_ECSPI1_MOSI, 0xE4);
			mxc_iomux_set_input(
			MUX_IN_CSPI_IPP_IND_SS1_B_SELECT_INPUT, 0x1);
		}

		mxc_request_iomux(MX50_PIN_CSPI_SCLK, IOMUX_CONFIG_ALT0);
		mxc_iomux_set_pad(MX50_PIN_CSPI_SCLK, 0x4);
		break;
	case CSPI2_BASE_ADDR:
	case CSPI1_BASE_ADDR:
		/* ecspi1-2 fall through */
		break;
	default:
		break;
	}
}
#endif

#ifdef CONFIG_NAND_GPMI
void setup_gpmi_nand(void)
{
	u32 src_sbmr = readl(SRC_BASE_ADDR + 0x4);

	/* Fix for gpmi gatelevel issue */
	mxc_iomux_set_pad(MX50_PIN_SD3_CLK, 0x00e4);

	/* RESETN,WRN,RDN,DATA0~7 Signals iomux*/
	/* Check if 1.8v NAND is to be supported */
	if ((src_sbmr & 0x00000004) >> 2)
		*(u32 *)(IOMUXC_BASE_ADDR + PAD_GRP_START + 0x58) = (0x1 << 13);

	/* RESETN */
	mxc_request_iomux(MX50_PIN_SD3_WP, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_WP, PAD_CTL_DRV_HIGH);

	/* WRN */
	mxc_request_iomux(MX50_PIN_SD3_CMD, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_CMD, PAD_CTL_DRV_HIGH);

	/* RDN */
	mxc_request_iomux(MX50_PIN_SD3_CLK, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_CLK, PAD_CTL_DRV_HIGH);

	/* D0 */
	mxc_request_iomux(MX50_PIN_SD3_D4, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D4, PAD_CTL_DRV_HIGH);

	/* D1 */
	mxc_request_iomux(MX50_PIN_SD3_D5, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D5, PAD_CTL_DRV_HIGH);

	/* D2 */
	mxc_request_iomux(MX50_PIN_SD3_D6, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D6, PAD_CTL_DRV_HIGH);

	/* D3 */
	mxc_request_iomux(MX50_PIN_SD3_D7, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D7, PAD_CTL_DRV_HIGH);

	/* D4 */
	mxc_request_iomux(MX50_PIN_SD3_D0, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D0, PAD_CTL_DRV_HIGH);

	/* D5 */
	mxc_request_iomux(MX50_PIN_SD3_D1, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D1, PAD_CTL_DRV_HIGH);

	/* D6 */
	mxc_request_iomux(MX50_PIN_SD3_D2, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D2, PAD_CTL_DRV_HIGH);

	/* D7 */
	mxc_request_iomux(MX50_PIN_SD3_D3, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D3, PAD_CTL_DRV_HIGH);

	/*CE0~3,and other four controls signals muxed on KPP*/
	switch ((src_sbmr & 0x00000018) >> 3) {
	case  0:
		/* Muxed on key */
		if ((src_sbmr & 0x00000004) >> 2)
			*(u32 *)(IOMUXC_BASE_ADDR + PAD_GRP_START + 0x20) =
								(0x1 << 13);

		/* CLE */
		mxc_request_iomux(MX50_PIN_KEY_COL0, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_COL0, PAD_CTL_DRV_HIGH);

		/* ALE */
		mxc_request_iomux(MX50_PIN_KEY_ROW0, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_ROW0, PAD_CTL_DRV_HIGH);

		/* READY0 */
		mxc_request_iomux(MX50_PIN_KEY_COL3, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_COL3,
				PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL |
				PAD_CTL_100K_PU);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_RDY0_IN_SELECT_INPUT,
			INPUT_CTL_PATH0);

		/* DQS */
		mxc_request_iomux(MX50_PIN_KEY_ROW3, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_ROW3, PAD_CTL_DRV_HIGH);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_DQS_IN_SELECT_INPUT,
			INPUT_CTL_PATH0);

		/* CE0 */
		mxc_request_iomux(MX50_PIN_KEY_COL1, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_COL1, PAD_CTL_DRV_HIGH);

		/* CE1 */
		mxc_request_iomux(MX50_PIN_KEY_ROW1, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_ROW1, PAD_CTL_DRV_HIGH);

		/* CE2 */
		mxc_request_iomux(MX50_PIN_KEY_COL2, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_COL2, PAD_CTL_DRV_HIGH);

		/* CE3 */
		mxc_request_iomux(MX50_PIN_KEY_ROW2, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_ROW2, PAD_CTL_DRV_HIGH);

		break;
	case 1:
		if ((src_sbmr & 0x00000004) >> 2)
			*(u32 *)(IOMUXC_BASE_ADDR + PAD_GRP_START + 0xc) =
								(0x1 << 13);

		/* CLE */
		mxc_request_iomux(MX50_PIN_EIM_DA8, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA8, PAD_CTL_DRV_HIGH);

		/* ALE */
		mxc_request_iomux(MX50_PIN_EIM_DA9, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA9, PAD_CTL_DRV_HIGH);

		/* READY0 */
		mxc_request_iomux(MX50_PIN_EIM_DA14, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA14,
				PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL |
				PAD_CTL_100K_PU);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_RDY0_IN_SELECT_INPUT,
			INPUT_CTL_PATH2);

		/* DQS */
		mxc_request_iomux(MX50_PIN_EIM_DA15, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA15, PAD_CTL_DRV_HIGH);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_DQS_IN_SELECT_INPUT,
			INPUT_CTL_PATH2);

		/* CE0 */
		mxc_request_iomux(MX50_PIN_EIM_DA10, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA10, PAD_CTL_DRV_HIGH);

		/* CE1 */
		mxc_request_iomux(MX50_PIN_EIM_DA11, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA11, PAD_CTL_DRV_HIGH);

		/* CE2 */
		mxc_request_iomux(MX50_PIN_EIM_DA12, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA12, PAD_CTL_DRV_HIGH);

		/* CE3 */
		mxc_request_iomux(MX50_PIN_EIM_DA13, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA13, PAD_CTL_DRV_HIGH);

		break;
	case 2:
		if ((src_sbmr & 0x00000004) >> 2)
			*(u32 *)(IOMUXC_BASE_ADDR + PAD_GRP_START + 0x48) =
								(0x1 << 13);

		/* CLE */
		mxc_request_iomux(MX50_PIN_DISP_D8, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D8, PAD_CTL_DRV_HIGH);

		/* ALE */
		mxc_request_iomux(MX50_PIN_DISP_D9, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D9, PAD_CTL_DRV_HIGH);

		/* READY0 */
		mxc_request_iomux(MX50_PIN_DISP_D14, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D14,
				PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL |
				PAD_CTL_100K_PU);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_RDY0_IN_SELECT_INPUT,
			INPUT_CTL_PATH1);

		/* DQS */
		mxc_request_iomux(MX50_PIN_DISP_D15, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D15, PAD_CTL_DRV_HIGH);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_DQS_IN_SELECT_INPUT,
			INPUT_CTL_PATH1);

		/* CE0 */
		mxc_request_iomux(MX50_PIN_DISP_D10, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D10, PAD_CTL_DRV_HIGH);

		/* CE1 */
		mxc_request_iomux(MX50_PIN_EIM_DA11, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA11, PAD_CTL_DRV_HIGH);

		/* CE2 */
		mxc_request_iomux(MX50_PIN_DISP_D12, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D12, PAD_CTL_DRV_HIGH);

		/* CE3 */
		mxc_request_iomux(MX50_PIN_DISP_D13, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D13, PAD_CTL_DRV_HIGH);

		break;
	default:
		break;
	}
}
#endif

#ifdef CONFIG_MXC_FEC

#ifdef CONFIG_GET_FEC_MAC_ADDR_FROM_IIM

#define HW_OCOTP_MACn(n)	(0x00000250 + (n) * 0x10)

int fec_get_mac_addr(unsigned char *mac)
{
	u32 *ocotp_mac_base =
		(u32 *)(OCOTP_CTRL_BASE_ADDR + HW_OCOTP_MACn(0));
	int i;

	for (i = 0; i < 6; ++i, ++ocotp_mac_base)
		mac[6 - 1 - i] = readl(++ocotp_mac_base);

	return 0;
}
#endif

static void setup_fec(void)
{
	volatile unsigned int reg;

#if defined(CONFIG_MX50_RDP)
	/* FEC_EN: gpio6-23 set to 0 to enable FEC */
	mxc_request_iomux(MX50_PIN_I2C3_SDA, IOMUX_CONFIG_ALT1);

	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg &= ~(1 << 23);
	writel(reg, GPIO6_BASE_ADDR + 0x0);

	reg = readl(GPIO6_BASE_ADDR + 0x4);
	reg |= (1 << 23);
	writel(reg, GPIO6_BASE_ADDR + 0x4);

#elif defined(CONFIG_MX50_RD3)
	/* FEC_EN: gpio4-15 set to 1 to enable FEC */
	mxc_request_iomux(MX50_PIN_ECSPI1_SS0, IOMUX_CONFIG_ALT1);

	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg |= (1 << 15);
	writel(reg, GPIO4_BASE_ADDR + 0x0);

	reg = readl(GPIO4_BASE_ADDR + 0x4);
	reg |= (1 << 15);
	writel(reg, GPIO4_BASE_ADDR + 0x4);

	/* DCDC_PWREN(GP4_16) set to 0 to enable DCDC_3V15 */
	mxc_request_iomux(MX50_PIN_ECSPI2_SCLK, IOMUX_CONFIG_ALT1);
	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg &= ~(1 << 16);
	writel(reg, GPIO4_BASE_ADDR + 0x0);

	reg = readl(GPIO4_BASE_ADDR + 0x4);
	reg |= (1 << 16);
	writel(reg, GPIO4_BASE_ADDR + 0x4);

	/* Isolate EIM signals and boot configuration signals. - GPIO6_11 to 1*/
	mxc_request_iomux(MX50_PIN_UART2_RXD, IOMUX_CONFIG_ALT1);
	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg |= (1 << 11);
	writel(reg, GPIO6_BASE_ADDR + 0x0);

	reg = readl(GPIO6_BASE_ADDR + 0x4);
	reg |= (1 << 11);
	writel(reg, GPIO6_BASE_ADDR + 0x4);
#endif

	/*FEC_MDIO*/
	mxc_request_iomux(MX50_PIN_SSI_RXC, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX50_PIN_SSI_RXC, 0xC);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_MDI_SELECT_INPUT, 0x1);

	/*FEC_MDC*/
	mxc_request_iomux(MX50_PIN_SSI_RXFS, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX50_PIN_SSI_RXFS, 0x004);

	/* FEC RXD1 */
	mxc_request_iomux(MX50_PIN_DISP_D3, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D3, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_RDATA_1_SELECT_INPUT, 0x0);

	/* FEC RXD0 */
	mxc_request_iomux(MX50_PIN_DISP_D4, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D4, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_RDATA_0_SELECT_INPUT, 0x0);

	 /* FEC TXD1 */
	mxc_request_iomux(MX50_PIN_DISP_D6, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D6, 0x004);

	/* FEC TXD0 */
	mxc_request_iomux(MX50_PIN_DISP_D7, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D7, 0x004);

	/* FEC TX_EN */
	mxc_request_iomux(MX50_PIN_DISP_D5, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D5, 0x004);

	/* FEC TX_CLK */
	mxc_request_iomux(MX50_PIN_DISP_D0, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D0, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_TX_CLK_SELECT_INPUT, 0x0);

	/* FEC RX_ER */
	mxc_request_iomux(MX50_PIN_DISP_D1, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D1, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_RX_ER_SELECT_INPUT, 0);

	/* FEC CRS */
	mxc_request_iomux(MX50_PIN_DISP_D2, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D2, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_RX_DV_SELECT_INPUT, 0);

#if defined(CONFIG_MX50_RDP) || defined(CONFIG_MX50_RD3)
	/* FEC_RESET_B: gpio4-12 */
	mxc_request_iomux(MX50_PIN_ECSPI1_SCLK, IOMUX_CONFIG_ALT1);

	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg &= ~(1 << 12);
	writel(reg, GPIO4_BASE_ADDR + 0x0);

	reg = readl(GPIO4_BASE_ADDR + 0x4);
	reg |= (1 << 12);
	writel(reg, GPIO4_BASE_ADDR + 0x4);

	udelay(500);

	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg |= (1 << 12);
	writel(reg, GPIO4_BASE_ADDR + 0x0);
#elif defined(CONFIG_MX50_ARM2)
	/* phy reset: gpio4-6 */
	mxc_request_iomux(MX50_PIN_KEY_COL3, IOMUX_CONFIG_ALT1);

	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg &= ~0x40;
	writel(reg, GPIO4_BASE_ADDR + 0x0);

	reg = readl(GPIO4_BASE_ADDR + 0x4);
	reg |= 0x40;
	writel(reg, GPIO4_BASE_ADDR + 0x4);

	udelay(500);

	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg |= 0x40;
	writel(reg, GPIO4_BASE_ADDR + 0x0);
#else
#	error "Unsupported board!"
#endif
}
#endif

#ifdef CONFIG_CMD_MMC

struct fsl_esdhc_cfg esdhc_cfg[3] = {
	{MMC_SDHC1_BASE_ADDR, 1, 1},
	{MMC_SDHC2_BASE_ADDR, 1, 1},
	{MMC_SDHC3_BASE_ADDR, 1, 1},
};


#ifdef CONFIG_DYNAMIC_MMC_DEVNO
int get_mmc_env_devno(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	int mmc_devno = 0;

	switch (soc_sbmr & 0x00300000) {
	default:
	case 0x0:
		mmc_devno = 0;
		break;
	case 0x00100000:
		mmc_devno = 1;
		break;
	case 0x00200000:
		mmc_devno = 2;
		break;
	}

	return mmc_devno;
}
#endif

#ifdef CONFIG_EMMC_DDR_PORT_DETECT
int detect_mmc_emmc_ddr_port(struct fsl_esdhc_cfg *cfg)
{
	return (MMC_SDHC3_BASE_ADDR == cfg->esdhc_base) ? 1 : 0;
}
#endif

int esdhc_gpio_init(bd_t *bis)
{
	s32 status = 0;
	u32 index = 0;
	u32 tmp = 0;

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM;
		++index) {
		switch (index) {
		case 0:
			mxc_request_iomux(MX50_PIN_SD1_CMD, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_CLK, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_D0,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_D1,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_D2,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_D3,  IOMUX_CONFIG_ALT0);

			/*adding for SD voltage*/
#ifndef BASE_EP7_BOARD			
			mxc_request_iomux(MX50_PIN_DISP_D8, IOMUX_CONFIG_ALT1);
			tmp = REG_RD_ADDR(GPIO2_BASE_ADDR + GPIO_GDIR);
			REG_WR_ADDR(GPIO2_BASE_ADDR + GPIO_GDIR, tmp | 0x100);	
			tmp = REG_RD_ADDR(GPIO2_BASE_ADDR + GPIO_DR);
			REG_WR_ADDR(GPIO2_BASE_ADDR + GPIO_DR, tmp &( ~0x100));	
#else			
			/*adding for SD voltage for base ep7 board*/
			mxc_request_iomux(MX50_PIN_KEY_COL1, IOMUX_CONFIG_ALT1);
			tmp = REG_RD_ADDR(GPIO4_BASE_ADDR + GPIO_GDIR);
			REG_WR_ADDR(GPIO4_BASE_ADDR + GPIO_GDIR, tmp | 0x4);	
			tmp = REG_RD_ADDR(GPIO4_BASE_ADDR + GPIO_DR);
			REG_WR_ADDR(GPIO4_BASE_ADDR + GPIO_DR, tmp &( ~0x4));	
#endif

			mxc_iomux_set_pad(MX50_PIN_SD1_CMD, 0x1E4);
			mxc_iomux_set_pad(MX50_PIN_SD1_CLK, 0xD4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D0,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D1,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D2,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D3,  0x1D4);

			break;
		case 1:
			mxc_request_iomux(MX50_PIN_SD2_CMD, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_CLK, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D0,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D1,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D2,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D3,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D4,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D5,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D6,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D7,  IOMUX_CONFIG_ALT0);

			mxc_iomux_set_pad(MX50_PIN_SD2_CMD, 0x14);
			mxc_iomux_set_pad(MX50_PIN_SD2_CLK, 0xD4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D0,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D1,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D2,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D3,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D4,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D5,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D6,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D7,  0x1D4);

			break;
		case 2:
			mxc_request_iomux(MX50_PIN_SD3_CMD, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_CLK, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D0,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D1,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D2,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D3,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D4,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D5,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D6,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D7,  IOMUX_CONFIG_ALT0);

			mxc_iomux_set_pad(MX50_PIN_SD3_CMD, 0x1E4);
			mxc_iomux_set_pad(MX50_PIN_SD3_CLK, 0xD4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D0,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D1,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D2,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D3,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D4,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D5,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D6,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D7,  0x1D4);
			
			break;
		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(2)\n",
				CONFIG_SYS_FSL_ESDHC_NUM);
			return status;
			break;
		}
		status |= fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
	}

	return status;
}

int board_mmc_init(bd_t *bis)
{
	if (!esdhc_gpio_init(bis))
		return 0;
	else
		return -1;
}

#endif

#ifdef CONFIG_MXC_EPDC
#ifdef CONFIG_SPLASH_SCREEN
int setup_splash_img(int logo_num)
{
//	printf("%s\n",__func__);
	if(logo_num == 1 || logo_num == 2){
#ifdef CONFIG_SPLASH_IS_IN_MMC
		int mmc_dev = get_mmc_env_devno();
		ulong offset = CONFIG_SPLASH_IMG_OFFSET;
		ulong size = CONFIG_SPLASH_IMG_SIZE;
		ulong addr = 0;
		char *s = NULL;
		struct mmc *mmc = find_mmc_device(mmc_dev);
		uint blk_start, blk_cnt, n;
	
		if(logo_num == 2){
			offset = CONFIG_LOGO2_OFFSET;
			//printf("offset=%0x\n",offset);
		}

	s = getenv("splashimage");

		if (NULL == s) {
			puts("env splashimage not found!\n");
			return -1;
		}
		addr = simple_strtoul(s, NULL, 16);

		if (!mmc) {
			printf("MMC Device %d not found\n",
				mmc_dev);
			return -1;
		}

		if (mmc_init(mmc)) {
			puts("MMC init failed\n");
			return  -1;
		}
		//printf("MMC %d is found\n",mmc_dev);
		blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
		blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
		n = mmc->block_dev.block_read(mmc_dev, blk_start,
						blk_cnt, (u_char *)addr);
		flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	//	printf("blk_start%0x\n",blk_start);

		return (n == blk_cnt) ? 0 : -1;
#endif
	}
	else
	{
		printf("wrong logo num\n");
		return -1;
	}
}
#endif
#endif

vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 1024,
	.vl_row = 758,
	.vl_pixclock = 40000000,
	.vl_left_margin = 6,
	.vl_right_margin = 76,
	.vl_upper_margin = 4,
	.vl_lower_margin = 5,
	.vl_hsync = 6,
	.vl_vsync = 2,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	cmap:0,
};
/* Add for calculate waveform data offset */
struct waveform_data_header {
	unsigned int wi0;
	unsigned int wi1;
	unsigned int wi2;
	unsigned int wi3;
	unsigned int wi4;
	unsigned int wi5;
	unsigned int wi6;
	unsigned int xwia:24;
	unsigned int cs1:8;
	unsigned int wmta:24;
	unsigned int fvsn:8;
	unsigned int luts:8;
	unsigned int mc:8;
	unsigned int trc:8;
	unsigned int reserved0_0:8;
	unsigned int eb:8;
	unsigned int sb:8;
	unsigned int reserved0_1:8;
	unsigned int reserved0_2:8;
	unsigned int reserved0_3:8;
	unsigned int reserved0_4:8;
	unsigned int reserved0_5:8;
	unsigned int cs2:8;
};

struct mxcfb_waveform_data_file {
	struct waveform_data_header wdh;
	u32 *data;	/* Temperature Range Table + Waveform Data */
};

static void setup_epdc_power()
{
	unsigned int reg;

	/* Setup epdc voltage */

	/* EPDC PWRSTAT - GPIO3[28] for EINK_PWR_EN status */
	mxc_request_iomux(MX50_PIN_EPDC_PWRSTAT, IOMUX_CONFIG_ALT1);
	reg = readl(GPIO3_BASE_ADDR + 0X04);
	reg |= (1 << 28);
	writel(reg, GPIO3_BASE_ADDR + 0X04);

	//for ep7 the VCOM_EN is pin GP3_27 
	mxc_request_iomux(MX50_PIN_EPDC_PWRCOM, IOMUX_CONFIG_ALT1);//set the mux mode alt1 and set it as gpio3_27 
	/* Set as output */
	reg = readl(GPIO3_BASE_ADDR + 0x4);
	reg |= (1 << 27);
	writel(reg, GPIO3_BASE_ADDR + 0x4);
//	printf("epdc power GPIO3 DATA %0x\n",readl(GPIO3_BASE_ADDR + 0X0));
}

void epdc_power_on()
{
	unsigned int reg;

#if 0
	/* Set PMIC Wakeup to high - enable Display power */
	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg |= (1 << 16);
	writel(reg, GPIO6_BASE_ADDR + 0x0);
#endif

	/* Wait for EINK_PW_EN == 1 */
#if 0
	while (1) {
		reg = readl(GPIO3_BASE_ADDR + 0x0);
		if (!(reg & (1 << 28)))
			break;

		udelay(100);
	}
#endif
	msleep(6);

	/* Enable EINK_PW_EN and VCOM */
	reg = readl(GPIO3_BASE_ADDR + 0x0);
	reg |= (1 << 28);
	writel(reg, GPIO3_BASE_ADDR + 0x0);

	msleep(21);

	reg = readl(GPIO3_BASE_ADDR + 0x0);
	reg |= (1 << 27);
	writel(reg, GPIO3_BASE_ADDR + 0x0);

//	printf("the GPIO3 reg data is 0x%0x\n",readl(GPIO3_BASE_ADDR + 0x0));
	msleep(1);
}

void  epdc_power_off()
{
	unsigned int reg;
#if 0
	/* Set PMIC Wakeup to low - disable Display power */
	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg &= ~(1 << 16);
	writel(reg, GPIO6_BASE_ADDR + 0x0);
#endif
	/* Disable EINK_PW_EN and VCOM */
	reg = readl(GPIO3_BASE_ADDR + 0x0);
	reg &= ~(1 << 27);
	writel(reg, GPIO3_BASE_ADDR + 0x0);
	msleep(1);

	reg = readl(GPIO3_BASE_ADDR + 0x0);
	reg &= ~(1 << 28);
	writel(reg, GPIO3_BASE_ADDR + 0x0);
	msleep(1);
//	printf("the GPIO3 DATA is 0x%0x\n",readl(GPIO3_BASE_ADDR + 0x0));
}

int setup_waveform_file()
{
#ifdef CONFIG_WAVEFORM_FILE_IN_MMC
	struct mxcfb_waveform_data_file *wv_file;
	int trt_entries;
	int wv_data_offs;
	int mmc_dev = get_mmc_env_devno();
	ulong offset = CONFIG_WAVEFORM_FILE_OFFSET;
	//ulong size = CONFIG_WAVEFORM_FILE_SIZE;
	ulong size = 0;
	ulong addr = WAVEFORM_BASE;
	//ulong addr = CONFIG_WAVEFORM_BUF_ADDR;
	unsigned char *waveform_head = NULL;
	uint waveform_size;
	unsigned short fpl_lot_num;
	char *s = NULL;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	if (!mmc) {
		printf("MMC Device %d not found\n",
			mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	size = 512;

	//printf("mmc->read_bl_len is %d\n", mmc->read_bl_len);
	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
		blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	waveform_head = (unsigned char *)addr;
/*	
*	if (*(waveform_head + 0x14) != 0x3c || *(waveform_head + 0x0D) != 0x6) 
	{
   		  printf("Fatal error: Can't find waveform!\n");
  		  return -1;
	}*/

	waveform_size = *((uint *)waveform_head + 1);
	//printf("Waveform size is %d Bytes\n", waveform_size);

	if((uint)waveform_size <= 0 || (uint)waveform_size > 3145728){
		printf("Waveform file format is wrong!");
		return -1;
	}

	fpl_lot_num = *((unsigned short *)waveform_head + 7);
	printf("fpl_lot_num is %d\n", fpl_lot_num);

/*	if (fpl_lot_num == 156 && waveform_size < 100 * 1024)
	{
		printf("Wrong waveform file length, fixing....\n");
		
		*((uint *)waveform_head + 1) = 973511;
		waveform_size = 973511;
		
		n = mmc->block_dev.block_write(mmc_dev, blk_start,
			         blk_cnt, (u_char *)addr);
		//printf("blk_start is %d\n", blk_start);
		//printf("blk_cnt is %d\n", blk_cnt);

		printf("fix OK!\n");
	}

	if (fpl_lot_num == 162 && waveform_size < 200 * 1024)
	{
		printf("Wrong waveform file length, fixing....\n");
		
		*((uint *)waveform_head + 1) = 1439615;
		waveform_size = 1439615;
		
		n = mmc->block_dev.block_write(mmc_dev, blk_start,
			         blk_cnt, (u_char *)addr);
		//printf("blk_start is %d\n", blk_start);
		//printf("blk_cnt is %d\n", blk_cnt);

		printf("fix OK!\n");
	}
*/	
	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	//printf("blk_start is %d\n", blk_start);
	blk_cnt   = ALIGN(waveform_size, mmc->read_bl_len) / mmc->read_bl_len;
	//printf("blk_cnt is %d\n", blk_cnt);
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
		blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	waveform_load = 1;

  	/* Add for calculate waveform data offset */
	wv_file = (struct mxcfb_waveform_data_file *)addr;
	trt_entries = wv_file->wdh.trc + 1;
	wv_data_offs = sizeof(wv_file->wdh) + trt_entries + 1;
	
	memcpy(CONFIG_WAVEFORM_BUF_HEAD, waveform_head, wv_data_offs);
	gptWfmHeader = (unsigned char *)CONFIG_WAVEFORM_BUF_HEAD;

//	printf("DATA OFFSET 0x%x\n trt_entries0x%x\n",wv_data_offs,trt_entries);
//	memcpy(CONFIG_WAVEFORM_BUF_ADDR, addr + wv_data_offs, CONFIG_WAVEFORM_FILE_SIZE);
	memcpy(CONFIG_WAVEFORM_BUF_ADDR, addr + wv_data_offs, waveform_size - wv_data_offs);
//	printf("exit from %s\n", __func__);
	return (n == blk_cnt) ? 0 : -1;
#endif
}

static void setup_epdc()
{
	unsigned int reg;

	/* epdc iomux settings */
	mxc_request_iomux(MX50_PIN_EPDC_D0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_D1, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_D2, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_D3, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_D4, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_D5, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_D6, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_D7, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_GDCLK, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_GDSP, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_GDOE, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_GDRL, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_SDCLK, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_SDOE, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_SDLE, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_SDSHR, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_BDR0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_SDCE0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_SDCE1, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX50_PIN_EPDC_SDCE2, IOMUX_CONFIG_ALT0);


	/*** epdc Maxim PMIC settings ***/

	/* EPDC PWRSTAT - GPIO3[28] for  eink_pwr_en status*/
	mxc_request_iomux(MX50_PIN_EPDC_PWRSTAT, IOMUX_CONFIG_ALT1);

	/* EPDC VCOM EN - GPIO3[27] for VCOM enable*/
	mxc_request_iomux(MX50_PIN_EPDC_PWRCOM, IOMUX_CONFIG_ALT1);

	/* UART4 TXD - GPIO6[16] for EPD PMIC WAKEUP */
//	mxc_request_iomux(MX50_PIN_UART4_TXD, IOMUX_CONFIG_ALT1);


	/*** Set pixel clock rates for EPDC ***/

	/* EPDC AXI clk and EPDC PIX clk from PLL1 */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CLKSEQ_BYPASS);
	reg &= ~(0x3 << 4);
	reg |= (0x2 << 4) | (0x2 << 12);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CLKSEQ_BYPASS);

	/* EPDC AXI clk enable and set to 200MHz (800/4) */
	reg = readl(CCM_BASE_ADDR + 0xA8);
	reg &= ~((0x3 << 30) | 0x3F);
	reg |= (0x2 << 30) | 0x4;
	writel(reg, CCM_BASE_ADDR + 0xA8);

	/* EPDC PIX clk enable and set to 40MHz (800/20) */
	reg = readl(CCM_BASE_ADDR + 0xA0);
	reg &= ~((0x3 << 30) | (0x3 << 12) | 0x3F);
	reg |= (0x2 << 30) | (0x1 << 12) | 0x14;
	writel(reg, CCM_BASE_ADDR + 0xA0);

	panel_info.epdc_data.working_buf_addr = CONFIG_WORKING_BUF_ADDR;
	panel_info.epdc_data.waveform_buf_addr = CONFIG_WAVEFORM_BUF_ADDR;

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	setup_epdc_power();

	/* Assign fb_base */
	gd->fb_base = CONFIG_FB_BASE;
}


#ifdef CONFIG_IMX_CSPI
static void setup_power(void)
{
	struct spi_slave *slave;
	unsigned int val;

//	puts("PMIC Mode: SPI\n");

	/* Enable VGEN1 to enable ethernet */
	slave = spi_pmic_probe();

#if defined(CONFIG_MX50_RD3)
	/* Set global reset time to 0s*/
	val = pmic_reg(slave, 15, 0, 0);
	val &= ~(0x300);
	pmic_reg(slave, 15, val, 1);
#else
	// increase VDDGP to 1.15V 
	val = pmic_reg(slave, 24, 0, 0);
	val &= 0x7FFFE0;
	val |= 0x000016;
	pmic_reg(slave, 24, val, 1);

	// increase VDDA to 1.275V
	val = pmic_reg(slave, 26, 0, 0);
	val &= 0x7FFFE0;
	val |= 0x00001B;
	pmic_reg(slave, 26, val, 1);

	// SW1MODE to PWM
	val = pmic_reg(slave, 28, 0, 0);
	val &= 0xFFFFF0;
	val |= 0x000009;
	pmic_reg(slave, 28, val, 1);
	
	udelay(10000);

	val = pmic_reg(slave, 30, 0, 0);
	val |= 0x3;
	pmic_reg(slave, 30, val, 1);

	val = pmic_reg(slave, 32, 0, 0);
	val |= 0x1;
	pmic_reg(slave, 32, val, 1);

	/* Adjust run mode output voltage, __Amos_20111024*/
	val = pmic_reg(slave, 26, 0, 0);
	val |= 0x19;
	pmic_reg(slave, 26, val, 1);

	/* Enable VCAM   */
	val = pmic_reg(slave, 33, 0, 0);
	val |= 0x40;
	pmic_reg(slave, 33, val, 1);
#endif
	val = pmic_reg(slave, 14, 0, 0);
	val &= ~0xf00;
	pmic_reg(slave, 14, val, 1);
	
	val = pmic_reg(slave, 14, 0, 0);
	val |= 0xf000;
	pmic_reg(slave, 14, val, 1);
	val = pmic_reg(slave, 14, 0, 0);
	
	val = pmic_reg(slave, 15, 0, 0);
	val |= 0x1000;
	pmic_reg(slave, 15, val, 1);
	
	spi_pmic_free(slave);
}

void setup_voltage_cpu(void)
{
	/* Currently VDDGP 1.05v
	 * no one tell me we need increase the core
	 * voltage to let CPU run at 800Mhz, not do it
	 */

	/* Raise the core frequency to 800MHz */
	writel(0x0, CCM_BASE_ADDR + CLKCTL_CACRR);

}
#endif
extern int epdc_ctrl_init(void *lcdbase);
int board_init(void)
{
#ifdef CONFIG_MFG
/* MFG firmware need reset usb to avoid host crash firstly */
#define USBCMD 0x140
	int val = readl(OTG_BASE_ADDR + USBCMD);
	val &= ~0x1; /*RS bit*/
	writel(val, OTG_BASE_ADDR + USBCMD);
#endif
	/* boot device */
	setup_boot_device();

	/* soc rev */
	setup_soc_rev();

	/* board rev */
	setup_board_rev();

	/* arch id for linux */
	setup_arch_id();

	/* boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	/* iomux for uart */
	setup_uart();

#ifdef CONFIG_MXC_FEC
	/* iomux for fec */
	setup_fec();
#endif

#ifdef CONFIG_NAND_GPMI
	setup_gpmi_nand();
#endif

#ifdef CONFIG_MXC_EPDC
	setup_epdc();
	epdc_ctrl_init(CONFIG_FB_BASE);
#endif

	return 0;
}

#ifdef CONFIG_ANDROID_RECOVERY
struct reco_envs supported_reco_envs[BOOT_DEV_NUM] = {
	{
	 .cmd = NULL,
	 .args = NULL,
	 },
	{
	 .cmd = NULL,
	 .args = NULL,
	 },
	{
	 .cmd = NULL,
	 .args = NULL,
	 },
	{
	 .cmd = NULL,
	 .args = NULL,
	 },
	{
	 .cmd = NULL,
	 .args = NULL,
	 },
	{
	 .cmd = NULL,
	 .args = NULL,
	 },
	{
	 .cmd = CONFIG_ANDROID_RECOVERY_BOOTCMD_MMC,
	 .args = CONFIG_ANDROID_RECOVERY_BOOTARGS_MMC,
	 },
	{
	 .cmd = CONFIG_ANDROID_RECOVERY_BOOTCMD_MMC,
	 .args = CONFIG_ANDROID_RECOVERY_BOOTARGS_MMC,
	 },
	{
	 .cmd = NULL,
	 .args = NULL,
	 },
};

int check_recovery_cmd_file(void)
{
	disk_partition_t info;
	ulong part_length;
	int filelen;

	switch (get_boot_device()) {
	case MMC_BOOT:
	case SD_BOOT:
		{
			block_dev_desc_t *dev_desc = NULL;
			struct mmc *mmc = find_mmc_device(0);

			dev_desc = get_dev("mmc", 0);

			if (NULL == dev_desc) {
				puts("** Block device MMC 0 not supported\n");
				return 0;
			}

			mmc_init(mmc);

			if (get_partition_info(dev_desc,
					CONFIG_ANDROID_CACHE_PARTITION_MMC,
					&info)) {
				printf("** Bad partition %d **\n",
					CONFIG_ANDROID_CACHE_PARTITION_MMC);
				return 0;
			}

			part_length = ext2fs_set_blk_dev(dev_desc,
							CONFIG_ANDROID_CACHE_PARTITION_MMC);
			if (part_length == 0) {
				printf("** Bad partition - mmc 0:%d **\n",
					CONFIG_ANDROID_CACHE_PARTITION_MMC);
				ext2fs_close();
				return 0;
			}

			if (!ext2fs_mount(part_length)) {
				printf("** Bad ext2 partition or disk - mmc 0:%d **\n",
					CONFIG_ANDROID_CACHE_PARTITION_MMC);
				ext2fs_close();
				return 0;
			}

			filelen = ext2fs_open(CONFIG_ANDROID_RECOVERY_CMD_FILE);

			ext2fs_close();
		}
		break;
	case NAND_BOOT:
		return 0;
		break;
	case SPI_NOR_BOOT:
		return 0;
		break;
	case UNKNOWN_BOOT:
	default:
		return 0;
		break;
	}

	return (filelen > 0) ? 1 : 0;

}
#endif

int board_late_init(void)
{
#ifdef CONFIG_IMX_CSPI
	setup_power();
#endif
	return 0;
}

int checkboard(void)
{
#if defined(CONFIG_MX50_RDP)
	printf("Board: MX50 RDP board\n");
#elif defined(CONFIG_MX50_RD3)
	printf("Board: MX50 RD3 board\n");
#elif defined(CONFIG_MX50_ARM2)
	printf("Board: MX50 ARM2 board\n");
#else
#	error "Unsupported board!"
#endif

	printf("Boot Reason: [");

	switch (__REG(SRC_BASE_ADDR + 0x8)) {
	case 0x0001:
		printf("POR");
		break;
	case 0x0009:
		printf("RST");
		break;
	case 0x0010:
	case 0x0011:
		printf("WDOG");
		break;
	default:
		printf("unknown");
	}
	printf("]\n");

	printf("Boot Device: ");
	switch (get_boot_device()) {
	case WEIM_NOR_BOOT:
		printf("NOR\n");
		break;
	case ONE_NAND_BOOT:
		printf("ONE NAND\n");
		break;
	case PATA_BOOT:
		printf("PATA\n");
		break;
	case SATA_BOOT:
		printf("SATA\n");
		break;
	case I2C_BOOT:
		printf("I2C\n");
		break;
	case SPI_NOR_BOOT:
		printf("SPI NOR\n");
		break;
	case SD_BOOT:
		printf("SD\n");
		break;
	case MMC_BOOT:
		printf("MMC\n");
		break;
	case NAND_BOOT:
		printf("NAND\n");
		break;
	case UNKNOWN_BOOT:
	default:
		printf("UNKNOWN\n");
		break;
	}

	return 0;
}

