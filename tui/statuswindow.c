#include "window.h"
#include "device.h"
#include "context.h"
#include <errno.h>

#define GPU_LOAD_THRESHOLD  (85)

static struct DeviceContext *dctx = NULL;
static struct WindowContext *wctx = NULL;

#define GPU_LOAD_BAR_WIDHT  (11)    /* strlen [Load:100%] */
static int update_status(struct Window *win)
{
    int ret = 0;
    size_t  size = 0;
    struct GpuSensorInfo sensorGpuLoadInfo;
    struct Device *device = getAcitveDevice();
    enum ColorType colorType = COLOR_DEAFULT;

    if (!DeviceDriverisLoaded(device))
        return 0;

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_GpuLoad, &sensorGpuLoadInfo);
    if (ret)
        return ret;

    colorType = sensorGpuLoadInfo.value > GPU_LOAD_THRESHOLD ? COLOR_RED_COLOR : COLOR_DEAFULT;
    mvwprintwc(win->nwin, 1, win->layout.width - GPU_LOAD_BAR_WIDHT - 1, COLOR_DEAFULT, "[Load:");
    mvwprintwc(win->nwin, 1, getcurx(win->nwin), colorType, "%3d%%", sensorGpuLoadInfo.value);
    mvwprintwc(win->nwin, 1, getcurx(win->nwin), COLOR_DEAFULT, "]");

    return ret;
}

static int print_device_info(struct Window *win, struct Device *device)
{
    WINDOW *nwin = win->nwin;

    mvwprintwc(nwin, 1, 1, COLOR_DEAFULT, "%d: [%04x:%02x:%02x.%d] %s",
              getContext()->activeDeviceID,
              device->domain, device->bus, device->dev, device->func,
              device->deviceName);

    if (DeviceDriverisLoaded(device))
        mvwprintwc(nwin, 1, getcurx(nwin) + 1, COLOR_DEAFULT, "-- %s",
            device->driverName);
    else
        mvwprintwc(nwin, 1, getcurx(nwin) + 1, COLOR_RED_COLOR, "No Driver");

    return 0;
}

static int statusInit(struct Window *win)
{
    int ret = 0;
    dctx = getContext()->dctx;
    wctx = getContext()->wctx;
    struct Device *device = getAcitveDevice();

    if (device) {
        ret = print_device_info(win, device);
        if (ret)
            return 0;
    } else {
       /* TODO no device found */
    }

    if (DeviceDriverisLoaded(device)) {
        ret = update_status(win);
        if (ret)
            return 0;
    }

    wrefresh(win->nwin);
    return 0;
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
    wrefresh(win->nwin);

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
    .period = MS_2_NS(500),
};
