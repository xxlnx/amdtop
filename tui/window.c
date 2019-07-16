#include <stdint.h>
#include <string.h>
#include "window.h"

#define WINDOW_DEVICE_HEIGHT  (6)
#define WINDOW_TAB_WIDTH      (20)
#define WINDOW_STATUS_HEIGH   (3)
#define DEFAULT_WINDOW_SIZE   (80) /* 95  full window size */

struct WindowContext *AllocWindowContext(void)
{
    return xAlloc(sizeof(struct WindowContext));
}

void FreeWindowContext(struct WindowContext *ctx)
{
    endwin();
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
    timeout(200);

    return 0;
}

int InitWinLayout(struct WindowContext *ctx)
{
    struct Window *win = NULL;
    struct WindowLayout *deviceLayout = &ctx->wins[WIN_TYPE_DEVICE]->layout;
    struct WindowLayout *tabLayout    = &ctx->wins[WIN_TYPE_TAB   ]->layout;
    struct WindowLayout *mainLayout   = &ctx->wins[WIN_TYPE_MAIN  ]->layout;
    struct WindowLayout *statusLayout = &ctx->wins[WIN_TYPE_STATUS]->layout;

    ctx->width = COLS * DEFAULT_WINDOW_SIZE / 100;
    ctx->height = LINES * DEFAULT_WINDOW_SIZE / 100;
    ctx->startx = (COLS - ctx->width) / 2;
    ctx->starty = (LINES - ctx->height) / 2;

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

    for (int i = 0 ; i < WIN_TYPE_COUNT; i++) {
        win = ctx->wins[i];
        if (win->nwin)
            delwin(win->nwin);
        win->nwin = newwin(win->layout.height, win->layout.width,
                           ctx->starty + win->layout.starty, ctx->startx + win->layout.startx);
        win->ctx = ctx;
    }

    return 0;
}

static void DrawMainWindowBorder(struct WindowContext *ctx)
{
    attron(COLOR_PAIR(COLOR_DEAFULT));
    /* */
    for (int y = ctx->starty - 1; y < ctx->starty + ctx->height; y++)
        for (int x = ctx->startx - 1; x < ctx->startx + ctx->width; x++)
            mvaddch(y, x, ' ');
    /* Main Window Border */
    mvwhline(stdscr, ctx->starty - 1, ctx->startx - 1, 0, ctx->width + 2);
    mvwhline(stdscr, ctx->starty + ctx->height, ctx->startx - 1 , 0, ctx->width + 2);
    mvwvline(stdscr, ctx->starty - 1, ctx->startx - 1, 0, ctx->height + 2);
    mvwvline(stdscr, ctx->starty - 1, ctx->startx + ctx->width, 0, ctx->height + 2);
    mvwaddch(stdscr, ctx->starty - 1, ctx->startx - 1, ACS_ULCORNER);
    mvwaddch(stdscr, ctx->starty + ctx->height, ctx->startx - 1, ACS_LLCORNER);
    mvwaddch(stdscr, ctx->starty - 1, ctx->startx + ctx->width, ACS_URCORNER);
    mvwaddch(stdscr, ctx->starty + ctx->height, ctx->startx + ctx->width, ACS_LRCORNER);
    attroff(COLOR_PAIR(COLOR_DEAFULT));

}

int InitMainWindow(struct WindowContext *ctx)
{
    int ret = 0;

    if (ctx->hasColor) {
        DrawMainWindowBorder(ctx);
    }

    for (int i = 0 ; i < WIN_TYPE_COUNT; i++) {
        ret = WindowClear(ctx->wins[i]);
        if (ret)
            return ret;
    }
    refresh();

    return 0;
}

static struct Color colors[] = {
    {0, 0,  0,  0},
    {COLOR_DEAFULT,         COLOR_BLACK,    COLOR_WHITE,    A_NORMAL},
    {COLOR_BACKGROUD,       COLOR_WHITE,    COLOR_BLACK,    A_NORMAL},
    {COLOR_TITLE,           COLOR_BLUE,     COLOR_WHITE,    A_BOLD  },
    {COLOR_TAB_ACITVE,      COLOR_WHITE,    COLOR_BLUE,     A_BOLD  },
    {COLOR_TAB_INACTIVE,    COLOR_WHITE,    COLOR_BLUE,     A_NORMAL},
    {COLOR_LABEL_NAME,      COLOR_BLACK,    COLOR_WHITE,    A_NORMAL},
    {COLOR_LABEL_VALUE,     COLOR_BLUE,     COLOR_WHITE,    A_NORMAL},
    {COLOR_BLUE_COLOR,      COLOR_BLUE,     COLOR_WHITE,    A_NORMAL},
    {COLOR_YELLOW_COLOR,    COLOR_YELLOW,   COLOR_WHITE,    A_NORMAL},
    {COLOR_GREEN_COLOR,     COLOR_GREEN,    COLOR_WHITE,    A_NORMAL},
    {COLOR_RED_COLOR,       COLOR_RED,      COLOR_WHITE,    A_NORMAL},
};

int InitColor(struct WindowContext *ctx)
{
    if (!ctx->hasColor)
        return 0;

    ctx->color = colors;

    for (int i = COLOR_DEAFULT; i < COLOR_COUNT; i++) {
        init_pair(colors[i].pair, colors[i].fg, colors[i].bg);
    }

    if (ctx->hasColor)
        wattrset(stdscr, COLOR_PAIR(COLOR_DEAFULT));

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

int WindowsUpdateUi(struct WindowContext *ctx)
{
    int ret = 0;
    ret = InitWinLayout(ctx);
    if (ret)
        return ret;

    ret = InitMainWindow(ctx);
    if (ret)
        return ret;

    ret = WindowsInit(ctx);
    if (ret)
        return ret;

    return ret;
}

int InitWindowContext(struct WindowContext *ctx)
{
    int ret = 0;

    ret = InitColor(ctx);
    if (ret)
        return ret;

    ret = SetupWindows(ctx);
    if (ret)
        return ret;

    ret = WindowsUpdateUi(ctx);
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

int WindowsDispatchInput(struct WindowContext *ctx, int ch)
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

int WindowsInit(struct WindowContext *ctx)
{
    int ret = 0;
    struct Window *win = NULL;
    for (int i = 0; i < WIN_TYPE_COUNT; i++) {
        win = ctx->wins[i];
        if (!win)
            continue;
        ret = WindowInit(win);
        if (ret)
            return ret;
    }

    return 0;
}

int WindowsExit(struct WindowContext *ctx)
{
    int ret = 0;
    struct Window *win = NULL;
    for (int i = 0; i < WIN_TYPE_COUNT; i++) {
        win = ctx->wins[i];
        if (!win)
            continue;
        ret = WindowExit(win);
        if (ret)
            return ret;
    }
    return -1;
}

int WindowsUpdate(struct WindowContext *ctx, uint32_t flags)
{
    int ret = 0;
    struct Window *win = NULL;
    for (int i = 0; i < WIN_TYPE_COUNT; i++) {
        win = ctx->wins[i];
        if (!win)
            continue;
        ret = WindowUpdate(win, flags);
        if (ret)
            return ret;
    }
    return 0;
}

int WindowClear(struct Window *win)
{
    int ret = 0;
    if (win->ctx->hasColor)
        wattrset(win->nwin, COLOR_PAIR(COLOR_DEAFULT)| colors[COLOR_DEAFULT].attrs);
    wclear(win->nwin);
    winclear(win->nwin);
    box(win->nwin, 0, 0);

    return ret;
}


void winclear(WINDOW* nwin)
{
    int x, y, ymax, xmax;
    getmaxyx(nwin, ymax, xmax);
    for (y = 0; y < ymax; y++)
        for (x = 0; x < xmax; x++)
            waddch(nwin, ' ');

}
int mvwprintwc(WINDOW *win, int y, int x, enum ColorType colorType, const char *fmt, ...)
{
    int ret = 0;
    va_list args;
    va_start(args, fmt);

    wmove(win, y, x);

    wattron(win, COLOR_PAIR(colorType) | colors[colorType].attrs);
    ret = vwprintw(win, fmt, args);
    wattroff(win, COLOR_PAIR(colorType) | colors[colorType].attrs);

    va_end(args);

    return ret;
}

int mvwprintw2c(WINDOW *win, int y, int x, const char *fmt, ...)
{
    int ret = 0;
    char *s1, *s2, *f1, *f2, *ptr = strdup(fmt);
    enum ColorType colorType;
    va_list args;
    const size_t ptr_len = strlen(ptr) + 1;

    /* Retrive args */
    va_start(args, fmt);
    f2 = strstr(fmt, ":") + 1;
    f1 = strncat(strtok(ptr, ": "), ":", ptr_len);
    s1 = va_arg(args, char *);
    s2 = va_arg(args, char *);

    /* Print label name */
    wmove(win, y, x);
    colorType = COLOR_LABEL_NAME;
        wattron(win, COLOR_PAIR(colorType) | colors[colorType].attrs);
    ret += wprintw(win, f1, s1);
    xFree(f1);
        wattroff(win, COLOR_PAIR(colorType) | colors[colorType].attrs);

    /* Print label value */
    colorType = COLOR_LABEL_VALUE;
        wattron(win, COLOR_PAIR(colorType) | colors[colorType].attrs);
    ret += wprintw(win, f2, s2);
        wattroff(win, COLOR_PAIR(colorType) | colors[colorType].attrs);

    va_end(args);
    return ret;
}

uint32_t WindowGetColor(struct WindowContext *ctx, enum ColorType colorType)
{
    struct Color *color = ctx->color;

    if (!color)
        return 0;

    if (colorType < COLOR_DEAFULT || colorType > COLOR_COUNT)
        return 0;

    return COLOR_PAIR(colorType) | color[colorType].attrs;
}

