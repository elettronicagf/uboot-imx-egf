// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */
#include <common.h>
#include <efi_loader.h>
#include <env.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <i2c.h>
#include <asm/io.h>
#include <usb.h>
#include <linux/delay.h>
#include "gf_mux.h"
#include "../common/gf_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.image_type_id = IMX_BOOT_IMAGE_GUID,
		.fw_name = u"IMX8MM-EVK-RAW",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 2=flash-bin raw 0x42 0x2000 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;
	set_wdog_reset(wdog);

	init_uart_clk(3);

	return 0;
}


int board_phys_sdram_size(phys_size_t *size)
{
	struct dram_timing_info *dram_timing;

	dram_timing = (struct dram_timing_info *) CONFIG_SAVED_DRAM_TIMING_BASE;

	if (!size)
		return -EINVAL;

	*size = dram_timing->total_size;
	return 0;
}


int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;

	printf("board_usb_init %d, type %d\n", index, init);

	imx8m_usb_power(index, true);

	gpio_direction_output(USB2_PWREN_GPIO, 1);
	gpio_direction_output(USB1_PWREN_GPIO, 1);

	return ret;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	int ret = 0;

	printf("board_usb_cleanup %d, type %d\n", index, init);

	imx8m_usb_power(index, false);

	gpio_direction_output(USB2_PWREN_GPIO, 0);
	gpio_direction_output(USB1_PWREN_GPIO, 0);

	return ret;
}

#if IS_ENABLED(CONFIG_FEC_MXC)
#define FEC_RST_PAD IMX_GPIO_NR(1, 11)
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	gpio_request(FEC_RST_PAD, "fec1_rst");
	gpio_direction_output(FEC_RST_PAD, 0);
	udelay(60000);
	gpio_direction_output(FEC_RST_PAD, 1);
	udelay(70000);		
	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	// int ret;
	// phydev->addr = 1;
	// /* Disabled 1000Base-T advertising as HW does not support it */
	// phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x80C);

	// ret = phy_read(phydev, MDIO_DEVAD_NONE, 0x00);
	// ret = ret | (0x1 << 15);
	// phy_write(phydev, MDIO_DEVAD_NONE, 0x00, ret);

	// phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x90C);

	// if (phydev->drv->config)
	// 	phydev->drv->config(phydev);



	if (phydev->drv->config)
		phydev->drv->config(phydev);

#ifndef CONFIG_DM_ETH
	/* enable rgmii rxc skew and phy mode select to RGMII copper */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);
#endif
	return 0;
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

int eeprom_write_enable(int eeprom_i2c_bus, unsigned dev_addr, int state)
{
	if(eeprom_i2c_bus == I2C_SOM_EEPROM_BUS_NO && dev_addr == I2C_SOM_EEPROM_ADDR) {
		// EEPROM on SoM 3SM2010
		state == 1 ? gpio_direction_output(EEPROM_WP_GPIO, 0) : gpio_direction_output(EEPROM_WP_GPIO, 1);
	} else if (eeprom_i2c_bus == I2C_CARRIER_EEPROM_BUS_NO && dev_addr == I2C_CARRIER_EEPROM_ADDR) {
		// Eeprom on Carrier
		state == 1 ? gpio_direction_output(CARRIER_WP_GPIO, 0) : gpio_direction_output(CARRIER_WP_GPIO, 1);
	} else if (eeprom_i2c_bus == I2C_DISPLAY_EEPROM_BUS_NO && dev_addr == I2C_DISPLAY_EEPROM_ADDR) {
		// Eeprom on Display Adapter - Not protected
		return 0;
	}		
	return 0;
}

// Return eeprom page_size in bytes
unsigned eeprom_page_size(int eeprom_i2c_bus, unsigned dev_addr)
{
	if(eeprom_i2c_bus == I2C_SOM_EEPROM_BUS_NO && dev_addr == I2C_SOM_EEPROM_ADDR) {
		// Eeprom on SoM 3SM2010
		return 64;
	} else if (eeprom_i2c_bus == I2C_CARRIER_EEPROM_BUS_NO && dev_addr == I2C_CARRIER_EEPROM_ADDR) {
		// Eeprom on Carrier
		return 32;
	} else if (eeprom_i2c_bus == I2C_DISPLAY_EEPROM_BUS_NO && dev_addr == I2C_DISPLAY_EEPROM_ADDR) {
		// Eeprom on Display Adapter - Not protected
		return 16;
	}
	return (1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS);
}


int board_init(void)
{
	do_tpl_pinmux();
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();
	return 0;
}

int board_late_init(void)
{
	char dts_name[100];
	char mac_address[18];
	int ret;

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", "eGF 3SM1008");
		env_set("board_rev", "iMX8MM");
	}

	gf_init_som_eeprom(I2C_SOM_EEPROM_BUS_NO, I2C_SOM_EEPROM_ADDR);
	gf_init_carrier_eeprom(I2C_CARRIER_EEPROM_BUS_NO, I2C_CARRIER_EEPROM_ADDR);

	/* Get DTS to load name from EEPROM */
	ret = gf_get_dts_name(dts_name);

	if (ret == TRUE)
	{
		env_set("fdtfile", strcat(dts_name, ".dtb"));
	}

	/* Get SoM MAC address from EEPROM */
	ret = gf_get_mac_address_1(mac_address);

	if (ret == TRUE)
	{
		env_set("ethaddr", mac_address);
	}
	return 0;
}
