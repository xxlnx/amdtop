#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "tabinfo.h"
#include "core/gpudridebug.h"

static struct Device *current_device = NULL;

#define MAX_RING_COUNT (20)
static struct AmdGpuRing gpuRings[MAX_RING_COUNT];
static int ring_count = 0;

static int getRingInfo(void)
{
    int ret = 0;
    struct Device *device = getAcitveDevice();
    if (device == current_device)
        return 0;

    ret = amdGpuRingInfo(device->gpuDevice, gpuRings, &ring_count);
    if (ret)
        return ret;

    return ret;
}

static int update_fence_info(struct Window *win)
{
    int ret = 0;
    struct Device *device = getAcitveDevice();
    struct GpuDevice *gpuDevice = device->gpuDevice;
    struct AmdGpuFenceInfo fenceInfo;
    WINDOW *nwin = win->nwin;
    int line = 2;
    int x = win->layout.width / 20;

/*    WindowClear(win);*/
    mvwprintwc(nwin, line++, x, COLOR_DEAFULT, "%-15s: %-8s  %-8s  t%-8s  %-8s  %-8s  %-8s  %-8s",
        "RingName",
        "signal",
        "emitted",
        "trail",
        "e-trail",
        "preempt",
        "reset",
        "both");

    for (int i = 0; i < ring_count; i++) {
        MemClear(&fenceInfo, sizeof(fenceInfo));
        ret = amdGpuQueryFenceInfo(&gpuRings[i], &fenceInfo);
        if (ret)
            return ret;
        mvwprintw2c(nwin, line++, x, "%-15s: %08x  %08x  %08x  %08x  %08x  %08x  %08x",
            gpuRings[i].shortname,
            fenceInfo.signaled,
            fenceInfo.emitted,
            fenceInfo.trailing_fence,
            fenceInfo.emitted_trial,
            fenceInfo.preempted,
            fenceInfo.reset,
            fenceInfo.both);
    }

    return ret;
}


static int tabFenceInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    int ret = 0;

    if (geteuid() != 0) {
        info->period = 0; /* disable refresh */
        const char* label = "Superuser permissions are Required";
        mvwprintwc_center(nwin, COLOR_RED_COLOR, label);
        wrefresh(nwin);
        return 0;
    }

    ret = getRingInfo();
    if (ret)
        return ret;

    ret = update_fence_info(win);
    if (ret)
        return ret;

    wrefresh(nwin);
    return ret;
}

static int tabFenceInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabFenceInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;

    ret = update_fence_info(win);
    if (ret)
        return ret;

    wrefresh(win->nwin);

    return ret;
}

struct TabInfo fenceInfo = {
    .id = TabID_FENCE,
    .name = "fenceInfo",
    .labelName = "Fence",
    .init = tabFenceInfoInit,
    .exit = tabFenceInfoExit,
    .update = tabFenceInfoUpdate,
    .period = MS_2_NS(500),
};

