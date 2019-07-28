#include "tabinfo.h"

static int tabAboutInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;

    int ret = 0;
    int line = 1;
    int startx = 2, starty = 0;
    int endx = win->layout.width - 2;
    int x  = 3;

    x = (win->layout.width - 60) / 2;
    starty = line;
    line = line + 2;
    mvwprintwc(nwin, line++, x, COLOR_DEAFULT, "The amdtop is a free Linux software.");
    mvwprintwc(nwin, line++, x, COLOR_DEAFULT, "that collects some hardware information about amdgpu.");
    line++;
    winframe(nwin, starty, startx, line, endx, "");
    line = line + 2;

    x = (win->layout.width - 40) / 2;
    starty = line;
    line = line + 2;
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "Author", "Wang Yang");
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "E-Mail", "wangyang7902@gmail.com");
    line++;
    winframe(nwin, starty, startx, line, endx, "Author");
    line = line + 2;

    x = (win->layout.width - 40) / 2;
    starty = line;
    line++;
    mvwprintwc(nwin, line++, x + 5, COLOR_BLUE_COLOR, "AMDTOP");
    line++;
    mvwprintw2c(nwin, line++, x, "%-10s: %d.%d.%d", "Version",
        PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "Revsion", BUILD_GIT_VERSION);
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "Compiler", C_COMPILER_ID);
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "ARCH", SYSTEM_PROCESSOR);
    line++;
    winframe(nwin, starty, startx, line, endx, "Version");

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
    .update = 0,
};

