/*
 * Freescale On-Chip OTP driver
 *
 * Copyright (C) 2010 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <common.h>
#include <asm/arch/mx50.h>
#include "regs-ocotp-v2.h"


#define REGS_OCOTP_BASE	OCOTP_CTRL_BASE_ADDR

#define HW_OCOTP_CUSTn(n)	(0x00000030 + (n) * 0x10)
#define BF(value, field) 	(((value) << BP_##field) & BM_##field)

#define DEF_RELEX	(15)	/* > 10.5ns */


static int otp_wait_busy(u32);

static int otp_write_bits(int addr, u32 data, u32 magic)
{
	u32 c; /* for control register */
	u32 i;

	/* init the control register */
	c = REG_RD_ADDR(REGS_OCOTP_BASE + HW_OCOTP_CTRL);
	c &= ~BM_OCOTP_CTRL_ADDR;
	c |= BF(addr, OCOTP_CTRL_ADDR);
	c |= BF(magic, OCOTP_CTRL_WR_UNLOCK);
	REG_WR_ADDR(REGS_OCOTP_BASE + HW_OCOTP_CTRL, c);

	/* init the data register */
	REG_WR_ADDR(REGS_OCOTP_BASE + HW_OCOTP_DATA, data);
	otp_wait_busy(0);

	//mdelay(2); /* Write Postamble */
	
  for (i=0; i<4; i++)
  udelay(500);

	return 0;
}

static int otp_write_post(void)
{
	/* Reload all the shadow registers */
	REG_WR_ADDR(REGS_OCOTP_BASE + HW_OCOTP_CTRL_SET,
			BM_OCOTP_CTRL_RELOAD_SHADOWS);
	udelay(1);
	otp_wait_busy(BM_OCOTP_CTRL_RELOAD_SHADOWS);
	return 0;
}

static int otp_wait_busy(u32 flags)
{
	int count;
	u32 c;

	for (count = 10000; count >= 0; count--) {
		c = REG_RD_ADDR(REGS_OCOTP_BASE + HW_OCOTP_CTRL);
		if (!(c & (BM_OCOTP_CTRL_BUSY | BM_OCOTP_CTRL_ERROR | flags)))
			break;
		//cpu_relax();
	}
	if (count < 0)
		return -1;
	return 0;
}

static int set_otp_timing(void)
{
	unsigned long freq = 0;
	unsigned long relex, sclk_count, rd_busy;
	u32 timing = 0;

	/* get the clock. It needs the AHB clock,though doc writes APB.*/

	freq = mxc_get_clock(MXC_AHB_CLK);
  //printf("get otp clock %d\n", freq);

	/* do optimization for too many zeros */
	relex	= freq / (1000000000 / DEF_RELEX) + 1;
	sclk_count = freq / (1000000000 / 5000) + 1 + DEF_RELEX;
	rd_busy	= freq / (1000000000 / 300)	+ 1;

  //printf("relax: %d\n",relex);
  //printf("sclk_count: %d\n",sclk_count);
  //printf("rd_busy: %d\n",rd_busy);
  
	timing = BF(relex, OCOTP_TIMING_RELAX);
	timing |= BF(sclk_count, OCOTP_TIMING_SCLK_COUNT);
	timing |= BF(rd_busy, OCOTP_TIMING_RD_BUSY);

	REG_WR_ADDR(REGS_OCOTP_BASE + HW_OCOTP_TIMING, timing);
	//printf("ocotp timing address: 0x%x\n",REGS_OCOTP_BASE + HW_OCOTP_TIMING);
	return 0;
}


static int otp_write_prepare()
{
	int ret = 0;

	/* [1] set timing */
	ret = set_otp_timing();
	if (ret)
		return ret;

	/* [2] wait */
	otp_wait_busy(0);
	return 0;
}


/* IMX5 does not need to open the bank anymore */
static int otp_read_prepare(void)
{
	return set_otp_timing();
}
static int otp_read_post(void)
{
	return 0;
}


int otp_write(int addr, u32 data)
{
	int ret;
  ret = otp_write_prepare();
  
  if(ret != 0){
  	printf("otp write prepare fail\n");
    return ret;
	}
	
  ret = otp_write_bits(addr, data, 0x3e77);
  ret = otp_write_post();
  return ret; 	
}
	
int otp_read(int addr)
{
	int ret;
  ret = otp_read_prepare();
  
  if(ret != 0){
  	printf("otp read prepare fail\n");
  return ret;
	}
	
	u32 data = REG_RD_ADDR(REGS_OCOTP_BASE + HW_OCOTP_CUSTn(addr));
	ret = otp_read_post();
	return data;
}	
	
