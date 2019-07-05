#include <stdio.h>
#include <ncurses.h>
#include "windows.h"

int main(int argc, char *argv[])
{
    int ch = 0;
    int ret = 0;
    struct WindowContext *ctx = NULL;

    ctx = AllocWindowContext();
    InitNcurse(ctx);
    InitWindowContext(ctx);

    while (ch = getch()) {
        switch (ch) {
            case 'q':
                goto out;
                break;
            case ERR:
                break;
            default:
                ret = WindowDispatchInput(ctx, ch);
                break;
        }
    }
out:
    endwin();
    FreeWindowContext(ctx);
    return 0;
}
