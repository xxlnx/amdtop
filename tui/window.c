#include <stdint.h>
#include "windows.h"

#define WINDOW_DEVICE_HEIGHT  (6)
#define WINDOW_TAB_WIDTH      (20)
#define WINDOW_STATUS_HEIGH   (2)
#define DEFAULT_WINDOW_SIZE   (95) /* 95  full window size */

struct WindowContext *AllocWindowContext(void)
{
    return xAlloc(sizeof(struct WindowContext));
}

void FreeWindowContext(struct WindowContext *ctx)
{
    xFree(ctx);
}

int InitNcurse(struct WindowContext *ctx)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    halfdelay(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    if (has_colors()) {
        ctx->hasColor = TRUE;
        start_color();
    } else {
        ctx->hasColor = FALSE;
    }
    ctx->width = COLS * DEFAULT_WINDOW_SIZE / 100;
    ctx->height = LINES * DEFAULT_WINDOW_SIZE / 100;
    ctx->startx = (COLS - ctx->width) / 2;
    ctx->starty = (LINES - ctx->height) / 2;

    return 0;
}

int InitWinLayout(struct WindowContext *ctx)
{
    struct WindowLayout *deviceLayout = &ctx->wins[WIN_TYPE_DEVICE]->layout;
    struct WindowLayout *tabLayout    = &ctx->wins[WIN_TYPE_TAB   ]->layout;
    struct WindowLayout *mainLayout   = &ctx->wins[WIN_TYPE_MAIN  ]->layout;
    struct WindowLayout *statusLayout = &ctx->wins[WIN_TYPE_STATUS]->layout;

    /* 1. first setup fix layout by ctx */
    deviceLayout->startx = 0;
    deviceLayout->starty = 0;
    deviceLayout->height = WINDOW_DEVICE_HEIGHT;
    deviceLayout->width = ctx->width;
    tabLayout->startx = 0;
    tabLayout->width = WINDOW_TAB_WIDTH;
    statusLayout->startx = 0;
    statusLayout->height = WINDOW_STATUS_HEIGH;
    statusLayout->width = ctx->width;

    /* 2. calc other layout by fix window */
    tabLayout->starty = deviceLayout->starty + deviceLayout->height;
    tabLayout->height = ctx->height - deviceLayout->height - statusLayout->height;
    mainLayout->starty = tabLayout->starty;
    mainLayout->startx = tabLayout->startx + tabLayout->width;
    mainLayout->height = tabLayout->height;
    mainLayout->width = ctx->width - tabLayout->width;
    statusLayout->starty = ctx->height - statusLayout->height;

    return 0;
}

int InitMainWindow(struct WindowContext *ctx)
{
    struct Window *win = NULL;

    refresh();
    for (int i = 0 ; i < WIN_TYPE_COUNT; i++) {
        win = ctx->wins[i];
        win->nwin = newwin(win->layout.height, win->layout.width,
            ctx->starty + win->layout.starty, ctx->startx + win->layout.startx);
    }

    return 0;
}

int SetupWindows(struct WindowContext *ctx)
{
    ctx->wins[WIN_TYPE_DEVICE] = &DeviceWindow;
    ctx->wins[WIN_TYPE_TAB   ] = &TabWindow;
    ctx->wins[WIN_TYPE_MAIN  ] = &MainWindow;
    ctx->wins[WIN_TYPE_STATUS] = &StatusWindow;

    return 0;
}

int InitWindowContext(struct WindowContext *ctx)
{
    int ret = 0;

    ret = SetupWindows(ctx);
    if (ret)
        return ret;

    ret = InitWinLayout(ctx);
    if (ret)
        return ret;

    ret = InitMainWindow(ctx);
    if (ret)
        return ret;

    return ret;
}

int WindowInit(struct Window *win)
{
   if (win->ops && win->ops->init)
       return win->ops->init(win);
   return 0;
}

int WindowExit(struct Window *win)
{
    if (win->ops && win->ops->exit)
        return win->ops->exit(win);
    return 0;
}

int WindowUpdate(struct Window *win, uint32_t flags)
{
    if (win->ops && win->ops->update)
        return win->ops->update(win, flags);
    return 0;
}

int WindowInput(struct Window *win, int ch)
{
    if (win->ops && win->ops->handle_input)
        return win->ops->handle_input(win, ch);
    return HANDLE_NONE;
}

int WindowControl(struct Window *win, int cmd, void *data)
{
    if (win->ops && win->ops->control)
        return win->ops->control(win, cmd, data);
    return 0;
}

int WindowDispatchInput(struct WindowContext *ctx, int ch)
{
    enum HANDLE_TYPE ret;
    struct Window *win = NULL;
    for (int i = 0; i < WIN_TYPE_COUNT; i++) {
        win = ctx->wins[i];
        if (!win)
            continue;
        ret = WindowInput(win, ch);
        if (ret == HANDLE_HANDLED)
            return ret;
    }
    return HANDLE_NONE;
}
