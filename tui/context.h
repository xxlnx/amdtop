#ifndef __GLOBAL_CONTEXT__
#define __GLOBAL_CONTEXT__

#include "window.h"
#include "device.h"

struct Context {
    struct WindowContext *wctx;
    struct DeviceContext *dctx;
    int32_t activeDeviceID;
    int32_t activeTabID;
};

extern struct Context gloalContext;
struct Context *getContext(void);
struct Device *getAcitveDevice(void);
#endif