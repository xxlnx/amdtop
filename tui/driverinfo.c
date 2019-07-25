#include <dirent.h>
#include <string.h>
#include <asm-generic/errno-base.h>
#include "tabinfo.h"

static struct GpuDriverInfo gpuDriverInfo;
static bool isDkms = false;
static struct moduleInfo {
    char version[MAX_NAME_SIZE] ;
    char srcversion[MAX_NAME_SIZE] ;
} modAmdkcl, modAmdgpu;


#define SYS_MODULE_PATH "/sys/module"

static bool isDkmsDriver(void)
{
    bool ret = false;
    DIR *dir = NULL;
    struct dirent *dent;
    dir = opendir(SYS_MODULE_PATH);
    while(dent = readdir(dir)) {
        if (dent->d_name[0] == '.')
            continue;
        if (!strncmp(dent->d_name, "amdkcl", 6))
            return true;
    }
    closedir(dir);
    return ret;
}

static int getModuleParam(const char *module, const char *filed, char *value, size_t *outsize)
{
    char buff[MAX_NAME_SIZE] = {0};
    int ret = 0;
    size_t size = 0;
    FILE *fp = NULL;
    size = snprintf(buff, MAX_NAME_SIZE, "%s/%s/%s", SYS_MODULE_PATH, module, filed);
    fp = fopen(buff, "r");
    if (!fp)
        return -EIO;

    size = fread(value, 1, MAX_NAME_SIZE - 1, fp);
    value[size - 1] = '\0';
    if (outsize)
        *outsize = size;
    fclose(fp);

    return ret;
}

static int getAllInfo(void)
{
    static int first = true;
    struct Device *device = getAcitveDevice();
    int ret = 0;
    if (!first)
        return 0;

    ret = gpuQueryDriverInfo(device->gpuDevice, &gpuDriverInfo);
    if (ret)
        return ret;

    ret = getModuleParam("amdgpu", "version", modAmdgpu.version, NULL);
    if (ret)
        return ret;
    ret = getModuleParam("amdgpu", "srcversion", modAmdgpu.srcversion, NULL);
    if (ret)
        return ret;

    isDkms = isDkmsDriver();
    if (isDkms) {
        ret = getModuleParam("amdkcl", "version", modAmdkcl.version, NULL);
        if (ret)
            return ret;
        ret = getModuleParam("amdkcl", "srcversion", modAmdkcl.srcversion, NULL);
        if (ret)
            return ret;
    }

    first = false;
    return ret;
}

static int tabDriverInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    int ret = 0;
    int x, y;
    int line = 1;

    ret = getAllInfo();
    if (ret)
        return ret;

    x = win->layout.width / 20;
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "Driver Name:", gpuDriverInfo.driver_name);
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "Driver Desc:", gpuDriverInfo.desc);
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "Driver Date:", gpuDriverInfo.date);
    mvwprintw2c(nwin, line++, x, "%-20s: %d.%d", "Driver Version:", gpuDriverInfo.version_major, gpuDriverInfo.version_minor);

    if (isDkms)
        mvwprintw2c(nwin, line++, x, "%-20s: %s", "DKMS:", modAmdgpu.version);
    else
        mvwprintw2c(nwin, line++, x, "%-20s: %s", "Build-In:", modAmdgpu.version);

    if (isDkms) {
        mvwprintw2c(nwin, line++, x, "%-20s: %s", "KCL Driver:", modAmdkcl.version);
        mvwprintw2c(nwin, line++, x, "%-20s: %s", "DKMS SRCVer:", modAmdgpu.srcversion);
        mvwprintw2c(nwin, line++, x, "%-20s: %s", "KCL  SRCVer:", modAmdkcl.srcversion);
    }

    wrefresh(nwin);

    return ret;
}

static int tabDriverInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabDriverInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo driverInfo = {
    .id = TabID_DRIVER,
    .name = "DriverInfo",
    .labelName = "Driver",
    .init = tabDriverInfoInit,
    .exit = tabDriverInfoExit,
    .update = tabDriverInfoUpdate,
};

