#include "gf_mux.h"

#define UART_PAD_CTRL	(PAD_CTL_DSE(6) | PAD_CTL_FSEL2)

static iomux_v3_cfg_t const lpuart3_pads[] = {
	MX93_PAD_GPIO_IO15__LPUART3_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX93_PAD_GPIO_IO14__LPUART3_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};


static void my_debug_uart_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(lpuart3_pads, ARRAY_SIZE(lpuart3_pads));
}

void do_spl_pinmux(void)
{
	my_debug_uart_init_mux();
}

void do_tpl_pinmux(void)
{
}
