#include "window.h"
#include "tabinfo.h"

static int mainInit(struct Window *win)
{
    int ret = 0;
    struct TabInfo *tabInfo = NULL;
    struct Device *device = getAcitveDevice();
    WINDOW *nwin = win->nwin;

    tabInfo = getTabInfoByIndex(getContext()->activeTabID);

    switch (tabInfo->id) {
        case TabID_SYSTEM:
        case TabID_ABOUT:
            break;
        default:
            if (!DeviceDriverisLoaded(device)) {
                mvwprintwc(nwin, win->layout.height / 2, (win->layout.width - 30) / 2, COLOR_RED_COLOR,
                    "%10s: Driver is not Loaded", tabInfo->labelName);
                wrefresh(nwin);
                return 0;
            }
            break;
    }

    ret = TabinfoInit(tabInfo, win);
    if (ret)
        return ret;

    return ret;
}

static int mainExit(struct Window *win)
{
    int ret = 0;
    struct TabInfo *tabInfo = NULL;
    tabInfo = getTabInfoByIndex(getContext()->activeTabID);
    TabinfoExit(tabInfo, win);
    return ret;
}

static int mainHandleInput(struct Window *win, int ch)
{
    return HANDLE_NONE;
}

static int mainControl(struct Window *win, int cmd, void *data)
{
    int ret = 0;
    return ret;
}

static int mainUpdate(struct Window *win, uint32_t flags)
{
    int ret = 0;
    struct TabInfo *tabInfo = getTabInfoByIndex(getContext()->activeTabID);
    uint64_t currnet_ns = getcurrent_ns();

    if (DeviceDriverisLoaded(getAcitveDevice())) {
        if (tabInfo->period > 0 && currnet_ns >= tabInfo->last_update_time + tabInfo->period) {
            ret = TabinfoUpdate(tabInfo, win);
            if (ret)
                return ret;
            tabInfo->last_update_time = getcurrent_ns();
        }
    }

    return ret;
}

static struct WindowOps mainOps = {
    .init = mainInit,
    .exit = mainExit,
    .control = mainControl,
    .update = mainUpdate,
    .handle_input = mainHandleInput,
};

struct Window MainWindow = {
    .name = "Main",
    .id = WIN_TYPE_MAIN,
    .ops = &mainOps,
    .period = MS_2_NS(100),
};
