#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tabinfo.h"
#include "core/gpuinfo.h"

#define ALIGN(val, align)   (((val) + (align - 1)) & ~(align - 1))

enum BarType {
    BarType_GFXClock,
    BarType_MemClock,
    BarType_GpuLoad,
    BarType_GpuTemp,
    BarType_GpuPower,
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
static bool needRefresh = true;
static struct WindowBar *getSensorBar(enum BarType type)
{
    return &sensorBar[type];
}

static int update_sensor_value(void)
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

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_GpuTemp, &sensorInfo);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_GpuTemp), sensorInfo.value / 1000);
    if (ret)
        return ret;

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_GpuLoad, &sensorInfo);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_GpuLoad), sensorInfo.value);
    if (ret)
        return ret;

    ret = gpuQuerySensorInfo(device->gpuDevice, SensorType_GpuPower, &sensorInfo);
    if (ret)
        return ret;
    ret = barSetValue(getSensorBar(BarType_GpuPower), sensorInfo.value);
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

    return ret;
}


static int tabStateInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    struct Device *device = getAcitveDevice();
    int ret = 0;
    int width, bar_width, x, line;

    line = 1;
    if (WindowCheckSize(win, BarType_COUNT, 28 + 10) == false) {
        needRefresh = false;
        mvwprintwc(nwin, 1, 1, COLOR_RED_COLOR, "Window Too Small!", win->layout.width, win->layout.height);
        wrefresh(nwin);
        return 0;
    }

    x = 10;
    bar_width = win->layout.width - 2 * x - 2;
    width = bar_width - 28;
    if (width < 5) {
        x = 2;
        bar_width = win->layout.width - 4;
        width = bar_width - 28;
        width = MAX(0, width);
    }

    ret = barCreate(nwin, getSensorBar(BarType_GFXClock), "SCLK",  "MHz", line++, x, width);
    ret = barCreate(nwin, getSensorBar(BarType_MemClock), "MCLK",  "MHz", line++, x, width);
    ret = barCreate(nwin, getSensorBar(BarType_GpuTemp),  "Temp",  "C",   line++, x, width);
    ret = barCreate(nwin, getSensorBar(BarType_GpuLoad),  "Load",  "%",   line++, x, width);
    ret = barCreate(nwin, getSensorBar(BarType_GpuPower), "Power", "Watt",   line++, x, width);
    ret = barCreate(nwin, getSensorBar(BarType_VRAM),     "VRAM", "MB",   line++, x, width);
    ret = barCreate(nwin, getSensorBar(BarType_VISIBLE),  "VIS", "MB",   line++, x, width);
    ret = barCreate(nwin, getSensorBar(BarType_GTT),      "GTT", "MB",   line++, x, width);
    ret = barCreate(nwin, getSensorBar(BarType_SysMem),   "SMem", "MB",   line++, x, width);

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
    ret = barSetMaxValue(getSensorBar(BarType_GpuTemp), 100);
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_GpuLoad), 100);
    if (ret)
        return  ret;
    ret = barSetMaxValue(getSensorBar(BarType_GpuPower), 150);
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

    ret = update_sensor_value();
    if (ret)
        return ret;

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

    if (needRefresh)
        ret = update_sensor_value();
    if (ret)
        return ret;

    wrefresh(win->nwin);
    return ret;
}

struct TabInfo stateInfo = {
    .id = TabID_STAT,
    .name = "StateInfo",
    .labelName = "Stat",
    .init = tabStateInfoInit,
    .exit = tabStateInfoExit,
    .update = tabStateInfoUpdate,
};

