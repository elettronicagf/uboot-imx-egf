#include "gf_rom_field.h"

#include <common.h>
#include <linux/string.h>

const char* device_type_string[] = {
		"Invalid",
		"SoM",
		"Carrier",
		"Display"
};

int gf_rom_field_get_int(const struct gf_rom_field *field)
{
	int value = 0;
	int i;

	if (field->size > 4)
	{
		printf("gf_rom_get_field_int: field is not an int");
		return 0;
	}

	for (i = 0; i < field->size; i++)
		value = value + (field->buf[i] << 8 * i);

	return value;
}

/**
 * gf_rom_field_get_ascii() - return a field which contains ASCII data
 * @field:	an initialized field to print
 */
void gf_rom_field_get_ascii(const struct gf_rom_field *field, char* buf, int max_len)
{
	int i;
	int max_avail_len;
	
	if (field->size > max_len)
		max_avail_len = max_len;
	else
		max_avail_len = field->size;

	for (i = 0; i < max_avail_len - 1; i++)
	{
		buf[i] = field->buf[i];
	}
	buf[max_avail_len - 1] = 0;
	
	return;
}


void gf_rom_field_print_dec(const struct gf_rom_field *field, const char * prefix)
{
	int value = 0;

	if (field->size > 4)
		return;

	value = gf_rom_field_get_int(field);

	printf("%s", prefix);
	printf("%s: ", field->name);
	printf("%d\n", value);
}

int gf_rom_field_update_dec(struct gf_rom_field *field, char *value)
{
	int i;
	memset(field->buf, 0, field->size);
	for (i = 0; i < field->size; i++)
	{
		field->buf[i] = value[i];
	}
	return 0;
}

void gf_rom_field_print_hex(const struct gf_rom_field *field, const char * prefix)
{
	int value = 0;

	if (field->size > 4)
		return;

	value = gf_rom_field_get_int(field);

	printf("%s", prefix);
	printf("%s: ", field->name);
	printf("0x%x\n", value);
}

int gf_rom_field_update_hex(struct gf_rom_field *field, char *value)
{
	return gf_rom_field_update_dec(field, value);
}

/**
 * gf_rom_field_print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
void gf_rom_field_print_ascii(const struct gf_rom_field *field, const char * prefix)
{
	int i;

	printf("%s", prefix);
	printf("%s: ", field->name);
	for (i = 0; i < field->size; i++)
	{
		printf("%c", field->buf[i]);
	}
	printf("\n");
}

/**
 * gf_rom_field_update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
int gf_rom_field_update_ascii(struct gf_rom_field *field, char *value)
{
	if (strlen(value) >= field->size) {
		printf("%s: new data too long\n", field->name);
		return -1;
	}

	strncpy((char *)field->buf, value, field->size - 1);
	field->buf[field->size - 1] = '\0';

	return 0;
}

/**
 * gf_rom_field_print_ascii_noterm() - print a field which contains ASCII data without terminator
 * @field:	an initialized field to print
 */
void gf_rom_field_print_ascii_noterm(const struct gf_rom_field *field, const char * prefix)
{
	gf_rom_field_print_ascii(field, prefix);
}

/**
 * gf_rom_field_update_ascii_noterm() - Update field with new data in ASCII form without termination char
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
int gf_rom_field_update_ascii_noterm(struct gf_rom_field *field, char *value)
{
	strncpy((char *)field->buf, value, field->size);
	return 0;
}

/**
 * gf_rom_field_print_bin_ver() - print a "version field" which contains binary
 *				  data
 *
 * Treat the field data as simple binary data, and print it formatted as a
 * version number (2 digits after decimal point).
 * The field size must be exactly 2 bytes.
 *
 * Sample output:
 *      Field Name      123.45
 *
 * @field:	an initialized field to print
 */
void gf_rom_field_print_bin_ver(const struct gf_rom_field *field, const char * prefix)
{
	if ((field->buf[0] == 0xff) && (field->buf[1] == 0xff)) {
		field->buf[0] = 0;
		field->buf[1] = 0;
	}

	printf("%s", prefix);
	printf("%s: ", field->name);
	int major = (field->buf[1] << 8 | field->buf[0]) / 100;
	int minor = (field->buf[1] << 8 | field->buf[0]) - major * 100;
	printf("%d.%02d\n", major, minor);
}

/**
 * gf_rom_field_update_bin_ver() - update a "version field" which contains
 *				   binary data
 *
 * This function takes a version string in the form of x.y (x and y are both
 * decimal values, y is limited to two digits), translates it to the binary
 * form, then writes it to the field. The field size must be exactly 2 bytes.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow.
 *
 * @field:	an initialized field
 * @value:	a version string
 *
 * Returns 0 on success, -1 on failure.
 */
int gf_rom_field_update_bin_ver(struct gf_rom_field *field, char *value)
{
	char *endptr;
	char *tok = strtok(value, ".");

	if (tok == NULL)
		return -1;

	int num = simple_strtol(tok, &endptr, 0);
	if (*endptr != '\0')
		return -1;

	tok = strtok(NULL, "");
	if (tok == NULL)
		return -1;

	int remainder = simple_strtol(tok, &endptr, 0);
	if (*endptr != '\0')
		return -1;

	num = num * 100 + remainder;
	if (num >> 16)
		return -1;

	field->buf[0] = (unsigned char)num;
	field->buf[1] = num >> 8;

	return 0;
}


void gf_rom_field_print_dev_type(const struct gf_rom_field *field, const char * prefix)
{
	if (*(field->buf) < ARRAY_SIZE(device_type_string))
	{
		printf("%s", prefix);
		printf("%s: ", field->name);
		printf("%s\n", device_type_string[*(field->buf)]);
	}
	return;
}

int gf_rom_field_update_dev_type(struct gf_rom_field *field, char *value)
{
	return gf_rom_field_update_dec(field, value);
}

