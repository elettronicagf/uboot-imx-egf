#include "gf_mux.h"

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

#define GPIO_PAD_CTRL	(PAD_CTL_DSE6)
#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE | \
			 	 	 	 PAD_CTL_FSEL2)
#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS)
#define QSPI_PAD_CTRL (PAD_CTL_PE | PAD_CTL_PUE | PAD_CTL_FSEL2 | PAD_CTL_DSE6)

struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX8MP_PAD_I2C1_SCL__I2C1_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX8MP_PAD_I2C1_SCL__GPIO5_IO14 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = MX8MP_PAD_I2C1_SDA__I2C1_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX8MP_PAD_I2C1_SDA__GPIO5_IO15 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 15),
	},
};

static iomux_v3_cfg_t const uart2_pads[] = {
	MX8MP_PAD_UART2_RXD__UART2_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_UART2_TXD__UART2_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX8MP_PAD_NAND_WE_B__USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_WP_B__USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA04__USDHC3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA05__USDHC3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA06__USDHC3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA07__USDHC3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_RE_B__USDHC3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CE2_B__USDHC3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CE3_B__USDHC3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CLE__USDHC3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CE1_B__USDHC3_STROBE | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_READY_B__USDHC3_RESET_B | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const eqos_phy_rst_pads[] = {
	MX8MP_PAD_SAI1_RXC__GPIO4_IO01 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const eeprom_pads[] = {
	MX8MP_PAD_SD1_RESET_B__GPIO2_IO10 | MUX_PAD_CTRL(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE),
	MX8MP_PAD_SAI2_MCLK__GPIO4_IO27 | MUX_PAD_CTRL(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE),
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
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

static void my_watchdog_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
}

static void my_enet_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(eqos_phy_rst_pads, ARRAY_SIZE(eqos_phy_rst_pads));
}

static void my_eeprom_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(eeprom_pads, ARRAY_SIZE(eeprom_pads));
	gpio_request(EEPROM_WP_GPIO, "EEPROM WP");
	//gpio_request(CARRIER_WP_GPIO, "CARRIER EEPROM WP");
}

void do_spl_pinmux(void)
{
	my_i2c1_init_mux();
	my_watchdog_init_mux();
	my_debug_uart_init_mux();
	my_emmc_init_mux();
}

void do_tpl_pinmux(void)
{
	my_eeprom_init_mux();
	my_enet_init_mux();
}
