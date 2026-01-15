// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <efi_loader.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <linux/delay.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <spl.h>
#include <asm/mach-imx/dma.h>
#include <power/pmic.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <mmc.h>
#include "gf_mux.h"
#include "../common/gf_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;


#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.image_type_id = IMX_BOOT_IMAGE_GUID,
		.fw_name = u"IMX8MP-EVK-RAW",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 2=flash-bin raw 0 0x2000 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	set_wdog_reset(wdog);

	init_uart_clk(1);

	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	struct dram_timing_info *dram_timing;

	dram_timing = (struct dram_timing_info *) CONFIG_SAVED_DRAM_TIMING_BASE;

	*size = dram_timing->total_size;
	if (!size)
		return -EINVAL;

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

#ifdef CONFIG_USB_DWC3

#define USB_PHY_CTRL0			0xF0040
#define USB_PHY_CTRL0_REF_SSP_EN	BIT(2)

#define USB_PHY_CTRL1			0xF0044
#define USB_PHY_CTRL1_RESET		BIT(0)
#define USB_PHY_CTRL1_COMMONONN		BIT(1)
#define USB_PHY_CTRL1_ATERESET		BIT(3)
#define USB_PHY_CTRL1_VDATSRCENB0	BIT(19)
#define USB_PHY_CTRL1_VDATDETENB0	BIT(20)

#define USB_PHY_CTRL2			0xF0048
#define USB_PHY_CTRL2_TXENABLEN0	BIT(8)

#define USB_PHY_CTRL6			0xF0058

#define HSIO_GPR_BASE                               (0x32F10000U)
#define HSIO_GPR_REG_0                              (HSIO_GPR_BASE)
#define HSIO_GPR_REG_0_USB_CLOCK_MODULE_EN_SHIFT    (1)
#define HSIO_GPR_REG_0_USB_CLOCK_MODULE_EN          (0x1U << HSIO_GPR_REG_0_USB_CLOCK_MODULE_EN_SHIFT)


static struct dwc3_device dwc3_device_data = {
#ifdef CONFIG_SPL_BUILD
	.maximum_speed = USB_SPEED_HIGH,
#else
	.maximum_speed = USB_SPEED_SUPER,
#endif
	.base = USB1_BASE_ADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.power_down_scale = 2,
};

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	dwc3_uboot_handle_interrupt(dev);
	return 0;
}

static void dwc3_nxp_usb_phy_init(struct dwc3_device *dwc3)
{
	u32 RegData;

	/* enable usb clock via hsio gpr */
	RegData = readl(HSIO_GPR_REG_0);
	RegData |= HSIO_GPR_REG_0_USB_CLOCK_MODULE_EN;
	writel(RegData, HSIO_GPR_REG_0);

	/* USB3.0 PHY signal fsel for 100M ref */
	RegData = readl(dwc3->base + USB_PHY_CTRL0);
	RegData = (RegData & 0xfffff81f) | (0x2a<<5);
	writel(RegData, dwc3->base + USB_PHY_CTRL0);

	RegData = readl(dwc3->base + USB_PHY_CTRL6);
	RegData &=~0x1;
	writel(RegData, dwc3->base + USB_PHY_CTRL6);

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_VDATSRCENB0 | USB_PHY_CTRL1_VDATDETENB0 |
			USB_PHY_CTRL1_COMMONONN);
	RegData |= USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET;
	writel(RegData, dwc3->base + USB_PHY_CTRL1);

	RegData = readl(dwc3->base + USB_PHY_CTRL0);
	RegData |= USB_PHY_CTRL0_REF_SSP_EN;
	writel(RegData, dwc3->base + USB_PHY_CTRL0);

	RegData = readl(dwc3->base + USB_PHY_CTRL2);
	RegData |= USB_PHY_CTRL2_TXENABLEN0;
	writel(RegData, dwc3->base + USB_PHY_CTRL2);

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET);
	writel(RegData, dwc3->base + USB_PHY_CTRL1);
}
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)
#define USB2_PWR_EN IMX_GPIO_NR(1, 14)
int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;

	if (index == 0 && init == USB_INIT_DEVICE) {
		imx8m_usb_power(index, true);
		dwc3_nxp_usb_phy_init(&dwc3_device_data);
		return dwc3_uboot_init(&dwc3_device_data);
	} else if (index == 0 && init == USB_INIT_HOST) {
		return ret;
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	int ret = 0;
	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_uboot_exit(index);
		imx8m_usb_power(index, false);
	} else if (index == 0 && init == USB_INIT_HOST) {
	}

	return ret;
}

#endif



int eeprom_write_enable(int eeprom_i2c_bus, unsigned dev_addr, int state)
{
	if(eeprom_i2c_bus == I2C_SOM_EEPROM_BUS_NO && dev_addr == I2C_SOM_EEPROM_ADDR) {
		// Eeprom on SoM 3SM2008
		state == 1 ? gpio_direction_output(EEPROM_WP_GPIO, 0) : gpio_direction_output(EEPROM_WP_GPIO, 1);
	} 
	// else if (eeprom_i2c_bus == I2C_CARRIER_EEPROM_BUS_NO && dev_addr == I2C_CARRIER_EEPROM_ADDR) {
	// 	// Eeprom on Carrier 0890
	// 	state == 1 ? gpio_direction_output(CARRIER_WP_GPIO, 0) : gpio_direction_output(CARRIER_WP_GPIO, 1);
	// } 
	return 0;
}

// Return eeprom page_size in bytes
unsigned eeprom_page_size(int eeprom_i2c_bus, unsigned dev_addr)
{
	if(eeprom_i2c_bus == I2C_SOM_EEPROM_BUS_NO && dev_addr == I2C_SOM_EEPROM_ADDR) {
		// Eeprom on SoM 3SM2008
		return 64;
	} 
	else if (eeprom_i2c_bus == I2C_CARRIER_EEPROM_BUS_NO && dev_addr == I2C_CARRIER_EEPROM_ADDR) {
		// Eeprom on Carrier 0890
		return 32;
	} 
	//else if (eeprom_i2c_bus == I2C_DISPLAY_EEPROM_BUS_NO && dev_addr == I2C_DISPLAY_EEPROM_ADDR) {
	// 	// Eeprom on Display Adapter - Not protected
	// 	return 16;
	//}
	return (1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS);
}


int board_init(void)
{

	do_tpl_pinmux();

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)
	init_usb_clk();
#endif

	return 0;
}

int board_late_init(void)
{
	char dts_name[100];
	char mac_address_1[18];
	char mac_address_2[18];
	int ret;

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "eGF 3SM2008");
	env_set("board_rev", "iMX8MP");
#endif
	printf("Power on PHY\n");
	gpio_request(EQOS_PHY_RESET,"EQOS_PHY_RESET");
	gpio_direction_output(EQOS_PHY_RESET, 0); // Switch on PHY. Important to let kernel probe it.


	gf_init_som_eeprom(I2C_SOM_EEPROM_BUS_NO, I2C_SOM_EEPROM_ADDR);
	gf_init_carrier_eeprom(I2C_CARRIER_EEPROM_BUS_NO, I2C_CARRIER_EEPROM_ADDR);

	/* Get DTS to load name from EEPROM */
	ret = gf_get_dts_name(dts_name);

	if (ret == TRUE)
	{
		env_set("fdtfile", strcat(dts_name, ".dtb"));
	}

	/* Get SoM MAC address from EEPROM */
	ret = gf_get_mac_address_1(mac_address_1);

	if (ret == TRUE)
	{
		env_set("ethaddr", mac_address_1);
	}

	/* Get SoM MAC address 2 from EEPROM */
	ret = gf_get_mac_address_2(mac_address_2);

	if (ret == TRUE)
	{
		env_set("eth1addr", mac_address_2);
	}


	return 0;
}

#ifdef CONFIG_SPL_MMC
#define UBOOT_RAW_SECTOR_OFFSET 0x40
unsigned long spl_mmc_get_uboot_raw_sector(struct mmc *mmc, unsigned long raw_sect)
{
	u32 boot_dev = spl_boot_device();
	switch (boot_dev) {
		case BOOT_DEVICE_MMC2:
			return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR - UBOOT_RAW_SECTOR_OFFSET;
		default:
			return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
	}
}
#endif
