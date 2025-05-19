// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <env.h>
#include <efi_loader.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/arch-imx9/ccm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/arch/clock.h>
#include <power/pmic.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <asm/gpio.h>
#include "../common/gf_eeprom.h"
#include "gf_mux.h"

#define errf(err) do { printf("ERROR %d @ %s() line %d: ", err, __func__, __LINE__);} while (0)
#define HANDLE_ERR(err) {if(err) {errf(err);return err;}}

DECLARE_GLOBAL_DATA_PTR;


#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
#define IMX_BOOT_IMAGE_GUID \
	EFI_GUID(0xbc550d86, 0xda26, 0x4b70, 0xac, 0x05, \
		 0x2a, 0x44, 0x8e, 0xda, 0x6f, 0x21)

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = IMX_BOOT_IMAGE_GUID,
		.fw_name = u"IMX93-11X11-EVK-RAW",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 0=flash-bin raw 0 0x2000 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{

	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	/* Read SDRAM size from the RAM address where it was written by SPL */
	u64 * total_dram_size = (u64 *) CONFIG_SAVED_DRAM_SIZE_BASE;
	*size = *total_dram_size;
	return 0;
}

static int setup_fec(void)
{
	return set_clk_enet(ENET_125MHZ);
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#define I2C_TCA6408_BUS_NUM 0
#define I2C_TCA6408_ADDR 0x21
#define I2C_TCA6408_INPUTPORT_REG 0
#define I2C_TCA6408_OUTPUTPORT_REG 1
#define I2C_TCA6408_INVERSION_REG 2
#define I2C_TCA6408_CONFIGURATION_REG 3

#define I2C_PCAL6416_BUS_NUM 0
#define I2C_PCAL6416_ADDR 0x20
#define I2C_PCAL6416_INPUTPORT_0_REG 0
#define I2C_PCAL6416_INPUTPORT_1_REG 1
#define I2C_PCAL6416_OUTPUTPORT_0_REG 2
#define I2C_PCAL6416_OUTPUTPORT_1_REG 3
#define I2C_PCAL6416_CONFIGURATION_0_REG 6
#define I2C_PCAL6416_CONFIGURATION_1_REG 7

int set_pcal6416_gpio(int gpio_port, int gpio_num, int value)
{
	struct udevice *dev;
	int bus_num = I2C_PCAL6416_BUS_NUM;
	int ret, output_port_reg, configuration_reg;
	u8 tmp_config, tmp_output;
	u8 gpio_bit_mask = (1 << (gpio_num));
	ret = i2c_get_chip_for_busnum(bus_num, I2C_PCAL6416_ADDR, 1, &dev);
	if(ret) {
		errf(ret);
		return ret;
	}

	if(gpio_port==0){
		output_port_reg = I2C_PCAL6416_OUTPUTPORT_0_REG;
		configuration_reg = I2C_PCAL6416_CONFIGURATION_0_REG;

	} else {
		output_port_reg = I2C_PCAL6416_OUTPUTPORT_1_REG;
		configuration_reg = I2C_PCAL6416_CONFIGURATION_1_REG;
	}
	
	//Set GPIO as output
	ret = dm_i2c_read(dev, configuration_reg, &tmp_config, 1);
	if(ret) {
		errf(ret);
		return ret;
	}
	tmp_config &= ~gpio_bit_mask;
	ret = dm_i2c_write(dev, configuration_reg, &tmp_config, 1);
	if(ret) {
		errf(ret);
		return ret;
	}

	//Set pin value
	ret = dm_i2c_read(dev, output_port_reg, &tmp_output, 1);
	if(ret) {
		errf(ret);
		return ret;
	}
	if(value == 0){
		tmp_output &= ~gpio_bit_mask;
	} else {
		tmp_output |= gpio_bit_mask;
	}
	ret = dm_i2c_write(dev, output_port_reg, &tmp_output, 1);
	if(ret) {
		errf(ret);
		return ret;
	}

	return 0;
}

int set_tca6408_gpio(int gpio_num, int value)
{
	struct udevice *dev;
	int bus_num = I2C_TCA6408_BUS_NUM;
	int ret;
	u8 tmp_config, tmp_output;
	u8 gpio_bit_mask = (1 << (gpio_num));
	ret = i2c_get_chip_for_busnum(bus_num, I2C_TCA6408_ADDR, 1, &dev);
	if(ret) {
		errf(ret);
		return ret;
	}

	//Set GPIO as output
	ret = dm_i2c_read(dev, I2C_TCA6408_CONFIGURATION_REG, &tmp_config, 1);
	if(ret) {
		errf(ret);
		return ret;
	}
	tmp_config &= ~gpio_bit_mask;
	ret = dm_i2c_write(dev, I2C_TCA6408_CONFIGURATION_REG, &tmp_config, 1);
	if(ret) {
		errf(ret);
		return ret;
	}

	//Set pin value
	ret = dm_i2c_read(dev, I2C_TCA6408_OUTPUTPORT_REG, &tmp_output, 1);
	if(ret) {
		errf(ret);
		return ret;
	}
	if(value == 0){
		tmp_output &= ~gpio_bit_mask;
	} else {
		tmp_output |= gpio_bit_mask;
	}
	ret = dm_i2c_write(dev, I2C_TCA6408_OUTPUTPORT_REG, &tmp_output, 1);
	if(ret) {
		errf(ret);
		return ret;
	}

	return 0;
}

int eeprom_write_enable(int eeprom_i2c_bus, unsigned dev_addr, int state)
{
	printf("\nState %d: ", state);
	state == 1 ? printf("Unlocking eeprom\n") : printf("Locking eeprom\n");

	if(eeprom_i2c_bus == I2C_SOM_EEPROM_BUS_NO && dev_addr == I2C_SOM_EEPROM_ADDR) {
		// EEPROM on SoM 3SM1008
		state == 1 ? set_tca6408_gpio(EEPROM_SOM_WP_GPIO_INDEX, 0) : set_tca6408_gpio(EEPROM_SOM_WP_GPIO_INDEX, 1);
	} /*else if (eeprom_i2c_bus == I2C_CARRIER_EEPROM_BUS_NO && dev_addr == I2C_CARRIER_EEPROM_ADDR) {
		// Eeprom on Carrier 0880
		state == 1 ? gpio_direction_output(CARRIER_WP_GPIO, 0) : gpio_direction_output(CARRIER_WP_GPIO, 1);
	} else if (eeprom_i2c_bus == I2C_DISPLAY_EEPROM_BUS_NO && dev_addr == I2C_DISPLAY_EEPROM_ADDR) {
		// Eeprom on Display Adapter - Not protected
		return 0;
	}	
*/
	return 0;
}

// Return eeprom page_size in bytes
unsigned eeprom_page_size(int eeprom_i2c_bus, unsigned dev_addr)
{
	if(eeprom_i2c_bus == I2C_SOM_EEPROM_BUS_NO && dev_addr == I2C_SOM_EEPROM_ADDR) {
		// Eeprom on SoM 3SM1008
		return 32;
	} /*else if (eeprom_i2c_bus == I2C_CARRIER_EEPROM_BUS_NO && dev_addr == I2C_CARRIER_EEPROM_ADDR) {
		// Eeprom on Carrier 0880
		return 32;
	} else if (eeprom_i2c_bus == I2C_DISPLAY_EEPROM_BUS_NO && dev_addr == I2C_DISPLAY_EEPROM_ADDR) {
		// Eeprom on Display Adapter - Not protected
		return 16;
	}*/
	return (1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS);
}



int board_init(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "11X11_EVK");
	env_set("board_rev", "iMX93");
#endif
	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY
int is_recovery_key_pressing(void)
{
	return 0;
}
#endif /*CONFIG_ANDROID_RECOVERY*/
#endif /*CONFIG_FSL_FASTBOOT*/
