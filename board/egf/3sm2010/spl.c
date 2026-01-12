// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019, 2021 NXP
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>
#include <asm/sections.h>

#include <power/pmic.h>
#ifdef CONFIG_POWER_PCA9450
#include <power/pca9450.h>
#else
#include <power/bd71837.h>
#endif
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <linux/delay.h>
#include <fsl_sec.h>
#include "../common/gf_eeprom.h"
#include "gf_mux.h"

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC1;
	default:
		return BOOT_DEVICE_NONE;
	}
}

void spl_dram_init(void)
{
#ifndef CONFIG_MFG_EMB_DDR
	int ret;
	struct dram_timing_info my_timings;
	ret = gf_read_dram_timings_mx8m(&my_timings);
	if (ret == FALSE)
	{
		printf("Fatal error while loading DDR timings from ROM\n");
		printf("Stopping boot\n");
		hang();
	}
	else
	{
		printf("Using GF ROM DDR timings\n");
		ddr_init(&my_timings);
	}
	ddr_init(&my_timings);
#else
	printf("Using integrated timings for forced WID\n");
	ddr_init(&dram_timing);
#endif
}


static struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{USDHC3_BASE_ADDR, 0, 8},
};

int board_mmc_init(struct bd_info *bis)
{
	int ret;
	init_clk_usdhc(2);
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	ret = fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
	if (ret)
		return ret;
	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC3_BASE_ADDR:
		ret = 1;
		break;
	}

	return ret;
}

#if CONFIG_IS_ENABLED(POWER_LEGACY)
#define I2C_PMIC	0
#ifdef CONFIG_POWER_PCA9450
int power_init_board(void)
{
	struct pmic *p;
	int ret;

	ret = power_pca9450_init(I2C_PMIC, 0x25);
	if (ret)
		printf("power init failed");
	p = pmic_get("PCA9450");
	pmic_probe(p);

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(p, PCA9450_BUCK123_DVS, 0x29);

	/* Buck 1 DVS control through PMIC_STBY_REQ */
	pmic_reg_write(p, PCA9450_BUCK1CTRL, 0x59);

	/* Set DVS1 to 0.8v for suspend */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS1, 0x10);

	/* increase VDD_DRAM to 0.95v for 3Ghz DDR */
	pmic_reg_write(p, PCA9450_BUCK3OUT_DVS0, 0x1C);

	/* VDD_DRAM needs off in suspend, set B1_ENMODE=10 (ON by PMIC_ON_REQ = H && PMIC_STBY_REQ = L) */
	pmic_reg_write(p, PCA9450_BUCK3CTRL, 0x4a);

	/* set VDD_SNVS_0V8 from default 0.85V */
	pmic_reg_write(p, PCA9450_LDO2CTRL, 0xC0);

	/* set WDOG_B_CFG to cold reset */
	pmic_reg_write(p, PCA9450_RESET_CTRL, 0xA1);

	return 0;
}
#else
int power_init_board(void)
{
	struct pmic *p;
	int ret;

	ret = power_bd71837_init(I2C_PMIC);
	if (ret)
		printf("power init failed");

	p = pmic_get("BD71837");
	pmic_probe(p);


	/* decrease RESET key long push time from the default 10s to 10ms */
	pmic_reg_write(p, BD718XX_PWRONCONFIG1, 0x0);

	/* unlock the PMIC regs */
	pmic_reg_write(p, BD718XX_REGLOCK, 0x1);

	/* increase VDD_SOC to typical value 0.85v before first DRAM access */
	pmic_reg_write(p, BD718XX_BUCK1_VOLT_RUN, 0x0f);

	/* increase VDD_DRAM to 0.975v for 3Ghz DDR */
	pmic_reg_write(p, BD718XX_1ST_NODVS_BUCK_VOLT, 0x83);

#ifndef CONFIG_IMX8M_LPDDR4
	/* increase NVCC_DRAM_1V2 to 1.2v for DDR4 */
	pmic_reg_write(p, BD718XX_4TH_NODVS_BUCK_VOLT, 0x28);
#endif

	/* lock the PMIC regs */
	pmic_reg_write(p, BD718XX_REGLOCK, 0x11);

	return 0;
}
#endif
#endif

void spl_board_init(void)
{
	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		if (sec_init())
			printf("\nsec_init failed!\n");
	}
	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	do_spl_pinmux();

	arch_cpu_init();

	board_early_init_f();

	timer_init();



	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	power_init_board();

	preloader_console_init();

	gf_init_som_eeprom(I2C_SOM_EEPROM_BUS_NO, I2C_SOM_EEPROM_ADDR);

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
