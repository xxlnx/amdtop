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

extern int requeset_exit;
static int exitProgram(struct Window *win)
{
    int ret = 0;
    WINDOW *nwin = win->nwin;
    const int lines = 5, cols = 35;

    int starty = (win->layout.height - lines - 2) / 2;
    int startx = (win->layout.width - cols - 2) / 2;

    winframe(nwin, starty++, startx, starty + lines, startx + cols, "");
    mvwprintwc(nwin, starty++, startx + 1, COLOR_DEAFULT, "       Are you sure exit ?       ");
    mvwprintwc(nwin, starty++, startx + 1, COLOR_DEAFULT, "                                 ");
    mvwprintwc(nwin, starty++, startx + 1, COLOR_DEAFULT, "       Type 'y' exit             ");
    mvwprintwc(nwin, starty++, startx + 1, COLOR_DEAFULT, "       Other continue.           ");
    wrefresh(nwin);

    timeout(5000);
    if (getch() == 'y') {
        requeset_exit = 1;
    } else {
        timeout(100);
        ret = WindowExit(win);
        if (ret)
            return ret;
        ret = WindowClear(win);
        if (ret)
            return ret;
        ret = WindowInit(win);
        if (ret)
            return ret;
    }

    return ret;
}

static int mainHandleInput(struct Window *win, int ch)
{
    int ret = 0;

    switch (ch) {
        case 'q':
            ret = exitProgram(win);
            if (ret)
                return ret;
            break;
        default:
            break;
    }

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
