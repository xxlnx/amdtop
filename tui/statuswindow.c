#include "windows.h"

static int statusInit(struct Window *win)
{
    int ret = 0;
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
    struct Device *dev = getAcitveDevice();
    if (!dev)
        return 0;
    mvwprintw(win->nwin, 1, 1, "%d:%s [%04x:%02x:%02x.%d]",
        getContext()->activeDeviceID, dev->deviceName,
        dev->domain, dev->bus, dev->dev, dev->func);
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
};
