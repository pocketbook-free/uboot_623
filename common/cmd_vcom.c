#include <common.h>
#include <command.h>

#include <asm/arch/mx50.h>
#include <asm/arch/mx50_pins.h>
#include <asm/arch/iomux.h>
#define MC13892_REG_POWER_CTL0                  13

int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        struct spi_slave *slave;
        unsigned int val;

        slave = spi_pmic_probe();

        val = pmic_reg(slave, MC13892_REG_POWER_CTL0, 0, 0);
        val |= 0x8;
        pmic_reg(slave,  MC13892_REG_POWER_CTL0, val, 1);
        printf("system shut down\n");
        udelay(1000000);
        return 0;
}

int do_vcom(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int size, uV, ret = 0;
	long int def;
	char *buffer = NULL;
	switch (argc) {
	case 0:
	case 1:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	case 2:
		if(!strcmp(argv[1], "read")) {
			size = 8;
			char *tmp = "VCOM";

			buffer = (char *)malloc(size);
			if(NULL == buffer)
				return -1;
			memset(buffer, 0x0, size);

			ret = read_device(buffer, 0x0, size);
			if(buffer != NULL) {
				*(buffer+size) = '\0';
				def = simple_strtol(buffer, NULL, 0);
			}

			if(def > -302000 || def < -4000000){
				puts("use the default vcom!\n");
				setenv("vcom", "-1250000");
				free(buffer);
				return -1;
			}
			else {
				setenv("vcom", buffer);
				free(buffer);
				return 0;
			}
		}
	
		else if(!strcmp(argv[1], "clear")) {
			policy_format();
			puts("clear over!\n");	
			return 0;
		}
	
	case 3:
		if(!strcmp(argv[1], "write")) {
			size = strlen(argv[2]);
			uV = simple_strtol(argv[2], NULL, 0);
			
			if(uV > -302000 || uV < -4000000){
				printf("the vcom value should be -4000000<value<-302000");
				return -1;
			}

			printf("the size is:%d\n", size);

			buffer = (char *)malloc(size+1);
			if(NULL == buffer)
				return -1;
			memset(buffer, 0, size+1);
			memcpy(buffer, argv[2], size);
			printf("input:%s\n", buffer);

			*(buffer+size) = '\0';
			
			write_device(buffer, 0x0, size+1);

			if(ret < 0) {
				puts("setup fail!\n");
				puts("use the default vcom!\n");
				setenv("vcom", "-1250000");
				free(buffer);
				return -1;
			}
			else {
				puts("setup success!\n");
				setenv("vcom", buffer);
				printf("vcom = %s\n", buffer);
				printf("vcom = complete\n");
				free(buffer);
				return 0;	
			}
		}
	}	
}

U_BOOT_CMD(
	vcom, 3, 0, do_vcom,
	"vcom read or write",
	"vcom read\n"
	"vcom write voltage");

U_BOOT_CMD(
        poweroff, 1, 0, do_poweroff,
        "PMIC close system power",
        ""
);

