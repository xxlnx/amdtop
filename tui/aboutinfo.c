#include "tabinfo.h"

static int tabAboutInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    struct Device * device = getAcitveDevice();

    int ret = 0;
    int xmax, ymax, x, y;
    int line = 1;

    getmaxyx(nwin, ymax, xmax);
    x = xmax / 5;
    mvwprintw2c(nwin, line++, x, "%s: %s", "Author", "Wang Yang");
    mvwprintw2c(nwin, line++, x, "%s: %s", "E-Mail", "wangyang7902@gmail.com");
    wrefresh(nwin);
    return ret;
}

static int tabAboutInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabAboutInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo aboutInfo = {
    .id = TabID_ABOUT,
    .name = "aboutInfo",
    .labelName = "About",
    .init = tabAboutInfoInit,
    .exit = tabAboutInfoExit,
};

