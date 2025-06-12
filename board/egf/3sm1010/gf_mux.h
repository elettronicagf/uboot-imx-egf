#ifndef GF_MUX_H_
#define GF_MUX_H_

#include <common.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch-imx9/imx93_pins.h>

#define EEPROM_SOM_WP_GPIO_INDEX 6
#define EEPROM_CARRIER_WP_GPIO_INDEX 5
#define EEPROM_CARRIER_WP_GPIO_PORT 0

#define I2C_SOM_EEPROM_BUS_NO	0
#define I2C_SOM_EEPROM_ADDR		0x50

#define I2C_CARRIER_EEPROM_BUS_NO	1
#define I2C_CARRIER_EEPROM_ADDR		0x54

#define I2C_DISPLAY_EEPROM_BUS_NO	7
#define I2C_DISPLAY_EEPROM_ADDR		0x56

void do_spl_pinmux(void);
void do_tpl_pinmux(void);
#endif
