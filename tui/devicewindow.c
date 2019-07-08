#include <stdio.h>
#include <string.h>
#include "window.h"
#include "device.h"
#include "context.h"

static struct DeviceContext *dctx = NULL;
static struct WindowContext *wctx = NULL;

static int updateActiveDevice(struct Window *win)
{
    const uint32_t x = 1, y = 1;
    mvwprintw(win->nwin, y + getContext()->activeDeviceID, x,  "*");
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
       mvwprintw(win->nwin, line, 2, " %d: %-20s", i, dev->deviceName);
       MemClear(buf, sizeof(buf));
       size = snprintf(buf, MAX_NAME_SIZE, "[%04x:%02x:%02x:%d]", dev->domain, dev->bus, dev->dev, dev->func);
       int xmax = getmaxx(win->nwin);
       mvwprintw(win->nwin, line, xmax - size - 1, "%s", buf);
       line++;
    }
    updateActiveDevice(win);

    if (wctx->hasColor)
        wattroff(win->nwin, WindowGetColor(wctx, COLOR_DEAFULT));

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

    if (dctx->deviceCount <= 1)
        return HANDLE_NONE;

    switch (ch) {
        case KEY_F(3):
            getContext()->activeDeviceID--;
            if (getContext()->activeDeviceID < 0)
                getContext()->activeDeviceID = 0;
            updateActiveDevice(win);
            break;
        case KEY_F(4):
            getContext()->activeDeviceID++;
            if (getContext()->activeDeviceID >= dctx->deviceCount)
                getContext()->activeDeviceID = dctx->deviceCount - 1;
            updateActiveDevice(win);
            break;
        default:
            handleType = HANDLE_NONE;
            break;
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
    if (!dev)
        return ret;
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
