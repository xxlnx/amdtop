#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include "tabinfo.h"
#include "core/gpuinfo.h"

#define ALIGN(val, align)   (((val) + (align - 1)) & ~(align - 1))

enum BarType {
    BarType_GFXClock,
    BarType_MemClock,
    BarType_GpuLoad,
    BarType_CpuLoad,
    BarType_VRAM,
    BarType_GTT,
    BarType_VISIBLE,
    BarType_SysMem,
    BarType_COUNT,
};

static struct WindowBar sensorBar[BarType_COUNT];
static struct GpuMemInfo vramInfo, visibleInfo, gttInfo;
static struct GpuSensorInfo sensorInfo;
static struct GpuDeviceInfo deviceInfo;

static struct WindowBar *getSensorBar(enum BarType type)
{
    return &sensorBar[type];
}

struct label_position {
    uint32_t y;
    uint32_t x;
    const char *fmt;
    const char *unit;
};

static struct label_position temp_label, power_label, pmclk_label, psclk_label;

struct CpuStat {
    uint64_t user;
    uint64_t nice;
    uint64_t system;
    uint64_t idle;
    uint64_t iowait;
    uint64_t irq;
    uint64_t soft_irq;
    uint64_t steal;
    uint64_t total;
};

struct CpuLoad {
    struct CpuStat last_stat, stat;
};

#define STAT_PATH "/proc/stat"

int getCpuStat(struct CpuStat *stat)
{
    size_t  size = 0;

    FILE *fp = fopen(STAT_PATH, "r");
    if (!fp)
        return 0;

    size = fscanf(fp, "cpu %ld %ld %ld %ld %ld %ld %ld %*s %*s %*s",
       &stat->user,
       &stat->nice,
       &stat->system,
       &stat->idle,
       &stat->iowait,
       &stat->irq,
       &stat->soft_irq);

    stat->total = stat->user + stat->nice + stat->system + stat->idle + stat->iowait + stat->irq + stat->soft_irq;

    fclose(fp);

    return 0;
}

static uint32_t getCpuLoad(void)
{
    int ret = 0;
    static struct CpuLoad load;
    struct CpuStat *cur = NULL, *last = NULL;
    static uint32_t last_percent = 0;
    static bool first = true;
    struct timespec ts;
    uint64_t avg_load = 0;
    uint32_t percent = 0;

    cur = &load.stat;
    last = &load.last_stat;

    if (first) {
        first = false;
        MemClear(&load, sizeof(load));
        ret = getCpuStat(last);
        if (ret)
            return ret;
        ts.tv_sec = 0;
        ts.tv_nsec = MS_2_NS(50);
        nanosleep(&ts, NULL);
    }

    ret = getCpuStat(cur);
    if (ret)
        return ret;

    if (cur->total == last->total)
        return last_percent;

    avg_load = ((cur->idle + cur->iowait) - (last->idle + last->iowait)) * 1000 / (cur->total - last->total);
    avg_load = 1000 - avg_load;
    percent = avg_load % 1000;

    memcpy(last, cur, sizeof(*cur));

    last_percent = percent;

    return percent;
}

static int update_sensor_value(WINDOW *nwin)
{
    int ret = 0;
    struct Device *device = getAcitveDevice();

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_GFXClock, &sensorInfo);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_GFXClock), sensorInfo.value);
    if (ret)
        return ret;

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_MemClock, &sensorInfo);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_MemClock), sensorInfo.value);
    if (ret)
        return ret;

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_GpuLoad, &sensorInfo);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_GpuLoad), sensorInfo.value);
    if (ret)
        return ret;

    ret = gpuQueryMemInfo(device->gpuDevice, MemType_VRAM, &vramInfo);
    if (ret)
        return ret;
    ret = gpuQueryMemInfo(device->gpuDevice, MemType_GTT, &gttInfo);
    if (ret)
        return ret;
    ret = gpuQueryMemInfo(device->gpuDevice, MemType_VISIBLE, &visibleInfo);
    if (ret)
        return ret;

    ret = barSetValue(getSensorBar(BarType_VRAM), vramInfo.used >> 20);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_VISIBLE), visibleInfo.used >> 20);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_GTT), gttInfo.used >> 20);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_SysMem), (getTotalMem() - getFreeMem()) >> 10);
    if (ret)
        return ret;

    ret = barSetValue(getSensorBar(BarType_CpuLoad), MIN(getCpuLoad() / 10, 100));
    if (ret)
        return ret;

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_GpuTemp, &sensorInfo);
    if (ret)
        return ret;
    mvwprintwc(nwin, temp_label.y, temp_label.x, COLOR_DEAFULT, temp_label.fmt, sensorInfo.value / 1000);
    mvwprintwc(nwin, temp_label.y, getcurx(nwin), COLOR_DEAFULT, "%s", temp_label.unit);

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_GpuPower, &sensorInfo);
    if (ret)
        return ret;
    mvwprintwc(nwin, power_label.y, power_label.x, COLOR_DEAFULT, power_label.fmt, sensorInfo.value);
    mvwprintwc(nwin, power_label.y, getcurx(nwin), COLOR_DEAFULT, "%s", power_label.unit);

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_PSTATE_GFXClock, &sensorInfo);
    if (ret)
        return ret;
    mvwprintwc(nwin, psclk_label.y, psclk_label.x, COLOR_DEAFULT, psclk_label.fmt, sensorInfo.value);
    mvwprintwc(nwin, psclk_label.y, getcurx(nwin), COLOR_DEAFULT, "%s", psclk_label.unit);

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_PSTATE_GFXClock, &sensorInfo);
    if (ret)
        return ret;
    mvwprintwc(nwin, pmclk_label.y, pmclk_label.x, COLOR_DEAFULT, pmclk_label.fmt, sensorInfo.value);
    mvwprintwc(nwin, pmclk_label.y, getcurx(nwin), COLOR_DEAFULT, "%s", pmclk_label.unit);


    return ret;
}


static int tabStateInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    struct Device *device = getAcitveDevice();
    int ret = 0;
    int width, bar_width, startx, start2x, line;

    line = 3;
    startx = win->layout.width / 20;
    bar_width = (win->layout.width - 3 * startx - 2) / 2;
    width = bar_width - 28;

    start2x = startx + bar_width + startx;

    winframe(nwin, 1, 1, 10, win->layout.width - 2, "HW Monitor");
    ret = barCreate(nwin, getSensorBar(BarType_GFXClock), "GFX",  "MHz", line++, startx, width);
    ret = barCreate(nwin, getSensorBar(BarType_MemClock), "MEM",  "MHz", line++, startx, width);
    ret = barCreate(nwin, getSensorBar(BarType_GpuLoad),  "GPU",  "%",   line++, startx, width);
    ret = barCreate(nwin, getSensorBar(BarType_CpuLoad),  "CPU",  "%",   line++, startx, width);

    line = 3;
    ret = barCreate(nwin, getSensorBar(BarType_VRAM),     "VRAM", "MB",   line++, start2x, width);
    ret = barCreate(nwin, getSensorBar(BarType_VISIBLE),  "VIS", "MB",   line++, start2x, width);
    ret = barCreate(nwin, getSensorBar(BarType_GTT),      "GTT", "MB",   line++, start2x, width);
    ret = barCreate(nwin, getSensorBar(BarType_SysMem),   "RAM", "MB",   line++, start2x, width);


    int label_starty = line;
    mvwprintwc(nwin, line++, startx, COLOR_DEAFULT, "%-5s :", "PGFX");
    psclk_label.y = getcury(nwin);
    psclk_label.x = getcurx(nwin);
    psclk_label.fmt = "% -3d";
    psclk_label.unit= " Mhz";

    mvwprintwc(nwin, line++, startx, COLOR_DEAFULT, "%-5s :", "PMEM");
    pmclk_label.y = getcury(nwin);
    pmclk_label.x = getcurx(nwin);
    pmclk_label.fmt = "% -3d";
    pmclk_label.unit= " Mhz";

    line = label_starty;
    mvwprintwc(nwin, line++, start2x, COLOR_DEAFULT, "%-5s :", "Temp");
    temp_label.y = getcury(nwin);
    temp_label.x = getcurx(nwin);
    temp_label.fmt = "% -3d";
    temp_label.unit = " C";

    mvwprintwc(nwin, line++, start2x, COLOR_DEAFULT, "%-5s :", "Power");
    power_label.y = getcury(nwin);
    power_label.x = getcurx(nwin);
    power_label.fmt = "% -3d";
    power_label.unit = " Watt";

    ret = gpuQueryDeviceInfo(device->gpuDevice, &deviceInfo);
    if (ret)
        return  ret;
    ret = gpuQueryMemInfo(device->gpuDevice, MemType_VRAM, &vramInfo);
    if (ret)
        return ret;
    ret = gpuQueryMemInfo(device->gpuDevice, MemType_GTT, &gttInfo);
    if (ret)
        return ret;
    ret = gpuQueryMemInfo(device->gpuDevice, MemType_VISIBLE, &visibleInfo);
    if (ret)
        return ret;
    ret = barSetMaxValue(getSensorBar(BarType_GFXClock), deviceInfo.amdgpu.max_engine_clock / 1000);
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_MemClock), deviceInfo.amdgpu.max_memory_clock / 1000);
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_GpuLoad), 100);
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_CpuLoad), 100);
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_VRAM), ALIGN(vramInfo.total >> 20, 1024));
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_VISIBLE), visibleInfo.total >> 20);
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_GTT), ALIGN(gttInfo.total >> 20, 1024));
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_SysMem), ALIGN(getTotalMem() >> 10, 1024));
    if (ret)
        return  ret;

    ret = update_sensor_value(nwin);

    wrefresh(nwin);

    return ret;
}

static int tabStateInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabStateInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    WINDOW *nwin = win->nwin;

    ret = update_sensor_value(nwin);
    if (ret)
        return ret;

    wrefresh(nwin);
    return ret;
}

struct TabInfo stateInfo = {
    .id = TabID_STAT,
    .name = "StateInfo",
    .labelName = "Stat",
    .init = tabStateInfoInit,
    .exit = tabStateInfoExit,
    .update = tabStateInfoUpdate,
    .period = MS_2_NS(500),
};

