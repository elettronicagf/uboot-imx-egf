#ifndef _FIELD_
#define _FIELD_


enum field_ids {
	FIELD_ROMID,
	FIELD_LAYOUT_VER,
	FIELD_ROM_SIZE,
	FIELD_DEV_TYPE,
	FIELD_PRODUCT_CODE,
	FIELD_PRODUCT_REV,
	FIELD_WID,
	FIELD_PROD_DATE,
	FIELD_OPERATOR,
	FIELD_SERIAL_NO,
	FIELD_DYN_BLOCK_NO,
	FIELD_HDR_CHECKSUM,
	FIELD_RFU,
};

struct gf_rom_field {
	char *name;
	int field_id;
	int size;
	unsigned char *buf;

	void (*print)(const struct gf_rom_field *field, const char * prefix);
	int (*update)(struct gf_rom_field *field, char *value);
};

void gf_rom_field_print_ascii(const struct gf_rom_field *field, const char * prefix);
int gf_rom_field_update_ascii(struct gf_rom_field *field, char *value);

void gf_rom_field_print_ascii_noterm(const struct gf_rom_field *field, const char * prefix);
int gf_rom_field_update_ascii_noterm(struct gf_rom_field *field, char *value);

void gf_rom_field_print_bin_ver(const struct gf_rom_field *field, const char * prefix);
int gf_rom_field_update_bin_ver(struct gf_rom_field *field, char *value);

void gf_rom_field_print_dec(const struct gf_rom_field *field, const char * prefix);
int gf_rom_field_update_dec(struct gf_rom_field *field, char *value);
int gf_rom_field_get_int(const struct gf_rom_field *field);
void gf_rom_field_get_ascii(const struct gf_rom_field *field, char* buf, int max_len);

void gf_rom_field_print_hex(const struct gf_rom_field *field, const char * prefix);
int gf_rom_field_update_hex(struct gf_rom_field *field, char *value);

void gf_rom_field_print_dev_type(const struct gf_rom_field *field, const char * prefix);
int gf_rom_field_update_dev_type(struct gf_rom_field *field, char *value);

#endif
