#ifndef _GFFACTORYINFO_
#define _GFFACTORYINFO_
#include "gf_rom_dyn_block.h"
#include "gf_rom_field.h"
#include <stdlib.h>

#define GF_ROM_SW_VERSION "3.0.0"

#define GF_ROM_ID					"GF"
#define GF_ROM_LAYOUT_REV_4_0		400

#define ROMID_OFFSET				0x0
#define LAYOUT_REV_OFFSET			0x2
#define ROM_SIZE_OFFSET			0x4

#define INITIAL_LOAD_REGION_SIZE	8

#define TRUE	0
#define FALSE	1

#define DEBUG_LEVEL 2

#define gf_debug(dbg_level,fmt,args...) \
	if (dbg_level<=DEBUG_LEVEL) printf(fmt, ##args); \
	else (void)0

enum rom_programming_status {
	GF_ROM_PROGRAMMED,
	GF_ROM_NOT_PROGRAMMED
};

enum rom_hw_status {
	GF_ROM_NOT_FOUND,
	GF_ROM_FOUND
};

enum rom_layout_version {
	GF_ROM_LAYOUT_V4,
	GF_ROM_LAYOUT_INVALID
};

enum rom_loaded_status {
	ROM_LOADED,
	ROM_NOT_LOADED,
};

struct gf_factory_data_rom {
	/* ROM hw status */
	int hw_status;
	/* ROM programming status */
	int programming_status;
	/* ROM loaded status */
	int loaded_status;
	/* GF ROM layout revision */
	int layout_revision;
	/* Total ROM size in bytes */
	int len;
	/* Number of fields in the header */
	int header_no_of_fields;
	/* List of pointer to the header fields.
	 * Pointer point to rom content starting char of each field.
	 * */
	struct gf_rom_field* header_fields;
	/* Number of dynamic blocks */
	int dyn_blocks_number;
	/* List of pointers to the dynamic
	 * blocks. Pointer point to rom content starting char of each dynamic block.
	 */
	struct gf_rom_dyn_block *dyn_blocks;
	/* RAW ROM content */
	unsigned char raw_content[SZ_16K];
};

void initialize_gf_rom_struct(struct gf_factory_data_rom* rom);
void print_eeprom_header(struct gf_factory_data_rom* rom);
int gf_rom_get_header_size(struct gf_factory_data_rom* rom);
int rom_identify(struct gf_factory_data_rom* rom);
int rom_setup_layout(struct gf_factory_data_rom *rom);
void rom_decode_header_fields(struct gf_factory_data_rom *rom);
int rom_decode_content(struct gf_factory_data_rom *rom);
int rom_get_dyn_block(struct gf_factory_data_rom* rom, int block_id, int * block_len, unsigned char **block_data );
struct gf_rom_field *get_field(struct gf_factory_data_rom *rom, int fieldId);
int rom_calculate_checksum(struct gf_factory_data_rom *rom, u8 *cs);
int rom_validate_checksum(struct gf_factory_data_rom *rom);
void rom_get_wid(struct gf_factory_data_rom *rom, char **buf);
int rom_get_mac_address_1(struct gf_factory_data_rom *rom, char *buf);
u64 rom_get_ram_size(struct gf_factory_data_rom *rom);
int rom_get_dts_name(struct gf_factory_data_rom *rom, char *buf);
#endif
