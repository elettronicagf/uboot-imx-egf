#include "gf_rom_dyn_block.h"
#include "gf_factory_data_rom.h"
#include <stdlib.h>

#define PRINT_FIELD_SEGMENT_BLOCK	"  "

void print_dyn_block(struct gf_rom_dyn_block *block)
{
	int i;
	gf_debug(5, PRINT_FIELD_SEGMENT_BLOCK);
	gf_debug(5, "Block ID: 0x%x\n", block->dyn_block_header->block_id);
	gf_debug(5, PRINT_FIELD_SEGMENT_BLOCK);
	gf_debug(5, "Block len: %d\n", block->dyn_block_header->block_len);
	gf_debug(5, PRINT_FIELD_SEGMENT_BLOCK);
	gf_debug(5, "Block offset: %d\n", block->offset);
	gf_debug(5, PRINT_FIELD_SEGMENT_BLOCK);

	gf_debug(7, "Block Data:");
	for (i = 0; i < block->dyn_block_header->block_len; i++)
	{
		gf_debug(7, "0x%x ", block->dyn_block_data[i]);
	}
	gf_debug(7, "\n");

}

void print_dyn_blocks(struct gf_rom_dyn_block *dyn_blocks, int number_of_dyn_blocks)
{
	int i;

	for (i = 0; i < number_of_dyn_blocks; i++)
	{
		gf_debug(5, "Block number: %d\n", i + 1);
		print_dyn_block(&(dyn_blocks[i]));
	}
}
