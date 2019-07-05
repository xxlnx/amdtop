#include "windows.h"

static int deviceInit(struct Window *win)
{
    int ret = 0;

    return ret;
}

static int deviceExit(struct Window *win)
{
    int ret = 0;
    return ret;
}

static int deviceHandleInput(struct Window *win, int ch)
{
    return HANDLE_NONE;
}

static int deviceControl(struct Window *win, int cmd, void *data)
{
    int ret = 0;
    return ret;
}

static int deviceUpdate(struct Window *win, uint32_t flags)
{
    int ret = 0;
    return ret;
}

static struct WindowOps deviceOps = {
    .init = deviceInit,
    .exit = deviceExit,
    .control = deviceControl,
    .update = deviceUpdate,
    .handle_input = deviceHandleInput,
};

struct Window DeviceWindow = {
    .name = "Device",
    .id = WIN_TYPE_DEVICE,
    .ops = &deviceOps,
};
