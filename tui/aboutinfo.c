#include <string.h>
#include "tabinfo.h"

const char* app_logo[] = {
"             __  __   _____    _______    ____    _____  ",
"     /\\     |  \\/  | |  __ \\  |__   __|  / __ \\  |  __ \\ ",
"    /  \\    | \\  / | | |  | |    | |    | |  | | | |__) |",
"   / /\\ \\   | |\\/| | | |  | |    | |    | |  | | |  ___/ ",
"  / ____ \\  | |  | | | |__| |    | |    | |__| | | |     ",
" /_/    \\_\\ |_|  |_| |_____/     |_|     \\____/  |_|     "
};

static int tabAboutInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;

    int ret = 0;
    int line = 1;
    int startx = 2, starty = 0;
    int endx = win->layout.width - 2;
    int x  = 3;

    x = (win->layout.width - 60) / 2;
    starty = line++;
    mvwprintwc(nwin, line++, x, COLOR_DEAFULT, "The amdtop is a free Linux software.");
    mvwprintwc(nwin, line++, x, COLOR_DEAFULT, "that collects some hardware information about amdgpu.");
    winframe(nwin, starty, startx, line, endx, "");
    line++;

    x = (win->layout.width - 40) / 2;
    starty = line++;
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "Author", "Wang Yang");
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "E-Mail", "wangyang7902@gmail.com");
    winframe(nwin, starty, startx, line, endx, "Author");
    line++;

    x = (win->layout.width - 40) / 2;
    starty = line++;
    mvwprintw2c(nwin, line++, x, "%-10s: %d.%d.%d", "Version",
        PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "Revsion", BUILD_GIT_VERSION);
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "Compiler", C_COMPILER_ID);
    mvwprintw2c(nwin, line++, x, "%-10s: %s", "ARCH", SYSTEM_PROCESSOR);
    winframe(nwin, starty, startx, line, endx, "Version");

    uint32_t max_size = 0;
    for (int i = 0; i < ARRAY_SIZE(app_logo); i++) {
        int size = strlen(app_logo[i]);
        if (size > max_size)
            max_size = size;
    }

    int logo_x  = (win->layout.width - max_size) / 2;
    int logo_y = (win->layout.height - line -  ARRAY_SIZE(app_logo)) / 2;
    line += logo_y;
    for (int i = 0; i < ARRAY_SIZE(app_logo); i++) {
        mvwprintwc(nwin, line++, logo_x, COLOR_LOGO_COLOR, "%s", app_logo[i]);
    }

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

