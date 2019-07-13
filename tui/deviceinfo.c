#include "tabinfo.h"

static int tabDeviceInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    struct Device * device = getAcitveDevice();
    int ret = 0;
    int xmax, ymax, x, y;
    int line = 1;

    getmaxyx(nwin, ymax, xmax);
    x = xmax / 5;
    mvwprintw2c(nwin, line++, x, "%s:%s", "DeviceName", device->deviceName);
    mvwprintw2c(nwin, line++, x, "%s:%s", "DriverName", device->driverName);
    wrefresh(nwin);

    return ret;
}

static int tabDeviceInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabDeviceInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo deviceInfo = {
    .name = "deviceInfo",
    .labelName = "DeviceInfo",
    .init = tabDeviceInfoInit,
    .exit = tabDeviceInfoExit,
};

