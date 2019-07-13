#include "window.h"
#include "context.h"
#include "tabinfo.h"

static struct WindowContext *wctx = NULL;
static struct DeviceContext *dctx = NULL;
static uint32_t  gTabCount = 0;

static void updateActiveTab(struct Window *win, int index)
{
    const uint32_t x = 1, y = 1;
    for (int i = 0 ; i < gTabCount; i ++){
        if (i == index)
            mvwprintw(win->nwin, y + i, x, "*");
        else
            mvwprintw(win->nwin, y + i, x, " ");
    }
    wrefresh(win->nwin);
}

static int tabInit(struct Window *win)
{
    int ret = 0;
    int line = 1;
    struct TabInfo *tabInfo = NULL;
    wctx = getContext()->wctx;
    dctx = getContext()->dctx;
    gTabCount = getTabInfoCount();
    for (int i = 0; i < gTabCount; i++) {
        tabInfo = getTabInfoByIndex(i);
        tabInfo->id = i;
        mvwprintw(win->nwin, line, 3, "%d. %s", tabInfo->id + 1, tabInfo->labelName);
        line++;
    }
    wrefresh(win->nwin);
    updateActiveTab(win, getContext()->activeTabID);
    return ret;
}

static int tabExit(struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabHandleInput(struct Window *win, int ch)
{
    enum HANDLE_TYPE handleType = HANDLE_HANDLED;
    int32_t tabid = getContext()->activeTabID;
    struct Window *mainWin = wctx->wins[WIN_TYPE_MAIN];
    struct TabInfo *preInfo = NULL, *curInfo = NULL;
    int ret = 0;

    switch (ch) {
        case KEY_NPAGE:
        case 14: /* CTRL + N */
            if (++tabid >= gTabCount)
                tabid = 0;
            break;
        case KEY_PPAGE:
        case 16: /* CTRL + P */
            if (--tabid < 0)
                tabid = gTabCount - 1;
            break;
        default:
            handleType = HANDLE_NONE;
            break;
    }

    if (tabid != getContext()->activeTabID) {
        /* need clear and refresh main window */
        preInfo = getTabInfoByIndex(getContext()->activeTabID);
        curInfo = getTabInfoByIndex(tabid);
        ret = TabinfoExit(preInfo, mainWin);
        if (ret)
            return ret;
        ret = WindowClear(mainWin);
        if (ret)
            return ret;
        ret = TabinfoInit(curInfo, mainWin);
        if (ret)
            return ret;
        updateActiveTab(win, tabid);
        getContext()->activeTabID = tabid;
    }

    return handleType;
}

static int tabControl(struct Window *win, int cmd, void *data)
{
    int ret = 0;
    return ret;
}

static int tabUpdate(struct Window *win, uint32_t flags)
{
    int ret = 0;
    return ret;
}

static struct WindowOps tabOps = {
    .init = tabInit,
    .exit = tabExit,
    .control = tabControl,
    .update = tabUpdate,
    .handle_input = tabHandleInput,
};

struct Window TabWindow = {
    .name = "Tab",
    .id = WIN_TYPE_TAB,
    .ops = &tabOps,
};
