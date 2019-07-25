#ifndef __ATOMBIOS_H__
#define __ATOMBIOS_H__

#include <stdint.h>
#include "import/atomfirmware.h"
#include "stddef.h"

struct atombios {
    uint8_t *image;
    uint32_t len;
    void *rom_header;
    uint16_t  function_offset;
    uint16_t  data_offset;
};

#define OFFSET_TO_GET_ATOMBIOS_TIME_STAMP   (0x50)
#define get_index_into_master_table(master_table, table_name) (offsetof(struct master_table, table_name) / sizeof(uint16_t))
#define get_data_table_id(table_name)   get_index_into_master_table(atom_master_list_of_data_tables_v2_1, table_name)
#define get_cmd_table_id(table_name)    get_index_into_master_table(atom_master_list_of_command_functions_v2_1, table_name)

int atombios_init(struct atombios *atombios, void *bios_image, uint32_t len);
void* atombios_get_image(struct atombios* atombios, uint32_t offset);
const char *atombios_get_timestamp(struct atombios *atombios);
void *atombios_get_romheader(struct atombios *atombios);
void *atombios_get_data_by_id(struct atombios *atombios, uint32_t index);
void *atombios_get_func_by_id(struct atombios *atombios, uint32_t index);

#endif