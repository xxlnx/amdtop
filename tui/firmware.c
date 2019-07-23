#include "tabinfo.h"

static struct Device *current_device = NULL;
static char *default_parse_firmware(struct GpuFwInfo *fwInfo, char *buf)
{
    size_t  size = 0;

    if (!fwInfo || !buf)
        return NULL;

    switch (fwInfo->type) {
        case FwType_SMC:
            size = sprintf(buf, "version: %d.%d.%d \t(0x%08x), feature: 0x%08x",
                           (fwInfo->version >> 16) & 0xffff,
                           (fwInfo->version >> 8) & 0xff,
                           (fwInfo->version) & 0xff,
                           fwInfo->version, fwInfo->feature);
            break;
        case FwType_UVD:
        case FwType_VCE:
            size = sprintf(buf, "version: %d.%d.%d.%d \t(0x%08x), feature: 0x%08x",
                           (fwInfo->version >> 24) & 0xff,
                           (fwInfo->version >> 16) & 0xff,
                           (fwInfo->version >> 8) & 0xff,
                           (fwInfo->version) & 0xff,
                           fwInfo->version, fwInfo->feature);
            break;
        case FwType_CE:
        case FwType_MC:
        case FwType_ME:
        case FwType_PFP:
        case FwType_RLC:
        case FwType_SDMA:
            size = sprintf(buf, "version: %d \t\t(0x%08x), feature: 0x%08x",
                            (fwInfo->version) & 0xff,
                           fwInfo->version, fwInfo->feature);
            break;
        case FwType_SOS:
            size = sprintf(buf, "version: %d.%d \t(0x%08x), feature: 0x%08x",
                           (fwInfo->version >> 16) & 0xffff,
                           (fwInfo->version) & 0xffff,
                           fwInfo->version, fwInfo->feature);
            break;
        case FwType_ASD:
            size = sprintf(buf, "version: %d.%d.%d \t(0x%08x), feature: 0x%08x",
                           (fwInfo->version >> 16) & 0xffff,
                           (fwInfo->version >> 8) & 0xff,
                           (fwInfo->version) & 0xff,
                           fwInfo->version, fwInfo->feature);
            break;
        default:
            size = sprintf(buf, "version: 0x%08x, feature: 0x%08x", fwInfo->version, fwInfo->feature);
            break;
    }

    buf[size] = '\0';

    return buf;
}

struct firmware_info {
    char *name;
    enum GpuFwType fwType;
    char *(*parse_version) (struct GpuFwInfo *fwInfo, char *buf);
    struct GpuFwInfo fwInfo;
};

static struct firmware_info fw_infos[] = {
    {"VCE", FwType_VCE, default_parse_firmware, {0}},
    {"UVD", FwType_UVD, default_parse_firmware, {0}},
    {"MC",  FwType_MC, default_parse_firmware, {0}},
    {"ME",  FwType_ME, default_parse_firmware, {0}},
    {"PFP", FwType_PFP, default_parse_firmware, {0}},
    {"CE", FwType_CE, default_parse_firmware, {0}},
    {"RLC", FwType_RLC, default_parse_firmware, {0}},
    {"SOS", FwType_SOS, default_parse_firmware, {0}},
    {"ASD", FwType_ASD, default_parse_firmware, {0}},
    {"SDMA", FwType_SDMA, default_parse_firmware, {0}},
    {"SMC", FwType_SMC, default_parse_firmware, {0}},
};

#define FW_INFO_COUNT ARRAY_SIZE(fw_infos)

static int getFirmwareInfo(void)
{
    int ret = 0;
    struct Device *device = getAcitveDevice();
    struct firmware_info *fwinfo = NULL;

    if (current_device == device)
        return 0;

    for (int i = 0; i < FW_INFO_COUNT; i++) {
        fwinfo = &fw_infos[i];
        ret = gpuQueryFWInfo(device->gpuDevice, fwinfo->fwType, 0, 0, &fwinfo->fwInfo);
        if (ret)
            return ret;
    }

    return ret;
}

static int tabFirmwareInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    int ret = 0;
    int x = 15, info_x = 20;
    int line = 2;
    struct firmware_info *fw_info = NULL;
    char buff[MAX_NAME_SIZE] = {0};

    ret = getFirmwareInfo();
    if (ret)
        return ret;

    for (int i = 0; i < FW_INFO_COUNT; i++) {
        fw_info = &fw_infos[i];
        mvwprintw2c(nwin, line++, x, "%s: %s", "Firmware", fw_info->name);
        mvwprintw2c(nwin, line++, info_x, "%s: %s", "Info", fw_info->parse_version(&fw_info->fwInfo, buff));
    }

    wrefresh(nwin);
    return ret;
}

static int tabFirmwareInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabFirmwareInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo firmwareInfo = {
    .name = "firwmare",
    .labelName = "Firmware",
    .init = tabFirmwareInfoInit,
    .exit = tabFirmwareInfoExit,
};

