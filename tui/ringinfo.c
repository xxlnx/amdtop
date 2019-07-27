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
    {"VCN_JPEG", GPU_HW_IP_VCN_ENC},
};

#define IP_INFO_COUNT ARRAY_SIZE(ipinfos)

static int getRingInfo(void)
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

static int tabRingInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    int ret = 0;
    int x, info_x;
    int line = 1;
    struct ip_info *ipInfo = NULL;
    struct GpuHwIPInfo *hwIpInfo = NULL;

    ret = getRingInfo();
    if (ret)
        return ret;

    x = win->layout.width / 20;
    info_x = x + 13;

    for (int i = 0; i < IP_INFO_COUNT; i++) {
        ipInfo = &ipinfos[i];
        mvwprintw2c(nwin, line, x, "%10s : %d", ipInfo->name, ipInfo->count);
        mvwprintwc(nwin, line++, getcurx(nwin) + 1, COLOR_DEAFULT, "inst(s)");
        if (ipInfo->count == 1) {
            hwIpInfo = &ipInfo->hwIpInfos[0];
            mvwprintw2c(nwin, line++, info_x, "%-12s: %d.%d", "Version", hwIpInfo->version_major, hwIpInfo->version_minor);
            mvwprintw2c(nwin, line++, info_x, "%-12s: %d", "Rings", calc_ring_count(hwIpInfo->avaiable_rings));
            mvwprintw2c(nwin, line, info_x, "%-12s: ", "IB Align");
            mvwprintw2c(nwin, line, getcurx(nwin), "%s : %3d", "Start", hwIpInfo->ib_start_aligment);
            mvwprintw2c(nwin, line++, getcurx(nwin) + 5, "%s : %3d", "Size", hwIpInfo->ib_size_alignment);
        } else {
            for (int j = 0; j < ipInfo->count; j++) {
                hwIpInfo = &ipInfo->hwIpInfos[j];
                mvwprintw2c(nwin, line++, info_x, "%-12s: [%d] %d.%d", "Version", j, hwIpInfo->version_major, hwIpInfo->version_minor);
                mvwprintw2c(nwin, line++, info_x, "%-12s: [%d] %d", "Rings", j, calc_ring_count(hwIpInfo->avaiable_rings));
                mvwprintw2c(nwin, line, info_x, "%-12s: ", "IB Align");
                mvwprintw2c(nwin, line, getcurx(nwin), "%s : %3d", "Start", hwIpInfo->ib_start_aligment);
                mvwprintw2c(nwin, line++, getcurx(nwin) + 5, "%s : %3d", "Size", hwIpInfo->ib_size_alignment);
            }
        }
    }

    wrefresh(nwin);
    return ret;
}

static int tabRingInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabRingInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo ringInfo = {
    .id = TabID_RING,
    .name = "ringInfo",
    .labelName = "Ring",
    .init = tabRingInfoInit,
    .exit = tabRingInfoExit,
};

