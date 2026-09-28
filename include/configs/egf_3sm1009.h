/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#ifndef __IMX8MP_EVK_H
#define __IMX8MP_EVK_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#define CONFIG_UPDATE_PACKAGE_HEADER_LENGTH 36
#if defined(CONFIG_CMD_NET)
#define CFG_FEC_MXC_PHYADDR          2
#endif

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	0x80000

/* Totally 6GB DDR */
#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x40000000	/* 1 GB */

#define CFG_MXC_UART_BASE		UART2_BASE_ADDR

#define CFG_SYS_NAND_BASE       0x20000000

#define CONFIG_SYS_I2C_SPEED 	400000

#ifdef CONFIG_TARGET_IMX8MP_DDR4_EVK
#define CFG_SYS_FSL_USDHC_NUM	1
#else
#define CFG_SYS_FSL_USDHC_NUM	2
#endif

#ifdef CONFIG_ANDROID_SUPPORT
#include "imx8mp_evk_android.h"
#endif

#endif
