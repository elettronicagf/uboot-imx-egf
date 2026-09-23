#ifndef _DYN_BLOCK_
#define _DYN_BLOCK_
#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/types.h>

#define DYN_BLOCK_MAC_ADDRESS_LEN 17
#define DYN_BLOCK_RAM_SIZE_LEN	4
#define DYN_BLOCK_DTS_TO_LOAD_LEN 100

enum dyn_block_ids {
	DYN_BLOCK_MAC_ADDRESS_1 = 1,
	DYN_BLOCK_MAC_ADDRESS_2,
	DYN_BLOCK_MAC_ADDRESS_3,
	DYN_BLOCK_MAC_ADDRESS_4,
	DYN_BLOCK_MAC_ADDRESS_5,
	DYN_BLOCK_MAC_ADDRESS_6,
	DYN_BLOCK_SOM_FEATURES,
	DYN_BLOCK_CARRIER_FEATURES,
	DYN_BLOCK_MX8M_RAM_SIZE,
	DYN_BLOCK_MX8M_RAM_DDRC_CFG,
	DYN_BLOCK_MX8M_RAM_DDRPHY_CFG,
	DYN_BLOCK_MX8M_RAM_DDRPHY_TRAINED_CSR,
	DYN_BLOCK_MX8M_RAM_DDRPHY_PIE,
	DYN_BLOCK_MX8M_RAM_FSP_TABLE,
	DYN_BLOCK_MX8M_RAM_FSP_MSG,
	DYN_BLOCK_MX8M_RAM_FSP_CFG_1,
	DYN_BLOCK_MX8M_RAM_FSP_CFG_2,
	DYN_BLOCK_MX8M_RAM_FSP_CFG_3,
	DYN_BLOCK_MX8M_RAM_FSP_CFG_4,
	DYN_BLOCK_MX8M_RAM_FSP_CFG_5,
	DYN_BLOCK_MX8M_RAM_FSP_CFG_6,
	DYN_BLOCK_MX8M_RAM_FSP_CFG_7,
	DYN_BLOCK_MX8M_RAM_FSP_CFG_8,
	DYN_BLOCK_WESTON_TOUCH_CAL,
	DYN_BLOCK_DISP_MODEL,
	DYN_BLOCK_DTS_TO_LOAD,
};

struct __attribute__((__packed__))  gf_rom_dyn_block_header {
	u32 block_id;
	u32 block_len;
	u8 rfu[16];
};

struct __attribute__((__packed__))  gf_rom_dyn_block {
	u32 offset;
	struct gf_rom_dyn_block_header *dyn_block_header;
	unsigned char *dyn_block_data;
};

void print_dyn_block(struct gf_rom_dyn_block *block);
void print_dyn_blocks(struct gf_rom_dyn_block *dyn_blocks, int number_of_dyn_blocks);

#endif
