#include "window.h"

static int mainInit(struct Window *win)
{
    int ret = 0;
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
