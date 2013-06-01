/*
 * iic driver for Freescale mx31
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
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
#include "asm/io.h"


#define IADR	0x00
#define IFDR	0x04
#define iicR	0x08
#define I2SR	0x0c
#define I2DR	0x10

#define iicR_IEN	(1 << 7)
#define iicR_IIEN	(1 << 6)
#define iicR_MSTA	(1 << 5)
#define iicR_MTX	(1 << 4)
#define iicR_TX_NO_AK	(1 << 3)
#define iicR_TX_AK	(0 << 3)
#define iicR_RSTA	(1 << 2)

#define I2SR_ICF	(1 << 7)
#define I2SR_IBB	(1 << 5)
#define I2SR_IIF	(1 << 1)
#define I2SR_RX_NO_AK	(1 << 0)


# define iic_BASE	0x63FC8000
//# define CCGR1 		0X53FD406C

#define iic_MAX_TIMEOUT		100000
#define iic_TIMEOUT_TICKET	1

#undef DEBUG
//#define DEBUG 1
#ifdef DEBUG
#define DPRINTF(args...)  printf(args)
#else
#define DPRINTF(args...)
#endif

#define PAD_iic1_SCL  *(volatile unsigned long *)(0x53FA8040)
#define PAD_iic1_SDA *(volatile unsigned long *)(0x53FA8044)

#define PULL_PAD_iic1_SCL *(volatile unsigned long *)(0x53FA82EC)
#define PULL_PAD_iic1_SDA *(volatile unsigned long *)(0x53FA82F0)

static u16 div[] = { 30, 32, 36, 42, 48, 52, 60, 72, 80, 88, 104, 128, 144,
	160, 192, 240, 288, 320, 384, 480, 576, 640, 768, 960,
	1152, 1280, 1536, 1920, 2304, 2560, 3072, 3840
};

static inline void iic_reset(void)
{
	
	writeb(0,iic_BASE + iicR);	/* Reset module */
	writeb(0,iic_BASE + I2SR);
	writeb(iicR_IEN,iic_BASE + iicR);
}

void iic_init(int speed, int unused)
{
	int freq;
	int i,temp;

	iic_reset();
	freq = mxc_get_clock(MXC_IPG_PERCLK);
	for (i = 0; i < 0x1f; i++)
		if (freq / div[i] <= speed)
			break;
	DPRINTF("%s: root clock: %d, speed: %d div: %x\n",
		__func__, freq, speed, i);

	writeb(i,iic_BASE + IFDR);
	DPRINTF("%s:%x\n", __func__, readb(iic_BASE + IFDR));
	temp = iicR_IEN | iicR_IIEN;
	writeb(temp,iic_BASE + iicR);
	writeb(0x7f,iic_BASE + IADR);

}

static int wait_idle(void)
{
	int timeout = iic_MAX_TIMEOUT,status;
	status = readb(iic_BASE + I2SR);
	while ((status & I2SR_IBB) && --timeout) {
		writeb(0,iic_BASE + I2SR);
	DPRINTF("%s:%x\n", __func__, readb(iic_BASE + I2SR));
		udelay(iic_TIMEOUT_TICKET);
	}

	return timeout ? timeout : (!(readb(iic_BASE + I2SR) & I2SR_IBB));
}

static int wait_busy(void)
{
	int timeout = iic_MAX_TIMEOUT;

	while ((!(readb(iic_BASE + I2SR) & I2SR_IBB) && (--timeout))) {
		writeb(0,iic_BASE + I2SR);

		udelay(iic_TIMEOUT_TICKET);
	}
	return timeout ? timeout : (readb(iic_BASE + I2SR) & I2SR_IBB);
	
}

static int wait_complete(void)
{
	int timeout = iic_MAX_TIMEOUT;

	while ((!(readb(iic_BASE + I2SR) & I2SR_ICF)) && (--timeout)) {
		writeb(0,iic_BASE + I2SR);
		udelay(iic_TIMEOUT_TICKET);
	}
	DPRINTF("%s:%x\n", __func__, readb(iic_BASE + I2SR));
	{
		int i;
		for (i = 0; i < 200; i++)
			udelay(10);

	}
	writeb(0,iic_BASE + I2SR);	/* clear interrupt */

	return timeout;
}

static int tx_byte(u8 byte)
{
	writeb(byte,iic_BASE + I2DR);

	if (!wait_complete() || readb(iic_BASE + I2SR) & I2SR_RX_NO_AK) {
		DPRINTF("%s:%x <= %x\n", __func__, readb(iic_BASE + I2SR),
			byte);
		return -1;
	}
	DPRINTF("%s:%x\n", __func__, byte);
	return 0;
}

static int rx_byte(u32 *pdata, int last)
{
	if (!wait_complete())
		return -1;

	if (last)
		writeb(iicR_IEN,iic_BASE + iicR);

	*pdata = readb(iic_BASE + I2DR);
	DPRINTF("%s:%x\n", __func__, *pdata);
	return 0;
}

int iic_probe(uchar chip)
{
	int ret,temp;

	writeb(0,iic_BASE + iicR);	/* Reset module */
	writeb(iicR_IEN,iic_BASE + iicR);
	for (ret = 0; ret < 1000; ret++)
		udelay(1);
	temp = iicR_IEN | iicR_MSTA | iicR_MTX;
	writeb(temp,iic_BASE + iicR); 
	ret = tx_byte(chip << 1);
	writeb(iicR_IEN,iic_BASE + iicR);

	return ret;
}

static int iic_addr(uchar chip, uint addr, int alen)
{
	int i,temp, retry = 0;
	for (retry = 0; retry < 3; retry++) {
		if (wait_idle())
		DPRINTF("%s:%x\n", __func__, readb(iic_BASE + I2SR));
			break;
		iic_reset();
		for (i = 0; i < iic_MAX_TIMEOUT; i++)
			udelay(iic_TIMEOUT_TICKET);
	}
	if (retry >= 3) {
		printf("%s:bus is busy(%x)\n",
		       __func__, readb(iic_BASE + I2SR));
		return -1;
	}

	temp = iicR_IEN | iicR_IIEN| iicR_MSTA | iicR_MTX | iicR_TX_AK;
	writeb(temp,iic_BASE + iicR);
	DPRINTF("after %s:%x\n", __func__, readb(iic_BASE + iicR));
	udelay(100);
	if (!wait_busy()) {
		printf("%s:trigger start fail(%x)\n",
		       __func__, readb(iic_BASE + I2SR));
		return -1;
	}
	if (tx_byte(chip << 1) || (readb(iic_BASE + I2SR) & I2SR_RX_NO_AK)) {
		printf("%s:chip address cycle fail(%x)\n",
		       __func__, readb(iic_BASE + I2SR));
		return -1;
	}

	while (alen--)
		if (tx_byte((addr >> (alen * 8)) & 0xff) ||
		    (readb(iic_BASE + I2SR) & I2SR_RX_NO_AK)) {
			printf("%s:device address cycle fail(%x)\n",
			       __func__, readb(iic_BASE + I2SR));
			return -1;
		}
	return 0;
}

int iic_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int temp,timeout = iic_MAX_TIMEOUT;
	uint ret;

	DPRINTF("%s chip: 0x%02x addr: 0x%04x alen: %d len: %d\n",
		__func__, chip, addr, alen, len);

	if (iic_addr(chip, addr, alen)) {
		printf("iic_addr failed\n");
		return -1;
	}

	temp = iicR_IEN | iicR_MSTA | iicR_MTX | iicR_RSTA;
	writeb(temp,iic_BASE + iicR);
	if (tx_byte(chip << 1 | 1) ||
	    (readb(iic_BASE + I2SR) & I2SR_RX_NO_AK)) {
		printf("%s:Send 2th chip address fail(%x)\n",
		       __func__, readb(iic_BASE + I2SR));
		return -1;
	}
	temp = iicR_IEN | iicR_MSTA |
	    ((len == 1) ? iicR_TX_NO_AK : 0);
	writeb(temp,iic_BASE + iicR);
	DPRINTF("CR=%x\n", readb(iic_BASE + iicR));
	ret = readb(iic_BASE + I2DR);

	while (len--) {
		if (len == 0)
			temp = iicR_IEN | iicR_MSTA | iicR_TX_NO_AK;
			writeb(temp,iic_BASE + iicR);
		if (rx_byte(&ret, len == 0) < 0) {
			printf("Read: rx_byte fail\n");
			return -1;
		}
		*buf++ = ret;
	}

	while (readb(iic_BASE + I2SR) & I2SR_IBB && --timeout) {
		writeb(0,iic_BASE + I2SR);
		udelay(iic_TIMEOUT_TICKET);
	}
	if (!timeout) {
		printf("%s:trigger stop fail(%x)\n",
		       __func__, readb(iic_BASE + I2SR));
	}
	return 0;
}

int iic_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int timeout = iic_MAX_TIMEOUT;
	DPRINTF("%s chip: 0x%02x addr: 0x%04x alen: %d len: %d\n",
		__func__, chip, addr, alen, len);

	if (iic_addr(chip, addr, alen))
		return -1;

	while (len--)
		if (tx_byte(*buf++))
			return -1;

	writeb(iicR_IEN,iic_BASE + iicR); 

	while (readb(iic_BASE + I2SR) & I2SR_IBB && --timeout)
		udelay(iic_TIMEOUT_TICKET);

	return 0;
}
//从IC读取16位数据
static unsigned short max17043_readw(u32 reg)
{
	u16 val = 0;

	//MAX17043 slave address is 0x6c
	iic_read(0x36, reg, 1, (uchar*)&val, 1);

	return val;
}

//向IC写16位数据
static void max17043_writew(u32 reg, u16 val)
{
	iic_write(0x36, reg, 1, (uchar*)&val, 1);
}
int battery_soc(void)
{

	int i = 0;
	ushort soc = 0;
	ushort soc_old = 0;

	PAD_iic1_SCL =0x10;
	PAD_iic1_SDA = 0x10;
	PULL_PAD_iic1_SCL &= ~(0x1<<7);// Pull Up/Down Disable	SCL, SDA
	PULL_PAD_iic1_SDA &= ~(0x1<<7);
	
	//iic初始化
	iic_init(100000,0);

	for (i = 0; i < 100; i++);
	//IC快速启动
	max17043_writew(0x06, 0x4000);

	udelay(600000);
		//读MAX17043 SOC 寄存器
		soc = max17043_readw(0x04);

		//printf("Read soc is 0x%x\n", soc);
		if (soc_old != soc)
		{
			soc_old = soc;
			
			//换算读取到的SOC数据
			soc = (soc & 0xFF) >> 1;
			if(soc >100)
			soc=100;
			//printf("Current Battery SOC is %%%d\n", soc );
		}
	return soc;
	
}
extern int battery_soc(void);
//u_boot命令调用函数
int do_batsoc(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{

	int i = 0;
	ushort soc = 0;
	ushort soc_old = 0;
	uchar soc_h = 0;
	uchar soc_l = 0;
	ushort vol = 0;
	ushort vol_old = 0;
	uchar vol_h = 0;
	uchar vol_l = 0;
	uint result = 0;

	if (1 != argc)
	{
		printf("Usage: %s\n", argv[0]);
		return -1;

	}
	PAD_iic1_SCL =0x10;
	PAD_iic1_SDA = 0x10;
	PULL_PAD_iic1_SCL &= ~(0x1<<7);// Pull Up/Down Disable	SCL, SDA
	PULL_PAD_iic1_SDA &= ~(0x1<<7);
	
	//iic初始化
	iic_init(100000,0);

	for (i = 0; i < 100; i++);
	//IC快速启动
	max17043_writew(0x06, 0x4000);

	udelay(600000);
	
	while(1)
	{
		//读MAX17043 SOC 寄存器
		soc = max17043_readw(0x04);

		//printf("Read soc is 0x%x\n", soc);

		vol = max17043_readw(0x02);

		if (soc_old != soc)
		{
			soc_old = soc;
			
			//换算读取到的SOC数据
			soc_h = (soc & 0xFF) >> 1;
			soc_l = (soc >> 8) & 0xFF;
			soc_l = (uchar)((soc_l / 256.0) * 100);

			printf("Current Battery SOC is %%%d.%.2d\n", soc_h ,soc_l);
		}

		if (vol_old != vol)
		{
			vol_old = vol;   
			
			vol_h = vol & 0xFF; 
			vol_l = (vol >> 12) & 0xFF;  
	
			vol = (vol_h << 4) | vol_l; 

                        // 在u_boot不能直接打印小数 因此只能通过技巧打印小数，以下是打印小数的一种方法
			result = (uint)(vol * 1.25 *100);   

			printf("Current Battery voltage is %d.%dmV\n", result/100, result%100 );
		} 
		
		udelay(40000000);
	}

	return 0;
}


/* -------------------------------------------------------------------- */

U_BOOT_CMD(
		batsoc, 1, 0, do_batsoc,
		"batsoc  - print battery's voltage and state-of-charge information\n",
		NULL
		);
		
