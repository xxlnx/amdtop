#include <errno.h>
#include "tabinfo.h"

static struct Device *current_device = NULL;
static int default_parse_firmware(struct GpuFwInfo *fwInfo, char *ver_buf, char* fea_buf)
{
    size_t ver_size = 0, fea_size = 0;

    if (!fwInfo || !ver_buf || !fea_buf)
        return -EINVAL;

    switch (fwInfo->type) {
        case FwType_SMC:
            ver_size = sprintf(ver_buf, "%d.%d.%d \t0x%08x",
                           (fwInfo->version >> 16) & 0xffff,
                           (fwInfo->version >> 8) & 0xff,
                           (fwInfo->version) & 0xff,
                           fwInfo->version);
            break;
        case FwType_UVD:
        case FwType_VCE:
            ver_size = sprintf(ver_buf, "%d.%d.%d.%d \t0x%08x",
                           (fwInfo->version >> 24) & 0xff,
                           (fwInfo->version >> 16) & 0xff,
                           (fwInfo->version >> 8) & 0xff,
                           (fwInfo->version) & 0xff,
                           fwInfo->version);
            break;
        case FwType_CE:
        case FwType_MC:
        case FwType_ME:
        case FwType_PFP:
        case FwType_RLC:
        case FwType_SDMA:
            ver_size = sprintf(ver_buf, "%d \t0x%08x",
                            (fwInfo->version) & 0xff, fwInfo->version);
            break;
        case FwType_SOS:
            ver_size = sprintf(ver_buf, "%d.%d \t0x%08x",
                           (fwInfo->version >> 16) & 0xffff,
                           (fwInfo->version) & 0xffff,
                           fwInfo->version);
            break;
        case FwType_ASD:
            ver_size = sprintf(ver_buf, "%d.%d.%d 0x%08x",
                           (fwInfo->version >> 16) & 0xffff,
                           (fwInfo->version >> 8) & 0xff,
                           (fwInfo->version) & 0xff,
                           fwInfo->version);
            break;
        default:
            ver_size = sprintf(ver_buf, "0x%08x", fwInfo->version);
            fea_size = sprintf(fea_buf, "0x%08x", fwInfo->feature);
            break;
    }

    if (!ver_size)
        ver_size = sprintf(ver_buf, "0x%08x", fwInfo->version);
    if (!fea_size)
        fea_size = sprintf(fea_buf, "0x%08x", fwInfo->feature);

    ver_buf[ver_size] = '\0';
    fea_buf[fea_size] = '\0';

    return 0;
}

struct firmware_info {
    char *name;
    enum GpuFwType fwType;
    int (*parse_version) (struct GpuFwInfo *fwInfo, char *ver_buf, char *fea_buf);
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
    char ver_buff[MAX_NAME_SIZE] = {0};
    char fea_buff[MAX_NAME_SIZE] = {0};

    ret = getFirmwareInfo();
    if (ret)
        return ret;

    for (int i = 0; i < FW_INFO_COUNT; i++) {
        fw_info = &fw_infos[i];
        mvwprintw2c(nwin, line++, x, "%s: %s", "Type", fw_info->name);
        ret = fw_info->parse_version(&fw_info->fwInfo, ver_buff, fea_buff);
        mvwprintw2c(nwin, line, info_x, "%s: %s", "Version", ver_buff);
        mvwprintw2c(nwin, line++, info_x + 40, "%s: %s", "Feature", fea_buff);
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

