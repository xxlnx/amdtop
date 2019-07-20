#include "window.h"
#include "tabinfo.h"

static int mainInit(struct Window *win)
{
    int ret = 0;
    struct TabInfo *tabInfo = NULL;
    tabInfo = getTabInfoByIndex(getContext()->activeTabID);
    ret = TabinfoInit(tabInfo, win);
    if (ret)
        return ret;
    return ret;
}

static int mainExit(struct Window *win)
{
    int ret = 0;
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
    ret = TabinfoUpdate(tabInfo, win);
    if (ret)
        return ret;
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
};
