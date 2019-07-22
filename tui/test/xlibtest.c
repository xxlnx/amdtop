#include <X11/Xlib.h>
#include <stdlib.h>
#include "utils/utils.h"

int main(int argc, char *argv[])
{
    Display *display = NULL;
    char *disp = getenv("DISPLAY");
    int screen_count = 0, display_count = 0;
    display = XOpenDisplay(disp);
    INFO("display = %#x\n", display);
    screen_count = ScreenCount(display);
    INFO("display name = %s\n", DisplayString(display));
    INFO("screen count = %d\n", ScreenCount(display));
    INFO("Serer Vendor = %s\n", ServerVendor(display));
    for (int i = 0 ; i < 2; i ++) {
        INFO("display (%d) %d x  %d \n", i, DisplayWidth(display, i), DisplayHeight(display, i));
        INFO("display (%d) %dmm x  %dmm \n", i, DisplayWidthMM(display, i), DisplayHeightMM(display, i));
    }

    if (!disp)
        XCloseDisplay(display);
    return 0;
}
