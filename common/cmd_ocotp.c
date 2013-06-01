/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 * Terry Lv <r65388@freescale.com>
 *
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
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
#include <command.h>
int otp_write(int, u32);
int otp_read(int);

int do_ocotp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;
	switch (argc) {
	case 0:
	case 1:
	case 2:
		     printf("Usage:\n%s\n", cmdtp->usage);
		     return 1;
	case 3:
        if (strcmp(argv[1], "read") == 0){
        	int index = simple_strtoul(argv[2], NULL, 16);
        	u32 data = otp_read(index);
          printf("ocotp read %d : 0x%x\n",index, data);
          return ret;}
          return 1;
	case 4:
	default: /* at least 3 args */
		if (strcmp(argv[1], "blow") == 0) {
						int index = simple_strtoul(argv[2], NULL, 16);
			      u32 data = simple_strtoul(argv[3], NULL, 16);
		        ret = otp_write(index,data);
		        if(ret !=0)
			      return 1;
			      return ret;}
			      return 1;
        }
}

U_BOOT_CMD(
	ocotp, 4, 1, do_ocotp,
	"ocotp sub system",
	"ocotp blow addr value--[addr]40 word address locations(0x00-0x27),refer OTP memory footprint\n"
	"ocotp read addr");


