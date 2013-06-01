#ifndef __PMIC_H__
#define __PMIC_H__
#include <config.h>

#define mdelay(n)  ({unsigned long msec=(n); while (msec--) udelay(1000);}) 



#define PAD_I2C1_SCL *(volatile unsigned long *)(0x53FA8040)
#define PAD_I2C1_SDA *(volatile unsigned long *)(0x53FA8044)

#define PULL_PAD_I2C1_SCL *(volatile unsigned long *)(0x53FA82EC)
#define PULL_PAD_I2C1_SDA *(volatile unsigned long *)(0x53FA82F0)

#define GPIO6_DR *(volatile unsigned long *)(0x53FE0000)
#define GPIO6_GDIR *(volatile unsigned long *)(0x53FE0004)

#define IIC_ESCL_Hi	GPIO6_DR |= (0x1<<18)
#define IIC_ESCL_Lo	GPIO6_DR &= ~(0x1<<18)
#define IIC_ESDA_Hi	GPIO6_DR |= (0x1<<19)
#define IIC_ESDA_Lo	GPIO6_DR &= ~(0x1<<19)

#define IIC_ESCL_INP	GPIO6_GDIR &= ~(0x1<<18)
#define IIC_ESCL_OUTP	GPIO6_GDIR = (GPIO6_GDIR & ~(0x1<<18))|(0x1<<18)

#define IIC_ESDA_INP	GPIO6_GDIR &= ~(0x1<<19)
#define IIC_ESDA_OUTP	GPIO6_GDIR = (GPIO6_GDIR & ~(0x1<<19))|(0x1<<19)

#define DELAY			100

#define MAX8698_ADDR	0x66	// when SRAD pin = 0, CC/CDh is selected

#define WM8994_ADDR 0x1A

extern void PMIC_InitIp(void);
char read_temperature(void);

#endif /*__PMIC_H__*/

