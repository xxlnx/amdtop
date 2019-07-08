#include "context.h"
#include "device.h"

struct Context gContext = {
    .wctx = NULL,
    .dctx = NULL,
    .activeDeviceID = 0,
    .activeTabID = 0,
};

struct Context *getContext(void)
{
    return &gContext;
}

struct Device *getAcitveDevice(void)
{
    struct DeviceContext *dctx = getContext()->dctx;

    if (!dctx)
        return NULL;

    if (getContext()->activeDeviceID > dctx->deviceCount)
        return NULL;

    return  dctx->devs[getContext()->activeDeviceID];
}
