#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <utils/utils.h>
#include <utils/list.h>
#include <ncurses.h>
#include <stdint.h>

struct WindowContext;
struct Window;

struct WindowOps {
    int (*init)(struct Window *win);
    int (*exit)(struct Window *win);
    int (*control)(struct Window *win, int cmd, void *data);
    int (*update)(struct Window *win, uint32_t flags);
    int (*handle_input) (struct Window *win, int ch);
};

struct WindowLayout {
    uint32_t starty, startx, width, height;
};

struct Window {
    const char *name;
    int id;
    struct WindowOps *ops;
    struct WindowLayout layout;
    struct WindowContext *ctx;
    WINDOW *nwin;
};

enum WIN_TYPE {
    WIN_TYPE_DEVICE = 0,
    WIN_TYPE_TAB,
    WIN_TYPE_MAIN,
    WIN_TYPE_STATUS,
    WIN_TYPE_COUNT,
};

enum ColorType {
    COLOR_UNKNOW = 0,
    COLOR_DEAFULT,
    COLOR_BACKGROUD,
    COLOR_TITLE,
    COLOR_TAB_ACITVE,
    COLOR_TAB_INACTIVE,
    COLOR_LABEL_NAME,
    COLOR_LABEL_VALUE,
    COLOR_RED_COLOR,
    COLOR_BLUE_COLOR,
    COLOR_YELLOW_COLOR,
    COLOR_GREEN_COLOR,
    COLOR_COUNT,
};

struct Color {
    uint16_t pair;
    uint16_t fg;
    uint16_t bg;
    uint32_t attrs;
};

struct WindowContext {
    uint32_t width, height;
    uint32_t startx, starty;
    struct Window *wins[WIN_TYPE_COUNT];
    bool hasColor;
    struct Color *color;
};

enum HANDLE_TYPE {
    HANDLE_NONE = (0 << 0),
    HANDLE_HANDLED = (1 << 0),
};

extern struct Window DeviceWindow;
extern struct Window TabWindow;
extern struct Window MainWindow;
extern struct Window StatusWindow;

struct WindowContext *AllocWindowContext(void);
void FreeWindowContext(struct WindowContext *ctx);

int InitNcurse(struct WindowContext *ctx);
int InitWindowContext(struct WindowContext *ctx);
int InitWinLayout(struct WindowContext *ctx);
int InitMainWindow(struct WindowContext *ctx);
int WindowsUpdateUi(struct WindowContext *ctx);

int WindowInit(struct Window *win);
int WindowExit(struct Window *win);
int WindowUpdate(struct Window *win, uint32_t flags);
int WindowInput(struct Window *win, int ch);
int WindowControl(struct Window *win, int cmd, void *data);
int WindowsDispatchInput(struct WindowContext *ctx, int ch);
int WindowsInit(struct WindowContext *ctx);
int WindowsExit(struct WindowContext *ctx);
int WindowsUpdate(struct WindowContext *ctx, uint32_t flags);
uint32_t WindowGetColor(struct WindowContext *ctx, enum ColorType colorType);
void winclear(WINDOW* nwin);
int mvwprintwc(WINDOW *win, int y, int x, enum ColorType colorType, const char *fmt, ...);
int mvwprintw2c(WINDOW *win, int y, int x, const char *fmt, ...);

#endif
