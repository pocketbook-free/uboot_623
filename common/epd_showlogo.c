/*
*********************************************
*	author:viola
*	version:v0.1
*	for reduce showing startup logo time
*
*********************************************
*/
/*
 * if you want to enable this macro, you should debug the show logo function with uboot command,
 *or it will be hang up.
 * 
 */
//#define DEBUG 1

#include <config.h>
#include <common.h>
#include <command.h>
#include <lcd.h>
#include <watchdog.h>

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
#define BMP_ALIGN_CENTER	0x7FFF
#endif

extern int setup_waveform_file();

ulong eink_setmem (ulong addr)
{
	ulong size;
	int line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;

	debug ("EINK panel info: %d x %d, %d bit/pix\n",
		panel_info.vl_col, panel_info.vl_row, NBITS (panel_info.vl_bpix) );

	size = line_length * panel_info.vl_row;

	/* Round up to nearest full page */
	size = (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

	/* Allocate pages for the frame buffer. */
	addr -= size;

	debug ("Reserving %ldk for EINK Framebuffer at: %08lx\n", size>>10, addr);

	return (addr);
}

/*----------------------------------------------------------------------*/

static void eink_setfgcolor (int color)
{
#if defined(CONFIG_ATMEL_LCD) || defined(CONFIG_MXC2_LCD)
		eink_color_fg = color;
#else
			eink_color_fg = color & 0x0F;
#endif
}

/*----------------------------------------------------------------------*/


static void eink_setbgcolor (int color)
{
#if defined(CONFIG_ATMEL_LCD) || defined(CONFIG_MXC2_LCD) || defined(CONFIG_VIDEO_MX5)
		eink_color_bg = color;
#else
			eink_color_bg = color & 0x0F;
#endif
}
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#ifdef	NOT_USED_SO_FAR
static int eink_getfgcolor (void)
{
		return eink_color_fg;
}
#endif	/* NOT_USED_SO_FAR */

/*----------------------------------------------------------------------*/

static int eink_getbgcolor (void)
{
		return eink_color_bg;
}

/*----------------------------------------------------------------------*/
static u8 cmap_to_grayscale(u8 blue, u8 green ,u8 red)
{
	u8 value;
	value = ((red * 77 + green * 151 + blue * 28) >> 8);
	//printf("value = %d\n", value);
	return value;
}

static int eink_display_bitmap(ulong bmp_image, int x, int y)
{
	debug("%s\n",__func__);
	ushort *cmap_base = NULL;
	ushort i, j;
	uchar *fb;
	uchar index;
	bmp_image_t *bmp=(bmp_image_t *)bmp_image;
	bmp_color_table_entry_t color_table[256];
	uchar *bmap;
	ushort padded_line;
	unsigned long width, height, byte_width;
	unsigned long pwidth = panel_info.vl_col;
	unsigned colors, bpix, bmp_bpix;
	unsigned long compression;
	static int flag = 0;

	if (!((bmp->header.signature[0]=='B') &&
		(bmp->header.signature[1]=='M'))) {
		printf ("Error: no valid bmp image at %lx\n", bmp_image);
		return 1;
	}

	width = le32_to_cpu (bmp->header.width);	//800
	height = le32_to_cpu (bmp->header.height);	//600
	bmp_bpix = le16_to_cpu(bmp->header.bit_count);	//8bit bmp picture
	colors = 1 << bmp_bpix;		//256
	compression = le32_to_cpu (bmp->header.compression);

	bpix = NBITS(panel_info.vl_bpix);	//8

	if ((bpix != 1) && (bpix != 8) && (bpix != 16) && (bpix != 24)) {
		printf ("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
			bpix, bmp_bpix);
		return 1;
	}

	/* We support displaying 8bpp BMPs on 16bpp LCDs */
	if (bpix != bmp_bpix && (bmp_bpix != 8 || bpix != 16)) {
		printf ("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
			bpix,
			le16_to_cpu(bmp->header.bit_count));
		return 1;
	}

	if(width != 1024 || height != 758){
		flag = 1;
/*	}
	else if(width == 758 && height == 1024){
		pwidth = panel_info.vl_row;
		flag = 0;
		}
		else{*/
		printf("Error:picture format %d*%d, program support 1024*758 8bit bmp picture\n",width,height);
		return 1;
		}
	debug ("Display-bmp: %d x %d  with %d colors\n",(int)width, (int)height, (int)colors);

	//store cmap
	memcpy(color_table, (uchar *)bmp + sizeof(bmp_header_t), 1024);
	/*
	printf("B...G...R\n");
	for (i = 0; i < 256 ; i++)
		printf("[%d] %d...%d...%d...\n", i, color_table[i].blue, color_table[i].green, color_table[i].red);
	*/

	/*
	 * BMP format for Monochrome assumes that the state of a
	 * pixel is described on a per Bit basis, not per Byte.
	 * So, in case of Monochrome BMP we should align widths
	 * on a byte boundary and convert them from Bit to Byte
	 * units.
	 * Probably, PXA250 and MPC823 process 1bpp BMP images in
	 * their own ways, so make the converting to be MCC200
	 * specific.
	 */
	padded_line = (width&0x3) ? ((width&~0x3)+4) : (width);	//800
#ifdef CONFIG_SPLASH_SCREEN_ALIGN
	if (x == BMP_ALIGN_CENTER)
		x = max(0, (pwidth - width) / 2);	//x=0
	else if (x < 0)
		x = max(0, pwidth - width + x + 1);

	if (y == BMP_ALIGN_CENTER)
		if(flag)
			y = max(0, (panel_info.vl_row - height) / 2);	//y=0
		else
			y = max(0, (panel_info.vl_col - width) / 2);	//y=0
	else if (y < 0)
		y = max(0, panel_info.vl_row - height + y + 1);
#endif /* CONFIG_SPLASH_SCREEN_ALIGN */

	if ((x + width)>pwidth)
		width = pwidth - x;
	if ((y + height)>panel_info.vl_row)
		height = panel_info.vl_row - y;
	bmap = (uchar *)bmp + le32_to_cpu (bmp->header.data_offset);
	fb   = (uchar *) (eink_base + (y + height - 1) * lcd_line_length + x * bpix / 8);

	debug("bmap 0x%0x,fb 0x%0x,x=%d,y=%d\n", bmap, fb, x, y);
	switch (bmp_bpix) {
	case 1: /* pass through */
	case 8:
		if (bpix != 16)
			byte_width = width;
		else
			byte_width = width * 2;
//		if(flag == 1){
		for (i = 0; i < height; ++i) {
			WATCHDOG_RESET();
//			printf("height %d\n",i);
			for (j = 0; j < width; j++) {
				if (bpix != 16) {
					index = *bmap++;
					*fb++ = cmap_to_grayscale(color_table[index].blue, \ 
							color_table[index].green, color_table[index].red);			
				} else {
					*(uint16_t *)fb = cmap_base[*(bmap++)];
					fb += sizeof(uint16_t) / sizeof(*fb);
				}
			}
			bmap += (width - padded_line);
			fb   -= (byte_width + lcd_line_length);
		}
/*		}
		else{
			int l,m;
			char bmp_img[800][600];
			for(l=0;l<800;l++)
				for(m=0; m<600;m++){
					bmp_img[l][m] = *bmap;
					bmap++;
				}
			printf("convert the bmp image for display bmap%0xbmp_img[0][0]%0x\n",*bmap,bmp_img[0][0]);
			for(l=0; l<width; l++){
				for(m=height-1; m>=0; m--){
					*fb = bmp_img[m][l] ;
					fb++;
				}
			}
		}*/
		debug("bmap %0x fb %0x*******************************\n", bmap, fb);
		break;

#if defined(CONFIG_BMP_16BPP)
	case 16:
		for (i = 0; i < height; ++i) {
			WATCHDOG_RESET();
			for (j = 0; j < width; j++) {
#if defined(CONFIG_ATMEL_LCD_BGR555)
				*(fb++) = ((bmap[0] & 0x1f) << 2) |
					(bmap[1] & 0x03);
				*(fb++) = (bmap[0] & 0xe0) |
					((bmap[1] & 0x7c) >> 2);
				bmap += 2;
#else
				*(fb++) = *(bmap++);
				*(fb++) = *(bmap++);
#endif
			}
			bmap += (padded_line - width) * 2;
			fb   -= (width * 2 + lcd_line_length);
		}
		break;
#endif /* CONFIG_BMP_16BPP */
#if defined(CONFIG_BMP_24BPP)
	case 24:
		if (bpix != 16) {
			printf("Error: %d bit/pixel mode,"
				"but BMP has %d bit/pixel\n",
				bpix, bmp_bpix);
			break;
		}
		for (i = 0; i < height; ++i) {
			WATCHDOG_RESET();
			for (j = 0; j < width; j++) {
				*(uint16_t *)fb = ((*(bmap + 2) << 8) & 0xf800)
						| ((*(bmap + 1) << 3) & 0x07e0)
						| ((*(bmap) >> 3) & 0x001f);
				bmap += 3;
				fb += sizeof(uint16_t) / sizeof(*fb);
			}
			bmap += (width - padded_line);
			fb   -= ((2*width) + lcd_line_length);
		}
		break;
#endif /* CONFIG_BMP_24BPP */
	default:
		break;
	};

	return (0);
}

static void *map_logo (void)
{
	debug("%s****************\n",__func__);
#ifdef CONFIG_SPLASH_SCREEN
	char *s;
	ulong addr;
//	static int do_splash = 1;
	debug(" env splashimage %0x\n", simple_strtol(getenv("splashimage"),NULL,0));
//	if (do_splash && (s = getenv("splashimage")) != NULL) {
	if ((s = getenv("splashimage")) != NULL) {
		int x = 0, y = 0;
//		do_splash = 0;

		addr = simple_strtoul (s, NULL, 16);

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
		if ((s = getenv ("splashpos")) != NULL) {
			if (s[0] == 'm')
				x = BMP_ALIGN_CENTER;
			else
				x = simple_strtol (s, NULL, 0);

			if ((s = strchr (s + 1, ',')) != NULL) {
				if (s[1] == 'm')
					y = BMP_ALIGN_CENTER;
				else
					y = simple_strtol (s + 1, NULL, 0);
			}
		}
#endif /* CONFIG_SPLASH_SCREEN_ALIGN */
		debug("addr %0x, x %d, y %d\n", addr, x, y);
		if (eink_display_bitmap (addr, x, y) == 0) {
			debug("%s******eink_base = 0x%08x\n",__func__,(unsigned int)eink_base);
			return ((void *)eink_base);
		}
		else
		{
			printf("eink_display_bitmap failed.\n");
			return NULL;
		}
	}
#endif /* CONFIG_SPLASH_SCREEN */


}

static int eink_clear (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{


#if LCD_BPP == LCD_COLOR8	//NO used
	/* Setting the palette */
	eink_setcolreg  (CONSOLE_COLOR_BLACK,       0,    0,    0);
	eink_setcolreg  (CONSOLE_COLOR_RED,	0xFF,    0,    0);
	eink_setcolreg  (CONSOLE_COLOR_GREEN,       0, 0xFF,    0);
	eink_setcolreg  (CONSOLE_COLOR_YELLOW,	0xFF, 0xFF,    0);
	eink_setcolreg  (CONSOLE_COLOR_BLUE,        0,    0, 0xFF);
	eink_setcolreg  (CONSOLE_COLOR_MAGENTA,	0xFF,    0, 0xFF);
	eink_setcolreg  (CONSOLE_COLOR_CYAN,	   0, 0xFF, 0xFF);
	eink_setcolreg  (CONSOLE_COLOR_GREY,	0xAA, 0xAA, 0xAA);
	eink_setcolreg  (CONSOLE_COLOR_WHITE,	0xFF, 0xFF, 0xFF);
#endif

#ifndef CONFIG_SYS_WHITE_ON_BLACK/*not defined the CONFIG_SYS_WHITE_ON_BLACK*/
	eink_setfgcolor (CONSOLE_COLOR_BLACK);	//set front color black background white
	eink_setbgcolor (CONSOLE_COLOR_WHITE);
#else
	eink_setfgcolor (CONSOLE_COLOR_WHITE);
	eink_setbgcolor (CONSOLE_COLOR_BLACK);
#endif	/* CONFIG_SYS_WHITE_ON_BLACK */

	/* set framebuffer to background color */
	memset ((char *)eink_base,
		COLOR_MASK(eink_getbgcolor()),
		lcd_line_length*panel_info.vl_row);

	/* Paint the logo and retrieve LCD base address */
	//printf ("[EINK] Drawing the logo...\n");
	eink_console_address = map_logo ();

	//if lcd_logo failed then return NULL
	if(eink_console_address == NULL)
		return -1;

	console_col = 0;
	console_row = 0;

	return (0);
}

int show_logo(int logo_num)
{
//	setup_waveform_file();
//	eink_base = (void *)(gd->fb_base);
	eink_base = CONFIG_FB_BASE;
	lcd_line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;
	
	if(	eink_clear(NULL, 1, 1, NULL) == -1){
		printf("failed to show_logo\n");
		return -1;
	}
	wait_for_update_complete();
	epdc_power_on();
	epd_enable();

	if (2 == logo_num)
	{
		printf("Show low battery logo!\n");
		wait_for_update_complete();
		epd_disable();
		epdc_power_off();
	}
	else 
	{
		printf("Show PocketBook logo!\n");
	}
	debug("end of show_logo\n");
	return 0;
}

int show2_logo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if(argc != 2)
		printf("%s\n", cmdtp->usage);

	if(*argv[1] == 0x31){
		setup_splash_img(1);
		show_logo(1);
		udelay(1000);
	}
	else if(*argv[1] == 0x32){
		setup_splash_img(2);
		show_logo(2);
	}
}

U_BOOT_CMD(
	startup_logo, 2, 0, show2_logo,
	"startup_logo under uboot",
	"startup_logo 2/1 \n 1:for startup logo 2:for low battery logo"
	);
