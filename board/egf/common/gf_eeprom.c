#include "gf_eeprom.h"

static struct i2c_eeprom GF_ATTRIBUTES som_eeprom;
static struct i2c_eeprom GF_ATTRIBUTES carrier_eeprom;

static void initialize_eeprom_struct(struct i2c_eeprom *eep, u8 bus_num, u8 i2c_address, u8 address_len) {
	eep->bus_num = bus_num;
	eep->i2c_address = i2c_address;
	eep->address_len = address_len;
	initialize_gf_rom_struct(&(eep->rom));
}

/* eeprom_check_hw() - Check eeprom hw
 * @eep:		A pointer to eeprom device
 */
static void eeprom_check_hw(struct i2c_eeprom *eep)
{
	int ret;
	eep->rom.hw_status = GF_ROM_NOT_FOUND;
#if CONFIG_IS_ENABLED(DM_I2C)
	ret = uclass_get_device_by_seq(UCLASS_I2C, eep->bus_num, &(eep->bus));
	if (ret) {
		gf_debug(0,"Can't find i2c bus %d\n", eep->bus_num);
		return;
	}

	ret = dm_i2c_probe(eep->bus, eep->i2c_address, 0, &(eep->i2c_dev));
	if (ret) {
		gf_debug(0,"Can't find device id=0x%x\n",eep->i2c_address);
		return;
	}
	i2c_set_chip_offset_len(eep->i2c_dev, eep->address_len);
#else
	ret = i2c_set_bus_num(eep->bus_num);
	if (ret) {
		gf_debug(0,"Error setting i2c bus %d\n", eep->bus_num);
		return;
	}

	ret = i2c_probe(eep->i2c_address);
	if (ret) {
		gf_debug(0,"Can't find device id=0x%x\n",eep->i2c_address);
		return;
	}
#endif
	eep->rom.hw_status = GF_ROM_FOUND;
}

static int eeprom_identify(struct i2c_eeprom *eep)
{
	int ret;

#if CONFIG_IS_ENABLED(DM_I2C)
	ret = dm_i2c_read(eep->i2c_dev, GF_EEPROM_BASE_OFFSET, eep->rom.raw_content, INITIAL_LOAD_REGION_SIZE);
#else
	ret = i2c_read(eep->i2c_address, GF_EEPROM_BASE_OFFSET, eep->address_len, eep->rom.raw_content, INITIAL_LOAD_REGION_SIZE);
#endif

	if (ret != 0)
	{
		gf_debug(0,"EEPROM initial load region read error\n");
		return FALSE;
	}

	ret = rom_identify(&(eep->rom));
	return ret;
}

static int eeprom_read(struct i2c_eeprom *eep)
{
	int ret;
	int index;
	gf_debug(3,"EEPROM reading %d bytes starting from offset 0x%x\n", eep->rom.len, GF_EEPROM_BASE_OFFSET);
#if CONFIG_IS_ENABLED(DM_I2C)
/* LPI2C has a maximum transfer size of 256 byte*/
#ifdef CONFIG_SYS_I2C_IMX_LPI2C
	printf("LPI2C controller detected: limit transfer size to 256 byte\n");
	for (index = 0; index < eep->rom.len / 256; index++)
		ret = dm_i2c_read(eep->i2c_dev, GF_EEPROM_BASE_OFFSET + index * 256, eep->rom.raw_content + 256 * index, 256);
	ret = dm_i2c_read(eep->i2c_dev, GF_EEPROM_BASE_OFFSET + index * 256, eep->rom.raw_content + 256 * index, eep->rom.len - 256 * index);
#else
	ret = dm_i2c_read(eep->i2c_dev, GF_EEPROM_BASE_OFFSET, eep->rom.raw_content, eep->rom.len);
#endif
#else
	ret = i2c_read(eep->i2c_address, GF_EEPROM_BASE_OFFSET, eep->address_len, eep->rom.raw_content, eep->rom.len);
#endif
	if (ret != 0)
	{
		gf_debug(0,"EEPROM read error %d\n", ret);
		return FALSE;
	}
	return TRUE;
}

int gf_init_som_eeprom(u8 bus_num, u8 i2c_addres)
{
	int ret;
	/* Prepare SOM eeprom structure */
	initialize_eeprom_struct(&som_eeprom, bus_num, i2c_addres, 2);
	/* Detect SOM eeprom */
	eeprom_check_hw(&som_eeprom);
	if (som_eeprom.rom.hw_status == GF_ROM_NOT_FOUND)
	{
		/* If SOM eeprom is not found return */
		gf_debug(0,"EEPROM not detected. HW issue?\n");
		return FALSE;
	}
	gf_debug(3,"EEPROM hw detected\n");

	ret = eeprom_identify(&som_eeprom);
	if (ret != 0)
	{
		gf_debug(0,"EEPROM setup error\n");
		return FALSE;
	}

	eeprom_read(&som_eeprom);

	ret = rom_decode_content(&(som_eeprom.rom));
	if (ret != 0)
	{
		gf_debug(0,"EEPROM decode contents error\n");
		return FALSE;
	}

	som_eeprom.rom.loaded_status = ROM_LOADED;

	return TRUE;
}

int gf_init_carrier_eeprom(u8 bus_num, u8 i2c_address)
{
	int ret;
	/* Prepare carrier eeprom structure */
	initialize_eeprom_struct(&carrier_eeprom, bus_num, i2c_address, 2);
	/* Detect SOM eeprom */
	eeprom_check_hw(&carrier_eeprom);
	if (carrier_eeprom.rom.hw_status == GF_ROM_NOT_FOUND)
	{
		/* If Carrier eeprom is not found return */
		gf_debug(0,"EEPROM Carrier not detected. HW issue?\n");
		return FALSE;
	}
	gf_debug(3,"EEPROM Carrier hw detected\n");

	ret = eeprom_identify(&carrier_eeprom);
	if (ret != 0)
	{
		gf_debug(0,"EEPROM Carrier setup error\n");
		return FALSE;
	}

	eeprom_read(&carrier_eeprom);

	ret = rom_decode_content(&(carrier_eeprom.rom));
	if (ret != 0)
	{
		gf_debug(0,"EEPROM Carrier decode contents error\n");
		return FALSE;
	}

	carrier_eeprom.rom.loaded_status = ROM_LOADED;

	return TRUE;
}


int gf_get_carrier_wid(char **buf)
{
	if (carrier_eeprom.rom.loaded_status != ROM_LOADED)
	{
		gf_debug(0, "GF ROM: Error Carrier ROM not loaded!\n");
		return FALSE;
	}
	rom_get_wid(&(carrier_eeprom.rom), buf);
	return TRUE;
}

int gf_get_mac_address_1(char *buf)
{
	int ret;
	if (som_eeprom.rom.loaded_status != ROM_LOADED)
	{
		gf_debug(0, "GF ROM: Error ROM not loaded!\n");
		return FALSE;
	}
	ret = rom_get_mac_address_1(&(som_eeprom.rom), buf);
	if (ret == TRUE)
	{
		printf("MAC Address is %s\n", buf);
		return TRUE;
	}
	else
	{
		printf("MAC Address not found or invalid\n");
		return FALSE;
	}
}

int gf_get_mac_address_2(char *buf)
{
	int ret;
	if (som_eeprom.rom.loaded_status != ROM_LOADED)
	{
		gf_debug(0, "GF ROM: Error ROM not loaded!\n");
		return FALSE;
	}
	ret = rom_get_mac_address_2(&(som_eeprom.rom), buf);
	if (ret == TRUE)
	{
		printf("MAC Address 2 is %s\n", buf);
		return TRUE;
	}
	else
	{
		printf("MAC Address 2 not found or invalid\n");
		return FALSE;
	}
}


int gf_get_dts_name(char *buf)
{
	int ret;
	if (carrier_eeprom.rom.loaded_status != ROM_LOADED)
	{
		gf_debug(0, "GF ROM: Error ROM not loaded!\n");
		return FALSE;
	}
	ret = rom_get_dts_name(&(carrier_eeprom.rom), buf);
	if (ret == TRUE)
	{
		printf("DTS to load name is: %s\n", buf);
		return TRUE;
	}
	else
	{
		printf("DTS to load name not found or invalid\n");
		return FALSE;
	}
}

#ifdef CONFIG_SPL_BUILD
static int get_fsp_cfg_block_id(int num)
{
	switch (num)
	{
	case 0:
		return DYN_BLOCK_MX8M_RAM_FSP_CFG_1;
	case 1:
		return DYN_BLOCK_MX8M_RAM_FSP_CFG_2;
	case 2:
		return DYN_BLOCK_MX8M_RAM_FSP_CFG_3;
	case 3:
		return DYN_BLOCK_MX8M_RAM_FSP_CFG_4;
	case 4:
		return DYN_BLOCK_MX8M_RAM_FSP_CFG_5;
	case 5:
		return DYN_BLOCK_MX8M_RAM_FSP_CFG_6;
	case 6:
		return DYN_BLOCK_MX8M_RAM_FSP_CFG_7;
	case 7:
		return DYN_BLOCK_MX8M_RAM_FSP_CFG_8;
	default:
		return -1;
	}
}

int gf_read_dram_timings_mx8m(struct dram_timing_info* info)
{
	int block_len, ret;
	unsigned char *data;
	int i, j;

	if (som_eeprom.rom.loaded_status != ROM_LOADED)
	{
		gf_debug(0, "GF ROM: Error ROM not loaded!\n");
		return FALSE;
	}

	info->total_size = rom_get_ram_size(&(som_eeprom.rom));
	gf_debug(3, "RAM size is %x\n", info->total_size);

	/* DDRC_CFG */
	ret = rom_get_dyn_block(&(som_eeprom.rom), DYN_BLOCK_MX8M_RAM_DDRC_CFG, &block_len, &data);
	if (ret == FALSE)
	{
		gf_debug(0, "Error getting dyn block for DDR_CFG\n");
		return ret;
	}
	gf_debug(5, "Read %d bytes for DDRC_CFG\n", block_len);

	info->ddrc_cfg_num = block_len / sizeof(struct dram_cfg_param);
	info->ddrc_cfg = (struct dram_cfg_param *)data;
	gf_debug(5, "DDRC_CFG size is %d\n", info->ddrc_cfg_num);
	gf_debug(6, "DDRC_CFG BLOCK:\n");

	for (i = 0; i < info->ddrc_cfg_num; i++)
	{
		gf_debug(6, "     0x%x 0x%x \n", info->ddrc_cfg[i].reg, info->ddrc_cfg[i].val);
	}
	/* END OF DDRC_CFG */

	/* DDRPHY_CFG */
	ret = rom_get_dyn_block(&(som_eeprom.rom), DYN_BLOCK_MX8M_RAM_DDRPHY_CFG, &block_len, &data);
	if (ret == FALSE)
	{
		gf_debug(0, "Error getting dyn block for DDRPHY_CFG\n");
		return ret;
	}
	gf_debug(5, "Read %d bytes for DDRPHY_CFG\n", block_len);
	info->ddrphy_cfg_num = block_len / sizeof(struct dram_cfg_param);
	info->ddrphy_cfg = (struct dram_cfg_param *)data;
	gf_debug(5, "DDRPHY_CFG size is %d\n", info->ddrphy_cfg_num);
	gf_debug(6, "DDRPHY_CFG BLOCK:\n");
	for (i = 0; i < info->ddrphy_cfg_num; i++)
	{
		gf_debug(6, "     0x%x 0x%x \n", info->ddrphy_cfg[i].reg, info->ddrphy_cfg[i].val);
	}
	/* END OF DDRPHY_CFG */

	/* DDRPHY_TRAINED_CSR */
	ret = rom_get_dyn_block(&(som_eeprom.rom), DYN_BLOCK_MX8M_RAM_DDRPHY_TRAINED_CSR, &block_len, &data);
	if (ret == FALSE)
	{
		gf_debug(0, "Error getting dyn block for DDRPHY_TRAINED_CSR\n");
		return ret;
	}
	gf_debug(5, "Read %d bytes for DDRPHY_TRAINED_CSR\n", block_len);
	info->ddrphy_trained_csr_num = block_len / sizeof(struct dram_cfg_param);
	info->ddrphy_trained_csr = (struct dram_cfg_param *)data;
	gf_debug(5, "DDRPHY_TRAINED_CSR size is %d\n", info->ddrphy_trained_csr_num);
	gf_debug(6, "DDRPHY_TRAINED_CSR BLOCK:\n");
	for (i = 0; i < info->ddrphy_trained_csr_num; i++)
	{
		gf_debug(6, "     0x%x 0x%x \n", info->ddrphy_trained_csr[i].reg, info->ddrphy_trained_csr[i].val);
	}
	/* END OF DDRPHY_TRAINED_CSR */


	/* DDR_PHY_PIE */
	ret = rom_get_dyn_block(&(som_eeprom.rom), DYN_BLOCK_MX8M_RAM_DDRPHY_PIE, &block_len, &data);
	if (ret == FALSE)
	{
		gf_debug(0, "Error getting dyn block for DDR_PHY_PIE\n");
		return ret;
	}
	gf_debug(5, "Read %d bytes for DDR_PHY_PIE\n", block_len);
	info->ddrphy_pie_num = block_len / sizeof(struct dram_cfg_param);
	info->ddrphy_pie = (struct dram_cfg_param *)data;
	gf_debug(5, "DDR_PHY_PIE size is %d\n", info->ddrphy_pie_num);
	gf_debug(6, "DDR_PHY_PIE BLOCK:\n");
	for (i = 0; i < info->ddrphy_pie_num; i++)
	{
		gf_debug(6, "     0x%x 0x%x \n", info->ddrphy_pie[i].reg, info->ddrphy_pie[i].val);
	}
	/* END OF DDR_PHY_PIE */


	/* FSP_TABLE */
	ret = rom_get_dyn_block(&(som_eeprom.rom), DYN_BLOCK_MX8M_RAM_FSP_TABLE, &block_len, &data);
	unsigned int *myfsp_table;
	if (ret == FALSE)
	{
		gf_debug(0, "Error getting dyn block for FSP_TABLE\n");
		return ret;
	}

	gf_debug(5, "Read %d bytes for FSP_TABLE\n", block_len);
	myfsp_table = malloc(ARRAY_SIZE(info->fsp_table));
	memcpy(myfsp_table, data, block_len);
	gf_debug(5, "FSP_TABLE size is %d\n",block_len);
	gf_debug(6, "FSP_TABLE BLOCK:\n");
	for (i = 0; i < ARRAY_SIZE(info->fsp_table); i++)
	{
		info->fsp_table[i] = myfsp_table[i];
		gf_debug(6, "     %d\n", info->fsp_table[i]);
	}
	/* END OF FSP_TABLE */

	/* FSP_MSG */
	ret = rom_get_dyn_block(&(som_eeprom.rom), DYN_BLOCK_MX8M_RAM_FSP_MSG, &block_len, &data);
	if (ret == FALSE)
	{
		gf_debug(0, "Error getting dyn block for FSP_MSG\n");
		return ret;
	}
	gf_debug(5, "Read %d bytes for FSP_MSG\n", block_len);
	info->fsp_msg_num = block_len / sizeof(struct dram_fsp_msg);
	info->fsp_msg = malloc(block_len);
	memcpy(info->fsp_msg, data, block_len);
	gf_debug(5, "FSP_MSG size is %d\n", info->fsp_msg_num);
	gf_debug(6, "FSP_MSG BLOCK:\n");

	for (i = 0; i < info->fsp_msg_num; i++)
	{
		gf_debug(6, "     drate = %d, fw_type =  %d, cfg_num = %d\n", info->fsp_msg[i].drate, info->fsp_msg[i].fw_type, info->fsp_msg[i].fsp_cfg_num);

		ret = rom_get_dyn_block(&(som_eeprom.rom), get_fsp_cfg_block_id(i), &block_len, &data);
		if (ret == FALSE)
		{
			printf("Error getting dyn block for FSP_CFG_BLOCK_%d\n", i);
			return ret;
		}
		gf_debug(5, "     Read %d bytes for FSP_CFG_BLOCK%d\n",block_len, i);

		info->fsp_msg[i].fsp_cfg = (struct dram_cfg_param *)data;
		gf_debug(6, "     FSP_CFG BLOCK:\n");
		for (j = 0; j < info->fsp_msg[i].fsp_cfg_num; j++)
			gf_debug(6, "          0x%x 0x%x\n", info->fsp_msg[i].fsp_cfg[j].reg, info->fsp_msg[i].fsp_cfg[j].val);

	}
	gf_debug(1, "GF ROM DDR timings loaded successfully\n");

	/* END OF FSP_MSG */
	return TRUE;
}

u64 gf_get_ram_size(void)
{
	u64 ret;

	if (som_eeprom.rom.loaded_status != ROM_LOADED)
	{
		gf_debug(0, "GF ROM: Error ROM not loaded!\n");
		return 0;

	}
	ret = rom_get_ram_size(&(som_eeprom.rom));
	return ret;
}
#endif