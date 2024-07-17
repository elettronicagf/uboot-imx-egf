#include "gf_factory_data_rom.h"

#define DEFINE_PRINT_UPDATE(x) gf_rom_field_print_##x, gf_rom_field_update_##x

struct gf_rom_field header_layout_v4[] = {
	{ "ROM Id",          	FIELD_ROMID, 		2, 	NULL, DEFINE_PRINT_UPDATE(ascii_noterm)},
	{ "Layout Version",     FIELD_LAYOUT_VER, 	2, 	NULL, DEFINE_PRINT_UPDATE(bin_ver)},
	{ "ROM Size",			FIELD_ROM_SIZE,		4, 	NULL, DEFINE_PRINT_UPDATE(dec)},
	{ "Device Type",        FIELD_DEV_TYPE, 	1, 	NULL, DEFINE_PRINT_UPDATE(dev_type)},
	{ "Product Code",       FIELD_PRODUCT_CODE, 40, NULL, DEFINE_PRINT_UPDATE(ascii)},
	{ "Product Revision",   FIELD_PRODUCT_REV,  4, 	NULL, DEFINE_PRINT_UPDATE(dec)},
	{ "WID",   				FIELD_WID,  		25, NULL, DEFINE_PRINT_UPDATE(ascii)},
	{ "Production Date",   	FIELD_PROD_DATE,  	12, NULL, DEFINE_PRINT_UPDATE(ascii)},
	{ "Operator",	   		FIELD_OPERATOR,  	10, NULL, DEFINE_PRINT_UPDATE(ascii)},
	{ "Serial Number",	   	FIELD_SERIAL_NO,  	20, NULL, DEFINE_PRINT_UPDATE(ascii)},
	{ "RFU",			   	FIELD_RFU,		  	78, NULL, DEFINE_PRINT_UPDATE(ascii)},
	{ "Dyn block number",	FIELD_DYN_BLOCK_NO,	1,	NULL, DEFINE_PRINT_UPDATE(dec)},
	{ "Checksum",			FIELD_HDR_CHECKSUM,	1,	NULL, DEFINE_PRINT_UPDATE(hex)},
};

int gf_rom_get_header_size(struct gf_factory_data_rom* rom)
{
	int i;
	int size = 0;
	for (i = 0; i < rom->header_no_of_fields; i++)
	{
		size = size + rom->header_fields[i].size;
	}
	return size;
}

void initialize_gf_rom_struct(struct gf_factory_data_rom *rom) {
	gf_debug(0,"GF ROM module version: %s\n",GF_ROM_SW_VERSION);
	rom->hw_status = GF_ROM_NOT_FOUND;
	rom->programming_status = GF_ROM_NOT_PROGRAMMED;
	rom->layout_revision = GF_ROM_LAYOUT_INVALID;
	rom->len = 0;
	rom->header_no_of_fields = 0;
	rom->dyn_blocks_number = 0;
	rom->loaded_status = ROM_NOT_LOADED;
}

/* rom_is_gf_std() - Check if rom is gf standard
 * @rom:		A pointer to rom device
 * Returns: 0 on success, 1 on failure
 */
static int rom_is_gf_std(struct gf_factory_data_rom *rom)
{
	/* Check EepId field value */
	if (!memcmp(rom->raw_content, GF_ROM_ID, 0x02))
	{
		gf_debug(3,"GF ROM Marker found\n");
		rom->programming_status = GF_ROM_PROGRAMMED;
		return TRUE;
	}
	else
	{
		gf_debug(0,"GF ROM Marker not found\n");
		rom->programming_status = GF_ROM_NOT_PROGRAMMED;
		return FALSE;
	}
}

/* rom_get_layout_rev() - Get GF rom layout rev
 * @rom:		A pointer to rom device
 * Returns: 0 on success, 1 on failure
 */
static int rom_get_layout_rev(struct gf_factory_data_rom *rom)
{
	u16 layout_revision;

	memcpy(&layout_revision, &(rom->raw_content[LAYOUT_REV_OFFSET]), sizeof(layout_revision));

	switch(layout_revision) {
	case GF_ROM_LAYOUT_REV_4_0:
		gf_debug(0,"GF ROM 4.0 layout revision\n");
		rom->layout_revision = GF_ROM_LAYOUT_V4;
		break;
	default:
		gf_debug(0,"GF ROM invalid layout revision\n");
		rom->layout_revision = GF_ROM_LAYOUT_INVALID;
		return FALSE;
	}
	return TRUE;
}

/* rom_get_size() - Get GF rom size in bytes
 * @rom:		A pointer to rom device
 * Returns: 0 on success, 1 on failure
 */
static int rom_get_size(struct gf_factory_data_rom* rom)
{
	u32 rom_size;

	memcpy(&rom_size, &(rom->raw_content[ROM_SIZE_OFFSET]), sizeof(rom_size));
	gf_debug(3,"GF ROM size is %d bytes\n", rom_size);
	rom->len = rom_size;
	return TRUE;
}

/* rom_identify() - Identify rom, check if is GF standard, layout revision and size.
 * @rom:		A pointer to rom device
 * Warning: Requires rom content already initialized, at least id, layout rev and size
 * Returns: 0 on success, 1 on failure
 */
int rom_identify(struct gf_factory_data_rom* rom)
{
	int ret;

	ret = rom_is_gf_std(rom);
	if (ret == FALSE)
	{
		return ret;
	}

	ret = rom_get_layout_rev(rom);
	if (ret == FALSE)
	{
		return ret;
	}

	ret = rom_get_size(rom);
	if (ret == FALSE)
	{
		return ret;
	}

	return TRUE;
}

int rom_setup_layout(struct gf_factory_data_rom *rom)
{
	if (rom->layout_revision != GF_ROM_LAYOUT_V4)
		return FALSE;

	rom->header_fields = header_layout_v4;
	rom->header_no_of_fields = ARRAY_SIZE(header_layout_v4);
	return TRUE;
}

struct gf_rom_field *get_field(struct gf_factory_data_rom *rom, int fieldId)
{
	int i;
	for (i = 0; i < rom->header_no_of_fields; i++)
	{
		if (rom->header_fields[i].field_id == fieldId)
			return &rom->header_fields[i];
	}
	return NULL;
}


static int rom_get_checksum_offset(struct gf_factory_data_rom *rom)
{
	int i;
	int offset = 0;
	for (i = 0; i < rom->header_no_of_fields; i++)
	{
		if (rom->header_fields[i].field_id == FIELD_HDR_CHECKSUM)
			return offset;
		else
			offset = offset + rom->header_fields[i].size;
	}
	return -1;
}

int rom_calculate_checksum(struct gf_factory_data_rom *rom, u8 *cs)
{
	int cs_offset;
	u8 checksum = 0;
	int i;

	cs_offset = rom_get_checksum_offset(rom);
	if (cs_offset == -1)
	{
		gf_debug(0, "GF ROM Error getting checksum offset\n");
		return FALSE;
	}
	gf_debug(6, "GF ROM checksum offset is %d\n", cs_offset);
	gf_debug(6, "GF ROM len is %d\n", rom->len);

	for (i = 0; i < rom->len; i++)
	{
		if (i == cs_offset)
		{
			gf_debug(5, "GF ROM Checksum: Skipping CS offset %x\n", rom->raw_content[i]);
			continue;
		} else {
			checksum = checksum + (u8)rom->raw_content[i];
		}
	}
	gf_debug(5, "GF ROM Calculated checksum is 0x%x\n", checksum);
	*cs = checksum;
	return TRUE;
}

int rom_validate_checksum(struct gf_factory_data_rom *rom)
{
	int ret;
	u8 calculated_checksum, rom_header_checksum;
	struct gf_rom_field *checksum_field;
	ret = rom_calculate_checksum(rom, &calculated_checksum);
	if (ret != TRUE)
	{
		gf_debug(0, "GF ROM Error while computing checksum\n");
		return ret;
	}
	gf_debug(3, "GF ROM Calculated Checksum is 0x%x\n", calculated_checksum);

	checksum_field = get_field(rom, FIELD_HDR_CHECKSUM);
	rom_header_checksum = (u8)*(checksum_field->buf);
	gf_debug(3, "GF ROM Header Checksum is 0x%x\n", rom_header_checksum);
	if(calculated_checksum != rom_header_checksum)
	{
		gf_debug(0, "GF ROM Checksum mismatch. Expected %d, got %d\n", calculated_checksum, rom_header_checksum);
		gf_debug(0, "GF ROM is corrupted\n");
		return FALSE;
	}
	gf_debug(0, "GF ROM Checksum validated\n");
	return TRUE;

}

void rom_decode_header_fields(struct gf_factory_data_rom *rom)
{
	int i;
	unsigned char *buf;

	buf = rom->raw_content;

	for (i = 0; i < rom->header_no_of_fields; i++)
	{
		rom->header_fields[i].buf = buf;
		buf += rom->header_fields[i].size;
	}
	return;
}



static int gf_rom_get_field_value_int(struct gf_factory_data_rom *rom, int fieldId)
{
	int value = 0;
	struct gf_rom_field * field;

	field = get_field(rom, fieldId);
	if (field == NULL)
	{
		gf_debug(0,"GF ROM Invalid field %d\n", fieldId);
		return 0;
	}
	value = gf_rom_field_get_int(field);

	return value;
}

static void gf_rom_get_field_value_ascii(struct gf_factory_data_rom *rom, int fieldId, char **buf)
{
	struct gf_rom_field * field;
	int max_len;

	field = get_field(rom, fieldId);
	if (field == NULL)
	{
		gf_debug(0,"GF ROM Invalid field %d\n", fieldId);
		return;
	}
	
	max_len = field->size + 1;
	*buf = malloc(max_len);
	
	gf_rom_field_get_ascii(field, *buf, max_len);

	return;
}



void print_eeprom_header(struct gf_factory_data_rom* rom)
{
	int i = 0;
	if (DEBUG_LEVEL <= 4)
		return;
	for (i = 0; i < rom->header_no_of_fields; i++)
	{
		rom->header_fields[i].print(&(rom->header_fields[i]),"  ");
	}
	return;
}


static int rom_init_dyn_blocks(struct gf_factory_data_rom *rom)
{
	rom->dyn_blocks_number = gf_rom_get_field_value_int(rom, FIELD_DYN_BLOCK_NO);
	gf_debug(3,"GF ROM Number of dynamic blocks: %d\n", rom->dyn_blocks_number);
	rom->dyn_blocks = malloc(rom->dyn_blocks_number * sizeof(struct gf_rom_dyn_block));
	if (rom->dyn_blocks == NULL)
	{
		printf("dyn_blocks malloc error");
		return FALSE;
	}
	return TRUE;
}

static void rom_decode_dyn_blocks(struct gf_factory_data_rom *rom)
{
	int i;
	int header_size;
	int offset;

	header_size = gf_rom_get_header_size(rom);

	gf_debug(5,"GF ROM header size is %d\n", header_size);

	offset = header_size;

	for (i = 0; i < rom->dyn_blocks_number; i++)
	{
		gf_debug(4, "Decoding block %d\n", i+1);
		rom->dyn_blocks[i].offset = offset;
		gf_debug(5, "Offset = %x\n", offset);
		rom->dyn_blocks[i].dyn_block_header = (struct gf_rom_dyn_block_header *)&(rom->raw_content[offset]);
		rom->dyn_blocks[i].dyn_block_data = &(rom->raw_content[offset + sizeof(struct gf_rom_dyn_block_header)]);
		offset = offset + sizeof(struct gf_rom_dyn_block_header) + rom->dyn_blocks[i].dyn_block_header->block_len;
	}
}

int rom_decode_content(struct gf_factory_data_rom *rom)
{
	int ret;
	ret = rom_setup_layout(rom);
	if (ret != TRUE)
	{
		gf_debug(0, "GF ROM: error during setup layout\n");
		return FALSE;
	}

	rom_decode_header_fields(rom);

	ret = rom_validate_checksum(rom);
	if (ret != 0)
	{
		gf_debug(0,"GF SPI ROM Checksum error. Skipping initialization\n");
		return FALSE;
	}

	print_eeprom_header(rom);
	ret = rom_init_dyn_blocks(rom);
	gf_debug(4, "Initialized dynamic blocks\n");
	if (ret != TRUE)
	{
		gf_debug(0, "GF ROM: error during dynamic blocks initialization\n");
		return FALSE;
	}
	rom_decode_dyn_blocks(rom);
	print_dyn_blocks(rom->dyn_blocks, rom->dyn_blocks_number);

	return TRUE;
}

int rom_get_dyn_block(struct gf_factory_data_rom *rom, int block_id, int * block_len, unsigned char **block_data )
{
	int i;
	for (i = 0; i < rom->dyn_blocks_number; i++)
	{
		if (rom->dyn_blocks[i].dyn_block_header->block_id == block_id)
		{
			*block_len = rom->dyn_blocks[i].dyn_block_header->block_len;
			*block_data = rom->dyn_blocks[i].dyn_block_data;
			return TRUE;
		}
	}
	return FALSE;
}

void rom_get_wid(struct gf_factory_data_rom *rom, char **buf)
{
	gf_rom_get_field_value_ascii(rom, FIELD_WID, buf);
}

unsigned int rom_get_ram_size(struct gf_factory_data_rom *rom)
{
	int i,j;
	unsigned int ret = 0;
	gf_debug(6, "Total Block number: %d\n", rom->dyn_blocks_number);
	for (i = 0; i < rom->dyn_blocks_number; i++)
	{
		gf_debug(6, "Block number: %d\n", i + 1);
		if (rom->dyn_blocks[i].dyn_block_header->block_id == DYN_BLOCK_MX8M_RAM_SIZE)
		{
			if (rom->dyn_blocks[i].dyn_block_header->block_len != DYN_BLOCK_RAM_SIZE_LEN)
				gf_debug(0,"RAM size dyn block malformed. Expected len: 4 found %d\n", rom->dyn_blocks[i].dyn_block_header->block_len);
			else
			{
				for (j = 0; j < DYN_BLOCK_RAM_SIZE_LEN; j++)
					ret = ret | (rom->dyn_blocks[i].dyn_block_data[j]) << (j * 8);
				return ret;
			}
		}
	}
	return 0;
}

int rom_get_mac_address_1(struct gf_factory_data_rom *rom, char *buf)
{
	int i;
	gf_debug(6, "Total Block number: %d\n", rom->dyn_blocks_number);
	for (i = 0; i < rom->dyn_blocks_number; i++)
	{
		gf_debug(6, "Block number: %d\n", i + 1);
		if (rom->dyn_blocks[i].dyn_block_header->block_id == DYN_BLOCK_MAC_ADDRESS_1)
		{
			if (rom->dyn_blocks[i].dyn_block_header->block_len != DYN_BLOCK_MAC_ADDRESS_LEN)
				gf_debug(0,"MAC address dyn block malformed. Expected len: 17 found %d\n", rom->dyn_blocks[i].dyn_block_header->block_len);
			else
			{
				strncpy(buf, (char *)rom->dyn_blocks[i].dyn_block_data, DYN_BLOCK_MAC_ADDRESS_LEN);
				buf[DYN_BLOCK_MAC_ADDRESS_LEN] = 0;
				return TRUE;
			}
		}
	}
	return FALSE;
}

int rom_get_dts_name(struct gf_factory_data_rom *rom, char *buf)
{
	int i;
	gf_debug(6, "Total Block number: %d\n", rom->dyn_blocks_number);
	for (i = 0; i < rom->dyn_blocks_number; i++)
	{
		gf_debug(6, "Block number: %d\n", i + 1);
		if (rom->dyn_blocks[i].dyn_block_header->block_id == DYN_BLOCK_DTS_TO_LOAD)
		{
			if (rom->dyn_blocks[i].dyn_block_header->block_len != DYN_BLOCK_DTS_TO_LOAD_LEN)
				gf_debug(0,"DTS to load dyn block malformed. Expected len: %d found %d\n", DYN_BLOCK_DTS_TO_LOAD_LEN, rom->dyn_blocks[i].dyn_block_header->block_len);
			else
			{
				strncpy(buf, (char *)rom->dyn_blocks[i].dyn_block_data, DYN_BLOCK_DTS_TO_LOAD_LEN);
				buf[DYN_BLOCK_DTS_TO_LOAD_LEN] = 0;
				return TRUE;
			}
		}
	}
	return 0;
}
