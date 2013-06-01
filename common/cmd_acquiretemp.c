#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/types.h>
#include <fp9928_pmic.h>


//int PMIC_IIC_ERead(unsigned int ChipId, unsigned int IicAddr);

/*static int fp9928_read()
{
	int reg_val;
//	ret = i2c_read(0x48, reg, 1, (uchar *)&reg_val, 1);
	reg_val = PMIC_IIC_ERead(0x48, 0x00);
	return reg_val;
}*/
char fp9928_read(unsigned char ChipId, u16 IicAddr);

static int do_acquire_temperature(cmd_tbl_t *cmdtp, int flag, int argc , char *argv[])
{
	if(argc != 1){
		cmd_usage(cmdtp);
		return 1;
	}
	
	char temperature, temperature1;

	PAD_I2C1_SCL =0x10;
	PAD_I2C1_SDA = 0x10;
	PULL_PAD_I2C1_SCL &= ~(0x1<<7);// Pull Up/Down Disable	SCL, SDA
	PULL_PAD_I2C1_SDA &= ~(0x1<<7);

	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	temperature1 = i2c_reg_read(0x48, 0x00);	//TMST_VALUE
	printf("current  enviroment temperature1 is %d\n",temperature1);

	temperature = fp9928_read(0x48,0x00);	//TMST_VALUE
	printf("current  enviroment temperature is %d\n",temperature);

	return 0;
}

U_BOOT_CMD(
		fp9928_temp, 1 , 0, do_acquire_temperature,
		"get the temperature from FP9928",
		"usage: fp9928_temp"
		);
