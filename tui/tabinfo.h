#ifndef __TABINFO_H__
#define __TABINFO_H__

#include <stdint.h>
#include "window.h"
#include "device.h"
#include "context.h"

struct TabInfo {
    int32_t id;
    const char *name;
    const char *labelName;
    int (*init)(struct TabInfo *tabinfo, struct Window *win);
    int (*exit)(struct TabInfo *tabinfo, struct Window *win);
    int (*update)(struct TabInfo *tabinfo, struct Window *win);
    void *data;
};

struct TabInfo * getTabInfoByIndex(uint32_t index);
uint32_t getTabInfoCount(void);
int TabinfoInit(struct TabInfo *tabinfo, struct Window *win);
int TabinfoExit(struct TabInfo *tabinfo, struct Window *win);
int TabinfoUpdate(struct TabInfo *tabinfo, struct Window *win);

#endif