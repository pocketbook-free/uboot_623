/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
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
 *
 */
/*
 * Based on STMP378X LCDIF
 * Copyright 2008 Embedded Alley Solutions, Inc All Rights Reserved.
 */

//#define DEBUG 1

#include <common.h>
#include <lcd.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/types.h>
#include <asm/arch/mx50.h>

#include "mxc_epdc_fb.h"
#include <fp9928_pmic.h>

#define EPD_DEBUG
#ifdef EPD_DEBUG
	#define epd_printf(fmt, args...); printf(fmt, ##args);
#endif
extern int setup_waveform_file();
extern void epdc_power_on();
extern void epdc_power_off();

extern unsigned char *gptWfmHeader;
extern unsigned char *gptWfmAddr;
extern unsigned char *gptWbufAddr;
extern unsigned char *gptLogoAddr;

DECLARE_GLOBAL_DATA_PTR;

void *eink_base;			/* Start of framebuffer memory	*/
void *eink_console_address;	/* Start of console buffer	*/

int lcd_line_length;
int eink_color_fg;
int eink_color_bg;

short console_col;
short console_row;

static int epdc_initialized = 0;

void lcd_initcolregs(void)
{
//	printf("empty function\n");
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
}

#define TEMP_USE_DEFAULT 5

#define UPDATE_MODE_PARTIAL			0x0
#define UPDATE_MODE_FULL			0x1

#define TRUE 1
#define FALSE 0

#define msleep(a)	udelay(a * 1000)


/********************************************************
 * Start Low-Level EPDC Functions
 ********************************************************/

static inline void epdc_set_screen_res(u32 width, u32 height)
{
	u32 val = (height << EPDC_RES_VERTICAL_OFFSET) | width;

	REG_WR(EPDC_BASE, EPDC_RES, val);
}

static inline void epdc_set_update_coord(u32 x, u32 y)
{
	u32 val = (y << EPDC_UPD_CORD_YCORD_OFFSET) | x;

	REG_WR(EPDC_BASE, EPDC_UPD_CORD, val);
}

static inline void epdc_set_update_dimensions(u32 width, u32 height)
{
	u32 val = (height << EPDC_UPD_SIZE_HEIGHT_OFFSET) | width;

	REG_WR(EPDC_BASE, EPDC_UPD_SIZE, val);
}

static void epdc_submit_update(u32 lut_num, u32 waveform_mode, u32 update_mode,
			       int use_cp_val, u32 cp_val, int use_np_val, u32 np_val)
{
	u32 reg_val = 0;
	u32 fix_val = 0;

	if (use_cp_val) {
		fix_val |=
			((np_val << EPDC_UPD_FIXED_FIXCP_OFFSET) &
			EPDC_UPD_FIXED_FIXCP_MASK) | EPDC_UPD_FIXED_FIXCP_EN;

		reg_val = EPDC_UPD_CTRL_USE_FIXED;
	}
	if (use_np_val) {
		fix_val |=
			((np_val << EPDC_UPD_FIXED_FIXNP_OFFSET) &
			EPDC_UPD_FIXED_FIXNP_MASK) | EPDC_UPD_FIXED_FIXNP_EN;

		reg_val = EPDC_UPD_CTRL_USE_FIXED;
	}
	
	REG_WR(EPDC_BASE, EPDC_UPD_FIXED, fix_val);

	reg_val |=
		((lut_num << EPDC_UPD_CTRL_LUT_SEL_OFFSET) &
		EPDC_UPD_CTRL_LUT_SEL_MASK) |
		((waveform_mode << EPDC_UPD_CTRL_WAVEFORM_MODE_OFFSET) &
		EPDC_UPD_CTRL_WAVEFORM_MODE_MASK) |
		update_mode;

	REG_WR(EPDC_BASE, EPDC_UPD_CTRL, reg_val);
}

static inline int epdc_is_lut_active(u32 lut_num)
{
	u32 val = REG_RD(EPDC_BASE, EPDC_STATUS_LUTS);
	int is_active = val & (1 << lut_num) ? TRUE : FALSE;

	return is_active;
}

static inline int epdc_any_luts_active(void)
{
        int any_active = REG_RD(EPDC_BASE, EPDC_STATUS_LUTS) ? TRUE : FALSE;
        return any_active;
}

void wait_for_update_complete(void) 
{
	int i;
	u32 status, activeluts;

	for (i = 0; i < 30; i++) {
		status = REG_RD(EPDC_BASE, EPDC_STATUS);
        	activeluts = REG_RD(EPDC_BASE, EPDC_STATUS_LUTS);
		if ((status & 1) == 0 && activeluts == 0) {
			//printf("wait_for_update_complete: %d ms\n", i*100);
			return;
		}
		msleep(100);
	}
	printf("wait_for_update_complete: operation timeout!\n");

}

static void epdc_set_horizontal_timing(u32 horiz_start, u32 horiz_end,
				       u32 hsync_width, u32 hsync_line_length)
{
	u32 reg_val =
		((hsync_width << EPDC_TCE_HSCAN1_LINE_SYNC_WIDTH_OFFSET) &
		EPDC_TCE_HSCAN1_LINE_SYNC_WIDTH_MASK)
		| ((hsync_line_length << EPDC_TCE_HSCAN1_LINE_SYNC_OFFSET) &
		EPDC_TCE_HSCAN1_LINE_SYNC_MASK);
	REG_WR(EPDC_BASE, EPDC_TCE_HSCAN1, reg_val);

	reg_val =
		((horiz_start << EPDC_TCE_HSCAN2_LINE_BEGIN_OFFSET) &
		EPDC_TCE_HSCAN2_LINE_BEGIN_MASK)
		| ((horiz_end << EPDC_TCE_HSCAN2_LINE_END_OFFSET) &
		EPDC_TCE_HSCAN2_LINE_END_MASK);
	REG_WR(EPDC_BASE, EPDC_TCE_HSCAN2, reg_val);
}

static void epdc_set_vertical_timing(u32 vert_start, u32 vert_end,
					u32 vsync_width)
{
	u32 reg_val =
		((vert_start << EPDC_TCE_VSCAN_FRAME_BEGIN_OFFSET) &
		EPDC_TCE_VSCAN_FRAME_BEGIN_MASK)
		| ((vert_end << EPDC_TCE_VSCAN_FRAME_END_OFFSET) &
		EPDC_TCE_VSCAN_FRAME_END_MASK)
		| ((vsync_width << EPDC_TCE_VSCAN_FRAME_SYNC_OFFSET) &
		EPDC_TCE_VSCAN_FRAME_SYNC_MASK);
	REG_WR(EPDC_BASE, EPDC_TCE_VSCAN, reg_val);
}

static void epdc_init_settings(void)
{
	u32 reg_val;

	/* EPDC_CTRL */
	reg_val = REG_RD(EPDC_BASE, EPDC_CTRL);
	reg_val &= ~EPDC_CTRL_UPD_DATA_SWIZZLE_MASK;
	reg_val |= EPDC_CTRL_UPD_DATA_SWIZZLE_NO_SWAP;
	reg_val &= ~EPDC_CTRL_LUT_DATA_SWIZZLE_MASK;
	reg_val |= EPDC_CTRL_LUT_DATA_SWIZZLE_NO_SWAP;
	REG_SET(EPDC_BASE, EPDC_CTRL, reg_val);

	/* EPDC_FORMAT - 2bit TFT and 4bit Buf pixel format */
	reg_val = EPDC_FORMAT_TFT_PIXEL_FORMAT_2BIT
		| EPDC_FORMAT_BUF_PIXEL_FORMAT_P4N
		| ((0x0 << EPDC_FORMAT_DEFAULT_TFT_PIXEL_OFFSET) &
		EPDC_FORMAT_DEFAULT_TFT_PIXEL_MASK);
	REG_WR(EPDC_BASE, EPDC_FORMAT, reg_val);

	/* EPDC_FIFOCTRL (disabled) */
	reg_val =
		((100 << EPDC_FIFOCTRL_FIFO_INIT_LEVEL_OFFSET) &
		EPDC_FIFOCTRL_FIFO_INIT_LEVEL_MASK)
		| ((200 << EPDC_FIFOCTRL_FIFO_H_LEVEL_OFFSET) &
		EPDC_FIFOCTRL_FIFO_H_LEVEL_MASK)
		| ((100 << EPDC_FIFOCTRL_FIFO_L_LEVEL_OFFSET) &
		EPDC_FIFOCTRL_FIFO_L_LEVEL_MASK);
	REG_WR(EPDC_BASE, EPDC_FIFOCTRL, reg_val);

	/* EPDC_TEMP - Use default temperature */
	REG_WR(EPDC_BASE, EPDC_TEMP, TEMP_USE_DEFAULT);

	/* EPDC_RES */
	epdc_set_screen_res(panel_info.vl_col, panel_info.vl_row);

	/*
	 * EPDC_TCE_CTRL
	 * VSCAN_HOLDOFF = 4
	 * VCOM_MODE = MANUAL
	 * VCOM_VAL = 0
	 * DDR_MODE = DISABLED
	 * LVDS_MODE_CE = DISABLED
	 * LVDS_MODE = DISABLED
	 * DUAL_SCAN = DISABLED
	 * SDDO_WIDTH = 8bit
	 * PIXELS_PER_SDCLK = 4
	 */
	reg_val =
		((4 << EPDC_TCE_CTRL_VSCAN_HOLDOFF_OFFSET) &
		EPDC_TCE_CTRL_VSCAN_HOLDOFF_MASK)
		| EPDC_TCE_CTRL_PIXELS_PER_SDCLK_4;
	REG_WR(EPDC_BASE, EPDC_TCE_CTRL, reg_val);

	/* EPDC_TCE_HSCAN */
	epdc_set_horizontal_timing(panel_info.vl_left_margin,
				panel_info.vl_right_margin,
				panel_info.vl_hsync,
				panel_info.vl_hsync);

	/* EPDC_TCE_VSCAN */
	epdc_set_vertical_timing(panel_info.vl_upper_margin,
				 panel_info.vl_lower_margin,
				 panel_info.vl_vsync);
	/* EPDC_TCE_OE */
	reg_val =
		((10 << EPDC_TCE_OE_SDOED_WIDTH_OFFSET) &
		EPDC_TCE_OE_SDOED_WIDTH_MASK)
		| ((20 << EPDC_TCE_OE_SDOED_DLY_OFFSET) &
		EPDC_TCE_OE_SDOED_DLY_MASK)
		| ((10 << EPDC_TCE_OE_SDOEZ_WIDTH_OFFSET) &
		EPDC_TCE_OE_SDOEZ_WIDTH_MASK)
		| ((20 << EPDC_TCE_OE_SDOEZ_DLY_OFFSET) &
		EPDC_TCE_OE_SDOEZ_DLY_MASK);
	REG_WR(EPDC_BASE, EPDC_TCE_OE, reg_val);

	/* EPDC_TCE_TIMING1 */
	REG_WR(EPDC_BASE, EPDC_TCE_TIMING1, 0x0);

	/* EPDC_TCE_TIMING2 */
	reg_val =
		((480 << EPDC_TCE_TIMING2_GDCLK_HP_OFFSET) &
		EPDC_TCE_TIMING2_GDCLK_HP_MASK)
		| ((20 << EPDC_TCE_TIMING2_GDSP_OFFSET_OFFSET) &
		EPDC_TCE_TIMING2_GDSP_OFFSET_MASK);
	REG_WR(EPDC_BASE, EPDC_TCE_TIMING2, reg_val);

	/* EPDC_TCE_TIMING3 */
	reg_val =
		((0 << EPDC_TCE_TIMING3_GDOE_OFFSET_OFFSET) &
		EPDC_TCE_TIMING3_GDOE_OFFSET_MASK)
		| ((1 << EPDC_TCE_TIMING3_GDCLK_OFFSET_OFFSET) &
		EPDC_TCE_TIMING3_GDCLK_OFFSET_MASK);
	REG_WR(EPDC_BASE, EPDC_TCE_TIMING3, reg_val);

	/*
	 * EPDC_TCE_SDCFG
	 * SDCLK_HOLD = 1
	 * SDSHR = 1
	 * NUM_CE = 1
	 * SDDO_REFORMAT = FLIP_PIXELS
	 * SDDO_INVERT = DISABLED
	 * PIXELS_PER_CE = display horizontal resolution
	 */
	reg_val = EPDC_TCE_SDCFG_SDCLK_HOLD | EPDC_TCE_SDCFG_SDSHR
		| ((1 << EPDC_TCE_SDCFG_NUM_CE_OFFSET) & EPDC_TCE_SDCFG_NUM_CE_MASK)
		| EPDC_TCE_SDCFG_SDDO_REFORMAT_FLIP_PIXELS
		| ((panel_info.vl_col << EPDC_TCE_SDCFG_PIXELS_PER_CE_OFFSET) &
		EPDC_TCE_SDCFG_PIXELS_PER_CE_MASK);
	REG_WR(EPDC_BASE, EPDC_TCE_SDCFG, reg_val);

	/*
	 * EPDC_TCE_GDCFG
	 * GDRL = 1
	 * GDOE_MODE = 0;
	 * GDSP_MODE = 0;
	 */
	reg_val = EPDC_TCE_SDCFG_GDRL;
	REG_WR(EPDC_BASE, EPDC_TCE_GDCFG, reg_val);

	/*
	 * EPDC_TCE_POLARITY
	 * SDCE_POL = ACTIVE LOW
	 * SDLE_POL = ACTIVE HIGH
	 * SDOE_POL = ACTIVE HIGH
	 * GDOE_POL = ACTIVE HIGH
	 * GDSP_POL = ACTIVE LOW
	 */
	reg_val = EPDC_TCE_POLARITY_SDLE_POL_ACTIVE_HIGH
		| EPDC_TCE_POLARITY_SDOE_POL_ACTIVE_HIGH
		| EPDC_TCE_POLARITY_GDOE_POL_ACTIVE_HIGH;
	REG_WR(EPDC_BASE, EPDC_TCE_POLARITY, reg_val);

	/* EPDC_IRQ_MASK */
	REG_WR(EPDC_BASE, EPDC_IRQ_MASK,
		EPDC_IRQ_TCE_UNDERRUN_IRQ);

	/*
	 * EPDC_GPIO
	 * PWRCOM = 1
	 * PWRCTRL = 0x110
	 * BDR = 0
	 */
	reg_val = ((0 << EPDC_GPIO_PWRCTRL_OFFSET) & EPDC_GPIO_PWRCTRL_MASK)
		| ((0 << EPDC_GPIO_BDR_OFFSET) & EPDC_GPIO_BDR_MASK);
	REG_WR(EPDC_BASE, EPDC_GPIO, reg_val);
}

static void draw_mode0(void)
{
	int i;
//	u32 val;

	/* Program EPDC update to process buffer */
	epdc_set_update_coord(0, 0);
	epdc_set_update_dimensions(panel_info.vl_col, panel_info.vl_row);
	epdc_submit_update(0, panel_info.epdc_data.wv_modes.mode_init,
				UPDATE_MODE_FULL, FALSE, 0, FALSE, 0);

	debug("Mode0 update - Waiting for LUT to complete...\n");

	/* Will timeout after ~4-5 seconds */

	for (i = 0; i < 40; i++) {
		if (!epdc_is_lut_active(0)) {
#if 0 
			val = REG_RD(EPDC_BASE, EPDC_IRQ_MASK);
			val |= (1<<0);
			REG_WR(EPDC_BASE, EPDC_IRQ_MASK, val);

			val = REG_RD(EPDC_BASE, EPDC_IRQ_CLR);
			val |= (1<<0);
			REG_WR(EPDC_BASE, EPDC_IRQ_CLR,val);
#endif
			debug("Mode0 init complete\n");
//			printf("%s\ti=%d\n",__func__,i);
			return;
		}
		msleep(100);
	}

	debug("Mode0 init failed!\n");

}

void temp_set_index(char temp)
{
	int i;
	int ntrt;
	if (gptWfmHeader && temp >= 0) {
		ntrt = gptWfmHeader[0x26];
		unsigned char *trt = gptWfmHeader + 0x30;

		for (i=0; i<ntrt; i++) {
			if (temp >= trt[i] && temp < trt[i+1]) {
				//printf("EPDC: temp = %d'C, index = %d\n", temp, i);
				REG_WR(EPDC_BASE, EPDC_TEMP, i);
				break;
			}
		}
		
		if(ntrt == 0){
			printf("NO trt entries, using default index!\n");
			REG_WR(EPDC_BASE, EPDC_TEMP, 0);
		}
	}	
	return ;
}

static void setup_waveform(void) 
{
	int len = 80;
	unsigned char *addr;
	int i;
	//char temp;
	unsigned char c;

	if (gptWfmAddr) {
		addr = gptWfmAddr;
	} else {
		addr = (unsigned char *)CONFIG_TEMP_INIT_WAVEFORM_ADDR;
		//printf("creating temporary waveform, %d steps\n", len);

		memset(addr, 0, 256);
		for (i=0x00; i<0x20; i+=8) *((u32 *)(addr+i)) = 0x20;
		for (i=0x20; i<0x90; i+=8) *((u32 *)(addr+i)) = 0x90;

		addr[0x90] = len;
		for (i=0; i<len; i++) {
			if (i < len/2) {
				c = 0x01;
			} else if (i == len/2 || i == len-1) {
				c = 0x00;
			} else {
				c = 0x02;
			}
			memset(addr+0x98+i*256, c, 256);
		}
	}
	//printf("EPDC: waveform is at %x\n", (unsigned int)addr);
	REG_WR(EPDC_BASE, EPDC_WVADDR, (unsigned long)addr);

	REG_WR(EPDC_BASE, EPDC_TEMP, TEMP_USE_DEFAULT);

	/* Set Working Buffer pointer */
	if (gptWbufAddr) {
		addr = gptWbufAddr;
	} else {
		addr = (unsigned char *) CONFIG_WORKING_BUF_ADDR;
	}
	//printf("EPDC: working buffer is at %x\n", (unsigned int)addr);
	REG_WR(EPDC_BASE, EPDC_WB_ADDR, (unsigned long)addr);

}

void epd_init_clean(void)
{

	if (!epdc_initialized) 
		return;

	setup_waveform();

	//memset(lcd_base, 0xFF, panel_info.vl_col * panel_info.vl_row);

	printf("EPD init clean!\n");
	//refresh display
	epdc_set_update_coord(0, 0);
	epdc_set_update_dimensions(panel_info.vl_col, panel_info.vl_row);
	epdc_submit_update(0, panel_info.epdc_data.wv_modes.mode_gc4,
				UPDATE_MODE_FULL, TRUE, 0xff, TRUE, 0xff);
}

static void draw_splash_screen(void)
{
//	int i;
	int lut_num = 1;

	/* Program EPDC update to process buffer */
	epdc_set_update_coord(0, 0);
	epdc_set_update_dimensions(panel_info.vl_col, panel_info.vl_row);
	epdc_submit_update(lut_num, panel_info.epdc_data.wv_modes.mode_gc16,
		UPDATE_MODE_FULL, FALSE, 0, FALSE, 0);

	//msleep(10);
#if 0
	for (i = 0; i < 40; i++) {
		if (!epdc_is_lut_active(lut_num)) {
			debug("Splash screen update complete\n");
			return;
		}
		msleep(100);
	}
	printf("Splash screen update failed!\n");
#endif

}


void epd_enable(void)
{
	char temp = TEMP_USE_DEFAULT;
	debug("epd_enable\n");
	
	//epdc_power_on();
	/* Draw data to display */
	//draw_mode0();

	/* Enable clock gating (clear to enable) */
	REG_CLR(EPDC_BASE, EPDC_CTRL, EPDC_CTRL_CLKGATE);
	while (REG_RD(EPDC_BASE, EPDC_CTRL) &
	       (EPDC_CTRL_SFTRST | EPDC_CTRL_CLKGATE))
		;

	REG_WR(EPDC_BASE, EPDC_TEMP, TEMP_USE_DEFAULT);
	temp = read_temperature();
	temp_set_index(temp);
	
	/* Set Waveform Bufferr register to real waveform address */
	REG_WR(EPDC_BASE, EPDC_WVADDR, panel_info.epdc_data.waveform_buf_addr);

	debug("epdc_irq's value %08x\nEPDC_IRQ_CLR's value %08x\n",REG_RD(EPDC_BASE, EPDC_IRQ), REG_RD(EPDC_BASE, EPDC_IRQ_CLR));
	debug("EPDC LUT STATUS %08x\n",REG_RD(EPDC_BASE,EPDC_STATUS_LUTS));
	
	draw_splash_screen();

	debug("epdc_irq's value %08x\nEPDC_IRQ_CLR's value %08x\n",REG_RD(EPDC_BASE, EPDC_IRQ), REG_RD(EPDC_BASE, EPDC_IRQ_CLR));
	debug("EPDC LUT STATUS %08x\n",REG_RD(EPDC_BASE,EPDC_STATUS_LUTS));
}

void epd_disable(void)
{
	debug("epd_disable\n");
	u32 val;
#if 1 
	/*clear the INT*/
	val = REG_RD(EPDC_BASE, EPDC_IRQ_MASK);
	val |= (1<<0);
	REG_WR(EPDC_BASE, EPDC_IRQ_MASK, val);

	val = REG_RD(EPDC_BASE, EPDC_IRQ_CLR);
	val |= (1<<0);
	REG_WR(EPDC_BASE, EPDC_IRQ_CLR,val);
#endif
	/* Disable clocks to EPDC */
	REG_SET(EPDC_BASE, EPDC_CTRL, EPDC_CTRL_CLKGATE);
	debug("EPDC LUT STATUS %08x\n",REG_RD(EPDC_BASE,EPDC_STATUS_LUTS));
}

void epd_panel_disable(void)
{
	epdc_power_off();
}

int epdc_ctrl_init(void *lcdbase)
{
	/*
	 * We rely on lcdbase being a physical address, i.e., either MMU off,
	 * or 1-to-1 mapping. Might want to add some virt2phys here.
	 */
	if (!lcdbase)
		return -1;

	eink_color_fg = 0xFF;
	eink_color_bg = 0xFF;

	/* Reset */
	REG_SET(EPDC_BASE, EPDC_CTRL, EPDC_CTRL_SFTRST);
	while (!(REG_RD(EPDC_BASE, EPDC_CTRL) & EPDC_CTRL_CLKGATE))
		;
	REG_CLR(EPDC_BASE, EPDC_CTRL, EPDC_CTRL_SFTRST);

	/* Enable clock gating (clear to enable) */
	REG_CLR(EPDC_BASE, EPDC_CTRL, EPDC_CTRL_CLKGATE);
	while (REG_RD(EPDC_BASE, EPDC_CTRL) &
	       (EPDC_CTRL_SFTRST | EPDC_CTRL_CLKGATE))
		;

	debug("resolution %dx%d, bpp %d\n", (int)panel_info.vl_col,
		(int)panel_info.vl_row, NBITS(panel_info.vl_bpix));


	/* Set framebuffer pointer */
	REG_WR(EPDC_BASE, EPDC_UPD_ADDR, (u32)lcdbase);

#if 0
	/* Set Working Buffer pointer */
	REG_WR(EPDC_BASE, EPDC_WB_ADDR, panel_info.epdc_data.working_buf_addr);
	/* Set Waveform Buffer pointer */
	REG_WR(EPDC_BASE, EPDC_WVADDR, panel_info.epdc_data.waveform_buf_addr);
#endif 

	/* Set waveform and working buffer, they will be changed later */
	REG_WR(EPDC_BASE, EPDC_WVADDR, (unsigned long)CONFIG_TEMP_INIT_WAVEFORM_ADDR);
	REG_WR(EPDC_BASE, EPDC_WB_ADDR, (unsigned long)CONFIG_WORKING_BUF_ADDR);
	
#if 0
	/* Get waveform data address and offset */
	int data_offs = setup_waveform_file();
	if(data_offs == -1) {
		printf("Can't load waveform data!\n");
		return -1;
	}
#endif

	/* Initialize EPDC, passing pointer to EPDC registers */
	epdc_init_settings();

	epdc_initialized = 1;

	return 0;
}

ulong calc_fbsize(void)
{
	return panel_info.vl_row * panel_info.vl_col * 2 \
		* NBITS(panel_info.vl_bpix) / 8;
}

