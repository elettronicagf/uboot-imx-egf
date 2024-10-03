/*
 * Copyright 2024 Elettronica GF s.r.l.
 *
 * Mantainers:
 * Stefano Donati <stefano.donati@elettronicagf.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <fs.h>

static int do_egf_update_validate_header(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;
	char *buf;
	char md5[33];
	int ret = 0;

	if (argc != 2) {
		return CMD_RET_USAGE;
	}

	addr = simple_strtoul(argv[1], NULL, 16);
	if (*argv[1] == 0)
		return -1;

	buf = map_physmem(addr, CONFIG_UPDATE_PACKAGE_HEADER_LENGTH, MAP_WRBACK);
	if ((*buf == 'e') &&
		*(buf + 1) == 'G' &&
		*(buf + 2) == 'F' &&
		*(buf + 3) == '1')
		ret = 0;
	else
		ret = 1;
	strncpy(md5, buf + 4, 32);
	md5[32] = 0;
	unmap_physmem(buf, CONFIG_UPDATE_PACKAGE_HEADER_LENGTH);
	env_set("update_md5", md5);
	return ret;
}

U_BOOT_CMD(
		egf_update_validate_header, 2, 1,	do_egf_update_validate_header,
		"Check if update package header is valid",
		"<addr>\n"
		"    - check update package header at address 'addr'\n"
);

