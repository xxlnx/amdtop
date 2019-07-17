#include <asm/errno.h>
#include "tabinfo.h"
#include "context.h"

extern struct TabInfo aboutInfo;
extern struct TabInfo deviceInfo;
extern struct TabInfo driverInfo;

static struct TabInfo * tabinfoList [] = {
    &deviceInfo,
    &driverInfo,
    &aboutInfo,
};

#define TABINFO_COUNT ARRAY_SIZE(tabinfoList)

struct TabInfo * getTabInfoByIndex(uint32_t index)
{
    if (index >= TABINFO_COUNT)
        return NULL;
    return tabinfoList[index];
}

uint32_t getTabInfoCount(void)
{
    return TABINFO_COUNT;
}

int TabinfoInit(struct TabInfo *tabinfo, struct Window *win)
{
    int ret = 0;

    if (!tabinfo)
        return -EINVAL;
    if (tabinfo->init)
        ret = tabinfo->init(tabinfo, win);

    return ret;
}

int TabinfoExit(struct TabInfo *tabinfo, struct Window *win)
{
    int ret = 0;

    if (!tabinfo)
        return -EINVAL;
    if (tabinfo->exit)
        ret = tabinfo->exit(tabinfo, win);

    return ret;
}

int TabinfoUpdate(struct TabInfo *tabinfo, struct Window *win)
{
    int ret = 0;

    if (!tabinfo)
        return -EINVAL;
    if (tabinfo->update)
        ret = tabinfo->update(tabinfo, win);

    return ret;
}