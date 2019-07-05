#include "windows.h"

static int tabInit(struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabExit(struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabHandleInput(struct Window *win, int ch)
{
    return HANDLE_NONE;
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
