#include "window.h"
#include "device.h"
#include "context.h"
#include <errno.h>

static struct DeviceContext *dctx = NULL;
static struct WindowContext *wctx = NULL;

#define ALIGN(val, align)   (((val) + (align - 1)) & ~(align - 1))

static int update_status(struct Window *win)
{
    int ret = 0;
    size_t  size = 0;
    char buff[MAX_NAME_SIZE];
    struct GpuMemInfo gpuMemInfo;
    struct GpuSensorInfo sensorGpuLoadInfo;
    struct Device *dev = getAcitveDevice();
    if (!dev)
        return -EINVAL;
    ret = gpuQueryMemInfo(dev->gpuDevice, MemType_VRAM, &gpuMemInfo);
    if (ret)
        return ret;
    ret = gpuQuerySensorInfo(dev->gpuDevice, SensorType_GpuLoad, &sensorGpuLoadInfo);
    if (ret)
        return ret;
    WindowClear(win);
    size = snprintf(buff, MAX_NAME_SIZE, "[%ldMB/%ldMB Load:%3d%%]", gpuMemInfo.used >> 20, ALIGN(gpuMemInfo.total >> 20, 1024), sensorGpuLoadInfo.value);
    mvwprintw(win->nwin, 1, 1, "%d:%s [%04x:%02x:%02x.%d]",
              getContext()->activeDeviceID, dev->deviceName,
              dev->domain, dev->bus, dev->dev, dev->func);
    mvwprintw(win->nwin, 1, win->layout.width - size - 1, "%s", buff);
    wrefresh(win->nwin);
    return ret;
}

static int statusInit(struct Window *win)
{
    int ret = 0;
    dctx = getContext()->dctx;
    wctx = getContext()->wctx;
    ret = update_status(win);
    return ret;
}

static int statusExit(struct Window *win)
{
    int ret = 0;
    return ret;
}

static int statusHandleInput(struct Window *win, int ch)
{
    return HANDLE_NONE;
}

static int statusControl(struct Window *win, int cmd, void *data)
{
    int ret = 0;
    return ret;
}

static int statusUpdate(struct Window *win, uint32_t flags)
{
    int ret = 0;
    ret = update_status(win);
    return ret;
}

static struct WindowOps statusOps = {
    .init = statusInit,
    .exit = statusExit,
    .control = statusControl,
    .update = statusUpdate,
    .handle_input = statusHandleInput,
};

struct Window StatusWindow = {
    .name = "Status",
    .id = WIN_TYPE_STATUS,
    .ops = &statusOps,
};
