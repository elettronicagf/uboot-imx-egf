#ifndef GF_MUX_H_
#define GF_MUX_H_

#include <common.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch-imx9/imx93_pins.h>

#define EEPROM_WP_GPIO 42
#define CARRIER_WP_GPIO 123

#define I2C_SOM_EEPROM_BUS_NO	0
#define I2C_SOM_EEPROM_ADDR		0x50

#define I2C_CARRIER_EEPROM_BUS_NO	1
#define I2C_CARRIER_EEPROM_ADDR		0x54

#define I2C_DISPLAY_EEPROM_BUS_NO	3
#define I2C_DISPLAY_EEPROM_ADDR		0x56

void do_spl_pinmux(void);
void do_tpl_pinmux(void);
#endif
