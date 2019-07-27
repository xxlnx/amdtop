#include <stdio.h>
#include <string.h>
#include "window.h"
#include "device.h"
#include "context.h"

static struct DeviceContext *dctx = NULL;
static struct WindowContext *wctx = NULL;

static int updateActiveDevice(struct Window *win, int32_t index)
{
    const uint32_t x = 1, y = 1;
    for (int i = 0 ; i < dctx->deviceCount; i ++){
        if (i == index)
            mvwprintwc(win->nwin, y + i, x, COLOR_DEAFULT, "*");
        else
            mvwprintwc(win->nwin, y + i, x, COLOR_DEAFULT, " ");
    }
    wrefresh(win->nwin);
}

static int deviceInit(struct Window *win)
{
    int ret = 0;
    int line = 1;
    int size = 0;
    struct Device *dev;
    char buf[MAX_NAME_SIZE] = {0};
    dctx = getContext()->dctx;
    wctx = getContext()->wctx;

    for (int i = 0; i < dctx->deviceCount; i++) {
       dev = getDeviceByIndex(dctx, i);
       if (!dev)
           return 0;
       mvwprintwc(win->nwin, line, 2, COLOR_DEAFULT, " %d: %-20s", i, dev->deviceName);
       MemClear(buf, sizeof(buf));
       size = snprintf(buf, MAX_NAME_SIZE, "[%04x:%02x:%02x:%d]", dev->domain, dev->bus, dev->dev, dev->func);
       int xmax = getmaxx(win->nwin);
       mvwprintwc(win->nwin, line, xmax - size - 1, COLOR_LABEL_VALUE,  "%s", buf);
       line++;
    }

    updateActiveDevice(win, getContext()->activeDeviceID);
    wrefresh(win->nwin);

    return ret;
}

static int deviceExit(struct Window *win)
{
    int ret = 0;
    return ret;
}

static int deviceHandleInput(struct Window *win, int ch)
{
    enum HANDLE_TYPE handleType = HANDLE_HANDLED;
    int32_t deviceId = getContext()->activeDeviceID;

    if (dctx->deviceCount <= 1)
        return HANDLE_NONE;

    switch (ch) {
        case KEY_F(3):
            if (--deviceId <= 0)
                deviceId = 0;
            break;
        case KEY_F(4):
            if (++deviceId >= dctx->deviceCount)
                deviceId = dctx->deviceCount - 1;
            break;
        default:
            handleType = HANDLE_NONE;
            break;
    }
    if (deviceId != getContext()->activeDeviceID){
        updateActiveDevice(win, deviceId);
        getContext()->activeDeviceID = deviceId;
    }

    return handleType;
}

static int deviceControl(struct Window *win, int cmd, void *data)
{
    int ret = 0;
    return ret;
}

static int deviceUpdate(struct Window *win, uint32_t flags)
{
    int ret = 0;
    struct Device *dev = getAcitveDevice();
    struct Window *mainWin = wctx->wins[WIN_TYPE_MAIN];

    for (int i = 0; i < dctx->deviceCount; i++) {

        dev = getDeviceByIndex(dctx, i);
        if (!dev)
            return 0;

        if (!dev->driverisLoaded) {
            if (pci_device_has_kernel_driver(dev->pdev)) {
                ret = UpdateDeviceInfo(dev);
                if (ret)
                    return ret;
                ret = WindowInit(mainWin);
                if (ret)
                    return ret;
            }
        }
    }

    return ret;
}

static struct WindowOps deviceOps = {
    .init = deviceInit,
    .exit = deviceExit,
    .control = deviceControl,
    .update = deviceUpdate,
    .handle_input = deviceHandleInput,
};

struct Window DeviceWindow = {
    .name = "Device",
    .id = WIN_TYPE_DEVICE,
    .ops = &deviceOps,
};
