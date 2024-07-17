#ifndef GF_MUX_H_
#define GF_MUX_H_

#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>

#define EEPROM_WP_GPIO 42

#define I2C_SOM_EEPROM_BUS_NO	0
#define I2C_SOM_EEPROM_ADDR		0x50

#define I2C_CARRIER_EEPROM_BUS_NO	1
#define I2C_CARRIER_EEPROM_ADDR		0x54

void do_spl_pinmux(void);
void do_tpl_pinmux(void);
#endif
