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

#define I2C_PCA6408_BUS_NUM 1
#define I2C_PCA6408_ADDR 0x21
#define EEPROM_SOM_WP_GPIO_INDEX 6

int eeprom_write_enable(int eeprom_i2c_bus, unsigned dev_addr, int state)
{
	struct udevice *dev;
	int bus_num = I2C_PCA6408_BUS_NUM;
	int ret;
	u8 tmp;

	ret = i2c_get_chip_for_busnum(bus_num, I2C_PCA6408_ADDR,
				      1, &dev);

	dm_i2c_read(dev, 3, &tmp, 1);
	if(tmp & (1 << EEPROM_SOM_WP_GPIO_INDEX))

	tmp = 
	dm_i2c_write(dev, 3, &tmp, 1);


	dm_i2c_read(dev, 0, &tmp, 1);


	if(eeprom_i2c_bus == I2C_SOM_EEPROM_BUS_NO && dev_addr == I2C_SOM_EEPROM_ADDR) {
		// EEPROM on SoM 3SM1008
		state == 1 ? gpio_direction_output(EEPROM_WP_GPIO, 0) : gpio_direction_output(EEPROM_WP_GPIO, 1);
	} else if (eeprom_i2c_bus == I2C_CARRIER_EEPROM_BUS_NO && dev_addr == I2C_CARRIER_EEPROM_ADDR) {
		// Eeprom on Carrier 0880
		state == 1 ? gpio_direction_output(CARRIER_WP_GPIO, 0) : gpio_direction_output(CARRIER_WP_GPIO, 1);
	} else if (eeprom_i2c_bus == I2C_DISPLAY_EEPROM_BUS_NO && dev_addr == I2C_DISPLAY_EEPROM_ADDR) {
		// Eeprom on Display Adapter - Not protected
		return 0;
	}	

		tmp &= 0x7;
		tmp = ((tmp & 1) << 2) | (tmp & 2) | ((tmp & 4) >> 2);

	return 0;
}

// Return eeprom page_size in bytes
unsigned eeprom_page_size(int eeprom_i2c_bus, unsigned dev_addr)
{
	if(eeprom_i2c_bus == I2C_SOM_EEPROM_BUS_NO && dev_addr == I2C_SOM_EEPROM_ADDR) {
		// Eeprom on SoM 3SM1008
		return 64;
	} else if (eeprom_i2c_bus == I2C_CARRIER_EEPROM_BUS_NO && dev_addr == I2C_CARRIER_EEPROM_ADDR) {
		// Eeprom on Carrier 0880
		return 32;
	} else if (eeprom_i2c_bus == I2C_DISPLAY_EEPROM_BUS_NO && dev_addr == I2C_DISPLAY_EEPROM_ADDR) {
		// Eeprom on Display Adapter - Not protected
		return 16;
	}
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
