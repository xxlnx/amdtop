#include <string.h>
#include <errno.h>
#include "tabinfo.h"
#include "core/atombios.h"

static struct atombios gAtombios;
static struct GpuVBiosInfo gpuVBiosInfo;
static struct device *cur_device = NULL;

static int getAllInfo(void)
{
    struct Device *device = getAcitveDevice();
    int ret = 0;

    if (device == cur_device)
        return 0;
    cur_device = device;

    ret = gpuQueryVBiosInfo(device->gpuDevice, &gpuVBiosInfo);
    if (ret)
        return ret;

    ret = atombios_init(&gAtombios, gpuVBiosInfo.image, gpuVBiosInfo.imagelen);
    if (ret)
        return ret;

    return ret;
}

static int vbios_get_capability(uint32_t capability, char *buff, uint32_t len)
{
    size_t size = 0;
    if (capability & ATOM_FIRMWARE_CAP_FIRMWARE_POSTED)
        size += snprintf(buff, len, "%s %s", buff, "posted");
    if (capability & ATOM_FIRMWARE_CAP_GPU_VIRTUALIZATION)
        size += snprintf(buff, len, "%s %s", buff, "virtual");
    if (capability & ATOM_FIRMWARE_CAP_WMI_SUPPORT)
        size += snprintf(buff, len, "%s %s", buff, "wmi");
    if (capability & ATOM_FIRMWARE_CAP_HWEMU_ENABLE)
        size += snprintf(buff, len, "%s %s", buff, "hwemu");
    if (capability & ATOM_FIRMWARE_CAP_HWEMU_UMC_CFG)
        size += snprintf(buff, len, "%s %s", buff, "(cfg)");
    if (capability & ATOM_FIRMWARE_CAP_SRAM_ECC)
        size += snprintf(buff, len, "%s %s", buff, "SRAM ECC");
    return size;
}

static void printFirmwareInfo(struct Window *win, int32_t *y, int32_t *x)
{
    WINDOW* nwin = win->nwin;
    int32_t line = *y, col = *x;
    struct atombios *atombios = &gAtombios;
    struct atom_common_table_header *tableHeader = NULL;
    struct atom_firmware_info_v3_1 *firwamre_v3_1;
    struct atom_firmware_info_v3_2 *firmware_v3_2;
    struct atom_firmware_info_v3_3 *firmware_v3_3;
    tableHeader = atombios_get_data_by_id(atombios, get_data_table_id(firmwareinfo));
    if (tableHeader->format_revision == 3) {
        firwamre_v3_1 = (void *)tableHeader;
        mvwprintw2c(nwin, line++, col, "%-15s: %d Khz", "Boot SCLK", firwamre_v3_1->bootup_sclk_in10khz * 10);
        mvwprintw2c(nwin, line++, col, "%-15s: %d Khz", "Boot MCLK", firwamre_v3_1->bootup_mclk_in10khz * 10);
        mvwprintw2c(nwin, line++, col, "%-15s: %d mv", "VDDC", firwamre_v3_1->bootup_vddc_mv);
        mvwprintw2c(nwin, line++, col, "%-15s: %d mv", "VDDCI", firwamre_v3_1->bootup_vddci_mv);
        mvwprintw2c(nwin, line++, col, "%-15s: %d mv", "MVDDC", firwamre_v3_1->bootup_mvddc_mv);
        mvwprintw2c(nwin, line++, col, "%-15s: %d mv", "VDDGfx", firwamre_v3_1->bootup_vddgfx_mv);
        char capability_str[MAX_NAME_SIZE] = {0};
        size_t  size = vbios_get_capability(firwamre_v3_1->firmware_capability, capability_str, MAX_NAME_SIZE);
        capability_str[size] = '\0';
        mvwprintw2c(nwin, line++, col, "%-15s: %s", "Capability", capability_str + 1);
        mvwprintw2c(nwin, line++, col, "%-15s: %s", "Fan Type", firwamre_v3_1->coolingsolution_id == AIR_COOLING ? "Air" : "Liquid");
        mvwprintw2c(nwin, line++, col, "%-15s: 0x%x%08x", "MC Base High", firwamre_v3_1->mc_baseaddr_high, firwamre_v3_1->mc_baseaddr_low);
        switch (tableHeader->content_revision) {
            case 1:
                break;
            case 2:
                break;
            case 3:
                firmware_v3_3 = (void *)tableHeader;
                mvwprintw2c(nwin, line++, col, "%-15s: %d (0x%08x)", "PPTABLE ID", firmware_v3_3->pplib_pptable_id, firmware_v3_3->pplib_pptable_id);
                break;
            default:
                break;
        }
    }
    *y = line;
    *x = col;
}

static void printSMUInfo(struct Window *win, int32_t *y, int32_t *x)
{
    WINDOW* nwin = win->nwin;
    int32_t line = *y, col = *x;
    struct atombios *atombios = &gAtombios;
    struct atom_smu_info_v3_1 *smu_v3_1;
    struct atom_common_table_header *tableHeader = NULL;
    tableHeader = atombios_get_data_by_id(atombios, get_data_table_id(smu_info));
    if (tableHeader->format_revision == 3) {
        smu_v3_1 = (void *)tableHeader;
        mvwprintw2c(nwin, line++, col, "%-15s: %d.%d", "SMU Version", smu_v3_1->smuip_max_ver, smu_v3_1->smuip_min_ver);
    }
    *y = line;
    *x = col;
}

static void printGfxInfo(struct Window *win, int32_t *y, int32_t *x)
{
    WINDOW* nwin = win->nwin;
    int32_t line = *y, col = *x;
    struct atombios *atombios = &gAtombios;
    struct atom_common_table_header *tableHeader = NULL;
    struct atom_gfx_info_v2_2 *gfx_v2_2;
    tableHeader = atombios_get_data_by_id(atombios, get_data_table_id(gfx_info));
    if (tableHeader->format_revision == 2) {
        gfx_v2_2 = (void *)tableHeader;
        mvwprintw2c(nwin, line++, col, "%-15s: %d.%d", "GFX Version", gfx_v2_2->gfxip_max_ver, gfx_v2_2->gfxip_min_ver);
    }
    *y = line;
    *x = col;
}

static void printMMInfo(struct Window *win, int32_t *y, int32_t *x)
{
    WINDOW* nwin = win->nwin;
    int32_t line = *y, col = *x;
    struct atombios *atombios = &gAtombios;
    struct atom_common_table_header *tableHeader = NULL;
    struct atom_multimedia_info_v2_1 *mm_v2_1;
    tableHeader = atombios_get_data_by_id(atombios, get_data_table_id(multimedia_info));
    if (tableHeader->format_revision == 2) {
        mm_v2_1 = (void *)tableHeader;
        mvwprintw2c(nwin, line++, col, "%-15s: %d.%d", "UVD Version", mm_v2_1->uvdip_max_ver, mm_v2_1->uvdip_min_ver);
        mvwprintw2c(nwin, line++, col, "%-15s: %d.%d", "VCE Version", mm_v2_1->vceip_max_ver, mm_v2_1->vceip_min_ver);
    }
    *y = line;
    *x = col;
}

static void printDCEInfo(struct Window *win, int32_t *y, int32_t *x)
{
    WINDOW* nwin = win->nwin;
    int32_t line = *y, col = *x;
    struct atombios *atombios = &gAtombios;
    struct atom_common_table_header *tableHeader = NULL;
    struct atom_display_controller_info_v4_1 *dce_v4_1;
    tableHeader = atombios_get_data_by_id(atombios, get_data_table_id(multimedia_info));
    if (tableHeader->format_revision == 2) {
        dce_v4_1 = (void *)tableHeader;
        mvwprintw2c(nwin, line++, col, "%-15s: %d.%d", "DCE Version", dce_v4_1->dceip_max_ver, dce_v4_1->dceip_min_ver);
    }
    *y = line;
    *x = col;
}

static void printDisplayInfo(struct Window *win, int32_t *y, int32_t *x)
{
    WINDOW* nwin = win->nwin;
    int32_t line = *y, col = *x;
    struct atombios *atombios = &gAtombios;
    struct atom_common_table_header *tableHeader = NULL;
    struct display_object_info_table_v1_4 *display_v1_4;
    tableHeader = atombios_get_data_by_id(atombios, get_data_table_id(displayobjectinfo));
    if (tableHeader->format_revision == 1) {
        display_v1_4 = (void *)tableHeader;
        mvwprintw2c(nwin, line++, col, "%-15s: %d.%d", "Display", display_v1_4->supporteddevices, display_v1_4->number_of_path);
    }
    *y = line;
    *x = col;
}

static char *getatomramtype(uint32_t type)
{
    switch (type) {
        case ATOM_DGPU_VRAM_TYPE_GDDR5:
            return "GDDR5";
        case ATOM_DGPU_VRAM_TYPE_GDDR6:
            return "GDDR6";
        case ATOM_DGPU_VRAM_TYPE_HBM2:
            return "HBM2";
        default:
            return "unknow";
    }
    return "unknow";
}

static void printVraminfo(struct Window *win, int32_t *y, int32_t *x)
{

    WINDOW* nwin = win->nwin;
    int32_t line = *y, col = *x;
    struct atombios *atombios = &gAtombios;
    struct atom_common_table_header *tableHeader = NULL;
    struct atom_vram_info_header_v2_3 *vram_info_v2_3;
    struct atom_vram_info_header_v2_4 *vram_info_v2_4;
    uint32_t mem_type = 0;
    tableHeader = atombios_get_data_by_id(atombios, get_data_table_id(vram_info));
    if (tableHeader->format_revision == 2) {
        switch (tableHeader->content_revision) {
            case 3:
                vram_info_v2_3 = (void *)tableHeader;
                mem_type = vram_info_v2_3->vram_module[0].memory_type;
                break;
            case 4:
                vram_info_v2_4 = (void *)tableHeader;
                mem_type = vram_info_v2_4->vram_module[0].memory_type;
                break;
            default:
                break;
        }
    }

    mvwprintw2c(nwin, line++, col, "%-15s: %s", "VRAM Type", getatomramtype(mem_type));
    *y = line;
    *x = col;

}

static int tabBiosInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    struct atombios *atombios = &gAtombios;
    struct atom_common_table_header *tableHeader = NULL;
    struct atom_rom_header_v2_2 *atomrom = NULL;
    int ret = 0;
    int x, y;
    int line = 1;

    ret = getAllInfo();
    if (ret)
        return ret;

    x = win->layout.width / 20;

    atomrom = atombios_get_romheader(atombios);
    mvwprintw2c(nwin, line++, x, "%-15s: %s", "VBios Version", gpuVBiosInfo.vbios_version);
    mvwprintw2c(nwin, line++, x, "%-15s: %d Bytes", "VBios Size", gpuVBiosInfo.imagelen);
    mvwprintw2c(nwin, line++, x, "%-15s: %s", "VBios Date", atombios_get_timestamp(atombios));
    mvwprintw2c(nwin, line++, x, "%-15s: %s", "ConfigName", atombios_get_image(atombios, atomrom->configfilenameoffset));
    /* atombios firmware info */
    printFirmwareInfo(win, &line, &x);
    /* atombios smu info */
    printSMUInfo(win, &line, &x);
    /* atombios gfx info */
    printGfxInfo(win, &line, &x);
    /* atombios mm info */
    printMMInfo(win, &line, &x);
    /* atombios dce info */
    /* printDCEInfo(win, &line, &x); */
    /* atombios display info */
    /* printDisplayInfo(win, &line, &x); */
    /* atombios vram info */
    printVraminfo(win, &line, &x);

    wrefresh(nwin);
    return ret;
}

static int tabBiosInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabBiosInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo biosInfo = {
    .id = TabID_VBIOS,
    .name = "BiosInfo",
    .labelName = "Bios",
    .init = tabBiosInfoInit,
    .exit = tabBiosInfoExit,
    .update = tabBiosInfoUpdate,
    .period = 0,
};

