/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX50-RDP Freescale board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/mx50.h>

 /* High Level Configuration Options */
#define CONFIG_MXC
#define CONFIG_MX50
#define CONFIG_MX50_RDP
#define CONFIG_DDR2
#define CONFIG_FLASH_HEADER
#define CONFIG_FLASH_HEADER_OFFSET 0x400

#define CONFIG_SKIP_RELOCATE_UBOOT


#define CONFIG_ARCH_CPU_INIT
//#define CONFIG_ARCH_MMU


#define CONFIG_MX50_HCLK_FREQ	24000000
#define CONFIG_SYS_PLL2_FREQ    400
#define CONFIG_SYS_AHB_PODF     2
#define CONFIG_SYS_AXIA_PODF    0
#define CONFIG_SYS_AXIB_PODF    1

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_64BIT_VSPRINTF

#define BOARD_LATE_INIT
/*
 * Disabled for now due to build problems under Debian and a significant
 * increase in the final file size: 144260 vs. 109536 Bytes.
 */

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_REVISION_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)
/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

/*
 * Hardware drivers
 */
#define CONFIG_MXC_UART
#define CONFIG_UART_BASE_ADDR	UART1_BASE_ADDR

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
//#define CONFIG_CMD_NET
#define CONFIG_NET_RETRY_COUNT  100
#define CONFIG_NET_MULTI 1
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_DNS

#define CONFIG_CMD_MMC
#define CONFIG_CMD_ENV

#define CONFIG_OCOTP    /*for burn ROM*/
#define CONFIG_FREESCALE_FUSE 
/*for fastboot*/
#include <asm/mxc_key_defs.h>

#define CONFIG_USB_DEVICE
#define CONFIG_FASTBOOT     1
#define CONFIG_IMX_UDC      1
#define CONFIG_FASTBOOT_STORAGE_EMMC_SATA
#define CONFIG_FASTBOOT_VENDOR_ID   0xbb4
#define CONFIG_FASTBOOT_PRODUCT_ID  0xc01
#define CONFIG_FASTBOOT_BCD_DEVICE  0x311
#define CONFIG_FASTBOOT_MANUFACTURER_STR  "Freescale"
#define CONFIG_FASTBOOT_PRODUCT_NAME_STR "i.mx51"
#define CONFIG_FASTBOOT_CONFIGURATION_STR  "Android fastboot"
#define CONFIG_FASTBOOT_INTERFACE_STR    "Android fastboot"
#define CONFIG_FASTBOOT_SERIAL_NUM   "12345"
#define CONFIG_FASTBOOT_TRANSFER_BUF     0x76000000
#define CONFIG_FASTBOOT_TRANSFER_BUF_SIZE 0x2000000 /* 90M byte */

#define CONFIG_ANDROID_ROOTFS_PARTITION_MMC 1
#define CONFIG_DIAGS_PARTITION_MMC 1
#define CONFIG_SYSTEM_PARTITION_MMC 2
#define CONFIG_RECOVERY_PARTITION_MMC 4


/*
 *configure for the epdc
 *show logo under u-boot
 */
#define CONFIG_SPLASH_SCREEN 	
#define CONFIG_VIDEO_MX50

#ifdef CONFIG_SPLASH_SCREEN
	#define CONFIG_MXC_EPDC 
	#define LOGO_TEST	1	
	#define LCD_BPP			LCD_MONOCHROME
	#define CONFIG_SPLASH_IS_IN_MMC
	#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1
	#define CONFIG_SPLASH_SCREEN_ALIGN 1

	#define CONFIG_TEMP_INIT_WAVEFORM_ADDR  (0x75000000)
	#define CONFIG_WORKING_BUF_ADDR (TEXT_BASE + 0x100000)
	#define CONFIG_WAVEFORM_BUF_ADDR (TEXT_BASE + 0x400000)
	#define CONFIG_WAVEFORM_BUF_HEAD  (0x75600000)
	#define CONFIG_WAVEFORM_FILE_IN_MMC
	#define CONFIG_WAVEFORM_FILE_OFFSET		0x580600	

	#define CONFIG_FB_BASE (TEXT_BASE + 0x700000)	
#endif

#ifdef CONFIG_SPLASH_IS_IN_MMC
	#define CONFIG_SPLASH_IMG_OFFSET 	0x80400
	#define CONFIG_SPLASH_IMG_SIZE 0x100000	 
#endif

/*define the hardware configuration*/
#define BOARD_EP7A	9
#define	BOARD_ARGS	BOARD_EP7A

#define	DISPLAY_6_756_1024
#define	DISPLAY_ARGS	DISPLAY_6_756_1024

#define	KEY_ARGS	4

#define	AUDIO_SSM2602	1
#define	AUDIO_ARGS	AUDIO_SSM2602

#define	USB_INTERNAL	3	
#define	USB_ARGS	USB_INTERNAL

#define	CAP_TOUCH_ARGS	1

#define IOC_ARGS	1

#define WIFI_ARGS	1

#define	HWCONFIG	(unsigned long long)((BOARD_ARGS<<0)|(DISPLAY_ARGS<<4)|(KEY_ARGS<<8)|(AUDIO_ARGS<<12)|(USB_ARGS<<16)|(CAP_TOUCH_ARGS<<20)|\
						(WIFI_ARGS<<24)|(IOC_ARGS<<28))
/*#define CONFIG_CMD */
#define CONFIG_REF_CLK_FREQ CONFIG_MX50_HCLK_FREQ

#define CONFIG_WAVEFORM 1
#define CONFIG_VCOM	1
#define CONFIG_SHARE_REGION	1

#define CONFIG_DIAG 1

/*IMX50 RDP raw data layout*/
#define CONFIG_MBR_OFFSET	    0
#define CONFIG_MBR_SIZE	    0x200
#define CONFIG_BOOTLOADER_OFFSET   0x400
#define CONFIG_BOOTLOADER_SIZE	    0x40000
#define CONFIG_LOGO_OFFSET	    0x80400
#define CONFIG_LOGO_SIZE	    0x100000
#define CONFIG_LOGO2_OFFSET	    0x180400
#define CONFIG_LOGO2_SIZE	    0x100000
#define CONFIG_KERNEL_OFFSET	    0x280400
#define CONFIG_KERNEL_SIZE	    0x300000
#define CONFIG_WAVEFORM_OFFSET	    0x580600
#define CONFIG_WAVEFORM_SIZE	    0x300000


#define CONFIG_OTHERS_OFFSET	    0xd00000
#define CONFIG_OTHERS_SIZE	    0x100000

/*for diag test*/
#define CONFIG_DIAG_BASE 0x70805000
#define BLK_SIZE 512

/*for boot cramfs*/
#define CONFIG_BOOT_CRAMFS

#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY	1

#define CONFIG_PRIME	"FEC0"

#define CONFIG_LOADADDR		0x70800000	/* loadaddr env var */
#define CONFIG_RD_LOADADDR	(CONFIG_LOADADDR + 0x300000)

#define	CONFIG_EXTRA_ENV_SETTINGS					\
		"uboot_addr=0x70800000\0"			\
		"uboot_addr1=0x70800400\0"   \
		"kernel_addr=0x70007FC0\0"		\
		"waveform_addr=0x70600000\0"	\
		"logo_addr=0x70604000\0"	\
		"cramfs_addr=0x71000000\0"	\
		"fastboot_dev=mmc2\0"	\
		"splashimage=0x70900000\0"	\
		"splashpos=m,m\0"	\
		\
		"bootargs_update=setenv bootargs root=/dev/ram0 console=ttymxc0,115200 rootfstype=cramfs rootdelay=2 ramdisk=8192 initrd=${cramfs_addr},8M vcom=${vcom} rootwait\0"\
		"bootargs_cramfs=setenv bootargs root=/dev/ram0 console=ttymxc0,115200 rootfstype=cramfs rootdelay=2 ramdisk=49152 initrd=${cramfs_addr},48M vcom=${vcom} rootwait\0"\
		"bootargs_sd=setenv bootargs console=ttymxc0,115200  fastboot_dev=mmc0 ip=none root=/dev/mmcblk0p2 vcom=${vcom} rootwait\0" \
		"bootargs_nand=setenv bootargs console=ttymxc0,115200  fastboot_dev=mmc2 ip=none root=/dev/mmcblk1p1 vcom=${vcom} rootwait\0" \
		\
		"load_kernel_sd=mmc read 0 ${kernel_addr} 0x1402 0x1800\0" \
		"load_kernel_nand= mmc read 2 ${kernel_addr} 0x1402 0x1800\0" \
		\
		"load_kernel_sys=mmc read 2 ${kernel_addr} 0x1402 0x1800; mmc read 2 ${cramfs_addr} 0x4402 0x4000\0"\
		"fatload_for_cramfs=mmcinfo 0; fatload mmc 0:1 ${kernel_addr} kernel; fatload mmc 0:1 ${cramfs_addr} epcramfs.img\0"\
		\
        "fatload_uboot_sd=mmcinfo 0; fatload mmc 0:1 ${uboot_addr} u-boot.bin; mmc write 0 ${uboot_addr1} 0x2 0x200\0"  \
        "fatload_kernel_sd=mmcinfo 0; fatload mmc 0:1 ${kernel_addr} uImage; mmc write 0 ${kernel_addr} 0x1402 0x1800\0" \
        "fatload_waveform_sd=mmcinfo 0; fatload mmc 0:1 ${waveform_addr} waveform.fw; mmc write 0 ${waveform_addr} 0x2c03 0x1800\0" \
        "fatload_logo1_sd=mmcinfo 0; fatload mmc 0:1 ${logo_addr} logo1.bmp; mmc write 0 ${logo_addr} 0x402 0x800\0" \
        "fatload_logo2_sd=mmcinfo 0; fatload mmc 0:1 ${logo_addr} logo2.bmp; mmc write 0 ${logo_addr} 0xC02 0x800\0" \
		\
        "fatload_uboot_nand=mmcinfo 0; fatload mmc 0:1 ${uboot_addr} u-boot.bin; mmc write 2 ${uboot_addr1} 0x2 0x200\0"  \
        "fatload_kernel_nand=mmcinfo 0; fatload mmc 0:1 ${kernel_addr} uImage; mmc write 2 ${kernel_addr} 0x1402 0x1800\0" \
        "fatload_waveform_nand=mmcinfo 0; fatload mmc 0:1 ${waveform_addr} waveform.fw; mmc write 2 ${waveform_addr} 0x2c03 0x1800\0" \
        "fatload_logo1_nand=mmcinfo 0; fatload mmc 0:1 ${logo_addr} logo1.bmp; mmc write 2 ${logo_addr} 0x402 0x800\0" \
        "fatload_logo2_nand=mmcinfo 0; fatload mmc 0:1 ${logo_addr} logo2.bmp; mmc write 2 ${logo_addr} 0xC02 0x800\0" \
		\
        "prg_uboot_sd=run fatload_uboot_sd\0"  \
        "prg_kernel_sd=run fatload_kernel_sd\0" \
        "prg_waveform_sd=run fatload_waveform_sd\0" \
        "prg_logo1_sd=run fatload_logo1_sd\0" \
        "prg_logo2_sd=run fatload_logo2_sd\0" \
		\
        "prg_uboot_nand=run fatload_uboot_nand\0"  \
        "prg_kernel_nand=run fatload_kernel_nand\0" \
        "prg_waveform_nand=run fatload_waveform_nand\0" \
        "prg_logo1_nand=run fatload_logo1_nand\0" \
        "prg_logo2_nand=run fatload_logo2_nand\0" \
		\
		"bootcmd_update=vcom read; run bootargs_update; run load_kernel_sys; bootm ${kernel_addr} \0"	\
		"bootcmd_cramfs=vcom read; run bootargs_cramfs; run fatload_for_cramfs; bootm ${kernel_addr} \0"	\
		"bootcmd_sd=vcom read;run bootargs_sd; run load_kernel_sd; bootm ${kernel_addr}\0"   \
		"bootcmd_nand=vcom read;run bootargs_nand; run load_kernel_nand; bootm ${kernel_addr}\0" \
		"bootcmd=run bootcmd_nand"

#define CONFIG_ARP_TIMEOUT	200UL

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_PROMPT		"EP7A_U-Boot > "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	0	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x10000

#undef	CONFIG_SYS_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_HZ				1000

#define CONFIG_CMDLINE_EDITING	1

//#define CONFIG_FEC0_IOBASE	FEC_BASE_ADDR
//#define CONFIG_FEC0_PINMUX	-1
//#define CONFIG_FEC0_PHY_ADDR	-1
//#define CONFIG_FEC0_MIIBASE	-1

//#define CONFIG_GET_FEC_MAC_ADDR_FROM_IIM

//#define CONFIG_MXC_FEC
#define CONFIG_MII
#define CONFIG_MII_GASKET
#define CONFIG_DISCOVER_PHY

/*
 * DDR ZQ calibration
 */
#define CONFIG_ZQ_CALIB

/*
 * I2C Configs
 */
#define CONFIG_CMD_I2C          1

#ifdef CONFIG_CMD_I2C
	#define CONFIG_HARD_I2C         1
	#define CONFIG_I2C_MXC          1
	#define CONFIG_SYS_I2C_PORT             I2C1_BASE_ADDR
	#define CONFIG_SYS_I2C_SPEED            100000
	#define CONFIG_SYS_I2C_SLAVE            0xfe
#endif


/*
 * SPI Configs
 */
#define CONFIG_FSL_SF		1
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_IMX_ATMEL	1
#define CONFIG_SPI_FLASH_CS	1
#define CONFIG_IMX_CSPI
#define IMX_CSPI_VER_0_7        1
#define MAX_SPI_BYTES		(8 * 4)
#define CONFIG_IMX_SPI_PMIC
#define CONFIG_IMX_SPI_PMIC_CS 0

/*
 * MMC Configs
 */
#ifdef CONFIG_CMD_MMC
	#define CONFIG_MMC				1
	#define CONFIG_GENERIC_MMC
	#define CONFIG_IMX_MMC
	#define CONFIG_SYS_FSL_ESDHC_NUM        3
	#define CONFIG_SYS_FSL_ESDHC_ADDR       0
	#define CONFIG_SYS_MMC_ENV_DEV  0
	#define CONFIG_DOS_PARTITION	1
	#define CONFIG_CMD_FAT		1
	#define CONFIG_CMD_EXT2		1

	/* detect whether ESDHC1, ESDHC2, or ESDHC3 is boot device */
	#define CONFIG_DYNAMIC_MMC_DEVNO

	#define CONFIG_BOOT_PARTITION_ACCESS
	#define CONFIG_EMMC_DDR_PORT_DETECT
	#define CONFIG_EMMC_DDR_MODE

	/* Indicate to esdhc driver which ports support 8-bit data */
	#define CONFIG_MMC_8BIT_PORTS		0x6   /* ports 1 and 2 */
#endif

/*
 * GPMI Nand Configs
 */
//#define CONFIG_CMD_NAND

#ifdef CONFIG_CMD_NAND
	#define CONFIG_NAND_GPMI
	#define CONFIG_GPMI_NFC_SWAP_BLOCK_MARK
	#define CONFIG_GPMI_NFC_V2

	#define CONFIG_GPMI_REG_BASE	GPMI_BASE_ADDR
	#define CONFIG_BCH_REG_BASE	BCH_BASE_ADDR

	#define NAND_MAX_CHIPS		8
	#define CONFIG_SYS_NAND_BASE		0x40000000
	#define CONFIG_SYS_MAX_NAND_DEVICE	1
#endif

/*
 * APBH DMA Configs
 */
#define CONFIG_APBH_DMA

#ifdef CONFIG_APBH_DMA
	#define CONFIG_APBH_DMA_V2
	#define CONFIG_MXS_DMA_REG_BASE	ABPHDMA_BASE_ADDR
#endif

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 * 1024)	/* regular stack */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)
#define iomem_valid_addr(addr, size) \
	(addr >= PHYS_SDRAM_1 && addr <= (PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE))

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

/* Monitor at beginning of flash */
#define CONFIG_FSL_ENV_IN_MMC

#define CONFIG_ENV_SECT_SIZE    (128 * 1024)
#define CONFIG_ENV_SIZE         CONFIG_ENV_SECT_SIZE

#if defined(CONFIG_FSL_ENV_IN_NAND)
	#define CONFIG_ENV_IS_IN_NAND 1
	#define CONFIG_ENV_OFFSET	0x100000
#elif defined(CONFIG_FSL_ENV_IN_MMC)
	#define CONFIG_ENV_IS_IN_MMC	1
	#define CONFIG_ENV_OFFSET	(320 * 1024)
#elif defined(CONFIG_FSL_ENV_IN_SF)
	#define CONFIG_ENV_IS_IN_SPI_FLASH	1
	#define CONFIG_ENV_SPI_CS		1
	#define CONFIG_ENV_OFFSET       (768 * 1024)
#else
	#define CONFIG_ENV_IS_NOWHERE	1
#endif
#endif				/* __CONFIG_H */
