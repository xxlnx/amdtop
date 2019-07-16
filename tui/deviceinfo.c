#include "tabinfo.h"

struct GpuDeviceInfo gpuDeviceInfo;
struct GpuMemInfo vramInfo, visibleInfo, gttInfo;
static int getAllInfo(void)
{
    static int first = true;
    struct Device * device = getAcitveDevice();
    int ret = 0;
    if (!first)
        return 0;

    ret = gpuQueryDeviceInfo(device->gpuDevice, &gpuDeviceInfo);
    if (ret)
        return 0;
    ret = gpuQueryMemInfo(device->gpuDevice, MemType_VRAM, &vramInfo);
    if (ret)
        return ret;
    ret = gpuQueryMemInfo(device->gpuDevice, MemType_GTT, &gttInfo);
    if (ret)
        return ret;
    ret = gpuQueryMemInfo(device->gpuDevice, MemType_VISIBLE, &visibleInfo);
    if (ret)
        return ret;

    first = false;
    return ret;
}

static int tabDeviceInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    struct drm_amdgpu_info_device *amdgpuinfo = &gpuDeviceInfo.amdgpu;
    struct Device * device = getAcitveDevice();
    int ret = 0;
    int x, y;
    int line = 1;

    ret = getAllInfo();
    if (ret)
        return ret;

    x = win->layout.width / 20;
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "Name:", device->deviceName);
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "Famliy", gpuGetFamilyType(amdgpuinfo->family));
    mvwprintw2c(nwin, line++, x, "%-20s: %#x", "VendorID:", device->pdev->vendor_id);
    mvwprintw2c(nwin, line++, x, "%-20s: %#x", "DeviceID:", device->pdev->device_id);
    mvwprintw2c(nwin, line++, x, "%-20s: %#x", "RevisionID", amdgpuinfo->chip_rev);
    mvwprintw2c(nwin, line++, x, "%-20s: %#x", "ExternalID", amdgpuinfo->external_rev);
    mvwprintw2c(nwin, line++, x, "%-20s: %d",  "CUs Active", amdgpuinfo->cu_active_number);
    mvwprintw2c(nwin, line++, x, "%-20s: %d",  "CUs Per Shader", amdgpuinfo->num_cu_per_sh);
    mvwprintw2c(nwin, line++, x, "%-20s: %d",  "Shader Engines", amdgpuinfo->num_shader_engines);
    mvwprintw2c(nwin, line++, x, "%-20s: %d",  "Shader Arrays", amdgpuinfo->num_shader_arrays_per_engine);
    mvwprintw2c(nwin, line++, x, "%-20s: %-4d Mhz", "GPU Counter", amdgpuinfo->gpu_counter_freq / 1000);
    mvwprintw2c(nwin, line++, x, "%-20s: %-4d Mhz", "Max EnigneClock", amdgpuinfo->max_engine_clock / 1000);
    mvwprintw2c(nwin, line++, x, "%-20s: %-4d Mhz", "Max MemClock", amdgpuinfo->max_memory_clock / 1000);
    mvwprintw2c(nwin, line++, x, "%-20s: %d", "Num Pipes",amdgpuinfo->num_rb_pipes);
    mvwprintw2c(nwin, line++, x, "%-20s: %d", "Num HWGfx Contexts",amdgpuinfo->num_hw_gfx_contexts);
    mvwprintw2c(nwin, line++, x, "%-20s: 0x%08x", "VAddress Offset",amdgpuinfo->virtual_address_offset);
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "VRam Type", gpuGetVramType(amdgpuinfo->vram_type));
    mvwprintw2c(nwin, line++, x, "%-20s: %d", "VRam Width",amdgpuinfo->vram_bit_width);
    mvwprintw2c(nwin, line++, x, "%-20s: %-6d MB", "VRam Size", vramInfo.total >> 20);
    mvwprintw2c(nwin, line++, x, "%-20s: %-6d MB", "Visible Size", visibleInfo.total >> 20);
    mvwprintw2c(nwin, line++, x, "%-20s: %-6d MB", "GTT Size", gttInfo.total >> 20);
    // mvwprintw2c(nwin, line++, x, "%-20s: 0x%08x",  "VAddress Max",amdgpuinfo->virtual_address_max);
    // mvwprintw2c(nwin, line++, x, "%-20s: 0x%08x",  "VAddress Align",amdgpuinfo->virtual_address_alignment);
    wrefresh(nwin);

    return ret;
}

static int tabDeviceInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabDeviceInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo deviceInfo = {
    .name = "deviceInfo",
    .labelName = "Device",
    .init = tabDeviceInfoInit,
    .exit = tabDeviceInfoExit,
};

