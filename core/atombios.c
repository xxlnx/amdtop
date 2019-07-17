#include "atombios.h"
#include <errno.h>
#include <wchar.h>
#include <stdio.h>

int atombios_init(struct atombios *atombios, void *bios_image, uint32_t len)
{
    int ret = 0;
    uint8_t *bios = (uint8_t *)bios_image;
    struct atom_rom_header_v2_2 *romHeader = NULL;

    if (!atombios || ! bios_image|| !len)
        return -EINVAL;
    if (!(bios[0] == 0x55 && bios[1] == 0xaa))
        return -EINVAL;

    atombios->image = (uint8_t *)bios_image;
    atombios->len = len;
    romHeader = atombios_get_romheader(atombios);
    atombios->data_offset = romHeader->masterdatatable_offset;
    atombios->function_offset = romHeader->masterhwfunction_offset;

    return ret;
}

void* atombios_get_image(struct atombios* atombios, uint32_t offset)
{
    if (!atombios)
        return  NULL;
    return atombios->image + offset;
}

const char* atombios_get_timestamp(struct atombios *atombios)
{
    return atombios_get_image(atombios, OFFSET_TO_GET_ATOMBIOS_TIME_STAMP);
}

void* atombios_get_romheader(struct atombios *atombios)
{
    uint16_t rom_offset = 0;
    rom_offset = *(uint16_t *)atombios_get_image(atombios, OFFSET_TO_ATOM_ROM_HEADER_POINTER);
    return atombios_get_image(atombios, rom_offset);
}

void *atombios_get_data_by_id(struct atombios *atombios, uint32_t index)
{
    uint16_t *data_table = NULL;
    if(!atombios)
        return NULL;
    data_table = atombios_get_image(atombios, atombios->data_offset + sizeof(struct atom_common_table_header));
    if (!data_table)
        return NULL;
    return atombios_get_image(atombios, data_table[index]);
}

void *atombios_get_func_by_id(struct atombios *atombios, uint32_t index)
{
    uint16_t *func_table = NULL;
    if(!atombios)
        return NULL;
    func_table = atombios_get_image(atombios, atombios->function_offset + sizeof(struct atom_common_table_header));
    if (!func_table)
        return NULL;
    return atombios_get_image(atombios, func_table[index]);
}
