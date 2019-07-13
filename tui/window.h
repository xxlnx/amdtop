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

struct WindowContext {
    uint32_t width, height;
    uint32_t startx, starty;
    struct Window *wins[WIN_TYPE_COUNT];
    bool hasColor;
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

int WindowInit(struct Window *win);
int WindowExit(struct Window *win);
int WindowUpdate(struct Window *win, uint32_t flags);
int WindowInput(struct Window *win, int ch);
int WindowControl(struct Window *win, int cmd, void *data);
int WindowDispatchInput(struct WindowContext *ctx, int ch);

#endif
