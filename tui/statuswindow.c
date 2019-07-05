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
