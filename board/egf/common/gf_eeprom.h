#ifndef GF_EEPROM_H_
#define GF_EEPROM_H_

#include "util.h"
#include "gf_factory_data_rom.h"
#include <asm/arch/ddr.h>

#define GF_EEPROM_BASE_OFFSET	0x0

struct i2c_eeprom {
	u8 bus_num;
	u8 i2c_address;
	u8 address_len;
	struct udevice *bus;
	struct udevice *i2c_dev;
	struct gf_factory_data_rom rom;
};

int gf_init_som_eeprom(u8 bus_num, u8 i2c_address);
int gf_init_carrier_eeprom(u8 bus_num, u8 i2c_address);
int gf_get_mac_address_1(char *buf);
int gf_get_carrier_wid(char **buf);
int gf_get_dts_name(char *buf);
#ifdef CONFIG_SPL_BUILD
int gf_read_dram_timings_mx8m(struct dram_timing_info* info);
u64 gf_get_ram_size(void);
#endif
#endif
