#include "gf_mux.h"

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define GPIO_PAD_CTRL	(PAD_CTL_DSE6)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE)
#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE | \
			 	 	 	 PAD_CTL_FSEL2)
#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS)

struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = IMX8MM_PAD_I2C1_SCL_I2C1_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = IMX8MM_PAD_I2C1_SCL_GPIO5_IO14 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MM_PAD_I2C1_SDA_I2C1_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = IMX8MM_PAD_I2C1_SDA_GPIO5_IO15 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 15),
	},
};

static iomux_v3_cfg_t const uart1_pads[] = {
	IMX8MM_PAD_SAI2_RXC_UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MM_PAD_SAI2_RXFS_UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MM_PAD_GPIO1_IO02_WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IMX8MM_PAD_NAND_WE_B_USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_WP_B_USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA04_USDHC3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA05_USDHC3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA06_USDHC3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA07_USDHC3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_RE_B_USDHC3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CE2_B_USDHC3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CE3_B_USDHC3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CLE_USDHC3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CE1_B_USDHC3_STROBE | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const fec1_rst_pads[] = {
	IMX8MM_PAD_GPIO1_IO11_GPIO1_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const power_enable_pads[] = {
	IMX8MM_PAD_SD1_DATA4_GPIO2_IO6  	| MUX_PAD_CTRL(GPIO_PAD_CTRL), /* PWR_5V0_EN */
	IMX8MM_PAD_SAI5_RXFS_GPIO3_IO19  	| MUX_PAD_CTRL(GPIO_PAD_CTRL), /* VIO_3V3_EN */
};

static iomux_v3_cfg_t const eeprom_pads[] = {
	IMX8MM_PAD_GPIO1_IO00_GPIO1_IO0 | MUX_PAD_CTRL(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE),
};

static iomux_v3_cfg_t const usb_pads[] = {
	IMX8MM_PAD_GPIO1_IO12_GPIO1_IO12 | MUX_PAD_CTRL(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE),
	IMX8MM_PAD_SD1_STROBE_GPIO2_IO11 | MUX_PAD_CTRL(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE),
};


static void my_i2c1_init_mux(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
}

static void my_emmc_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
}

static void my_debug_uart_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static void my_watchdog_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
}

static void my_enet_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(fec1_rst_pads, ARRAY_SIZE(fec1_rst_pads));
}

static void my_power_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(power_enable_pads, ARRAY_SIZE(power_enable_pads));
	/* Enable board power supplies */
	gpio_direction_output(IMX_GPIO_NR(2, 6), 1);
	gpio_direction_output(IMX_GPIO_NR(3, 19), 1);
}

static void my_eeprom_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(eeprom_pads, ARRAY_SIZE(eeprom_pads));
	gpio_request(EEPROM_WP_GPIO, "EEPROM WP");
}

static void my_usb_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(usb_pads, ARRAY_SIZE(usb_pads));
	gpio_request(USB2_PWREN_GPIO, "USB2 PWR EN");
	gpio_request(USB1_PWREN_GPIO, "USB1 PWR EN");
}


void do_spl_pinmux(void)
{
	my_power_init_mux();
	my_i2c1_init_mux();
	my_watchdog_init_mux();
	my_debug_uart_init_mux();
	my_emmc_init_mux();
}

void do_tpl_pinmux(void)
{
	my_eeprom_init_mux();
	my_enet_init_mux();
	my_usb_init_mux();
}
