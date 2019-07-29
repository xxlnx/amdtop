#include <string.h>
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
                mvwprintwc_center(nwin, COLOR_RED_COLOR, "%10s: Driver is not Loaded!", tabInfo->labelName);
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

    timeout(-1);
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

const char *helper_str[] = {
    "Help:",
    "F3:",
    "   switch previous device",
    "F4:",
    "   switch next device",
    "Ctrl-P / PageUP:",
    "   switch previous tabpage",
    "Ctrl-N / PageDown:",
    "   switch next tabpage",
    "? / h :",
    "   show this information",
    "q: ",
    "   exit program",
    "",
    "",
    "type any key exit this help!",
};

static int helperHandler(struct Window *win)
{
    int ret = 0;
    uint32_t  max_len = 0;
    uint32_t len = 0;
    int starty, startx;
    WINDOW *nwin = win->nwin;

    for (int i = 0; i < ARRAY_SIZE(helper_str); i++) {
        len = strlen(helper_str[i]);
        max_len = MAX(max_len, len);
    }

    starty = (win->layout.height -  ARRAY_SIZE(helper_str)) / 2;
    startx = (win->layout.width - max_len - 2) / 2;

    if (startx < 0 || starty < 0)
        return 0;

    WindowClear(win);

    for (int i = 0; i < ARRAY_SIZE(helper_str); i++ ) {
        mvwprintwc(nwin, starty + i, startx, COLOR_DEAFULT, "%s", helper_str[i]);
    }

    winframe(nwin, starty - 2, startx - 2,
        starty + ARRAY_SIZE(helper_str) + 4,
        startx + max_len + 4, "HELP");

    wrefresh(nwin);

    timeout(-1);
    getch();

    ret = WindowExit(win);
    if (ret)
        return  ret;
    ret = WindowClear(win);
    if (ret)
        return ret;
    ret = WindowInit(win);
    if (ret)
        return ret;

    timeout(200);

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
        case 'h':
        case '?':
            ret = helperHandler(win);
            if (ret)
                return ret;
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
