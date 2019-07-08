#include <stdio.h>
#include <ncurses.h>
#include "window.h"
#include "device.h"
#include "context.h"

int main(int argc, char *argv[])
{
    int ch = 0;
    int ret = 0;

    struct Context *context = getContext();
    struct DeviceContext *dctx = NULL;
    struct WindowContext *wctx = NULL;

    dctx = AllocDeviceContext();
    InitDeviceContext(dctx);
    context->dctx = dctx;
    wctx = AllocWindowContext();
    context->wctx = wctx;
    InitNcurse(wctx);
    InitWindowContext(wctx);

    while (ch = getch()) {
        switch (ch) {
            case 'q':
                goto out;
                break;
            case ERR:
                ret = WindowsUpdate(wctx, 0);
                break;
            default:
                ret = WindowsDispatchInput(wctx, ch);
                break;
        }
    }
out:
    FreeWindowContext(wctx);
    FreeDeviceContext(dctx);
    return 0;
}
