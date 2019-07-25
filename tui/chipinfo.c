#include "tabinfo.h"

static struct Device *current_device = NULL;

#define MAX_IP_INSTANCE (4)

struct ip_info {
    char *name;
    enum GpuHwIpType ipType;
    uint32_t count;
    struct GpuHwIPInfo hwIpInfos[MAX_IP_INSTANCE];
};

struct ip_info ipinfos[] = {
    {"GFX", GPU_HW_IP_GFX},
    {"COMP", GPU_HW_IP_COMPUTE},
    {"DMA", GPU_HW_IP_DMA},
    {"UVD", GPU_HW_IP_UVD},
    {"VCE", GPU_HW_IP_VCE},
    {"UVD_ENC", GPU_HW_IP_UVD_ENC},
    {"VCN_DEC", GPU_HW_IP_VCN_DEC},
    {"VCN_ENC", GPU_HW_IP_VCN_ENC},
};

#define IP_INFO_COUNT ARRAY_SIZE(ipinfos)

static int getChipInfo(void)
{
    int ret = 0;
    struct Device *device = getAcitveDevice();
    struct ip_info *ipinfo = NULL;
    if (current_device == device)
        return 0;

    for (int i = 0; i < IP_INFO_COUNT; i++) {
        ipinfo = &ipinfos[i];
        ret = gpuQueryHwIpCount(device->gpuDevice, ipinfo->ipType, &ipinfo->count);
        if (ret)
            return ret;
        for (int j = 0; j < MIN(ipinfo->count, MAX_IP_INSTANCE); j++) {
            ret = gpuQueryHwIpInfo(device->gpuDevice, ipinfo->ipType, j, &ipinfo->hwIpInfos[j]);
            if (ret)
                return ret;
        }
    }

    return ret;
}

static uint32_t calc_ring_count(uint32_t rings)
{
    uint32_t  count = 0;

    for (int i = 0; i < sizeof(rings) * 8; i ++) {
        if (rings & (1 << i))
            count++;
    }

    return count;
}

static int tabChipInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    int ret = 0;
    int x = 10, info_x = 20;
    int line = 1;
    struct ip_info *ipInfo = NULL;
    struct GpuHwIPInfo *hwIpInfo = NULL;

    ret = getChipInfo();
    if (ret)
        return ret;

    for (int i = 0; i < IP_INFO_COUNT; i++) {
        ipInfo = &ipinfos[i];
        mvwprintw2c(nwin, line++, x, "%5s : %d instance", ipInfo->name, ipInfo->count);
        if (ipInfo->count == 1) {
            hwIpInfo = &ipInfo->hwIpInfos[0];
            mvwprintw2c(nwin, line++, info_x, "%-10s: %d.%d", "Version", hwIpInfo->version_major, hwIpInfo->version_minor);
            mvwprintw2c(nwin, line++, info_x, "%-10s: %d", "Rings", calc_ring_count(hwIpInfo->avaiable_rings));
            mvwprintw2c(nwin, line++, info_x, "%-10s: start %5d, size %5d", "IB Align", hwIpInfo->ib_start_aligment, hwIpInfo->ib_size_alignment);
        } else {
            for (int j = 0; j < ipInfo->count; j++) {
                hwIpInfo = &ipInfo->hwIpInfos[j];
                mvwprintw2c(nwin, line++, info_x, "%-10s: [%d] %d.%d", "Version", j, hwIpInfo->version_major, hwIpInfo->version_minor);
                mvwprintw2c(nwin, line++, info_x, "%-10s: [%d] %d", "Rings", j, calc_ring_count(hwIpInfo->avaiable_rings));
                mvwprintw2c(nwin, line++, info_x, "%-10s: start %5d, size %5d", "IB Align", j, hwIpInfo->ib_start_aligment, hwIpInfo->ib_size_alignment);
            }
        }
    }

    wrefresh(nwin);
    return ret;
}

static int tabChipInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabChipInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo chipInfo = {
    .id = TabID_CHIP,
    .name = "chipInfo",
    .labelName = "Chip",
    .init = tabChipInfoInit,
    .exit = tabChipInfoExit,
};

