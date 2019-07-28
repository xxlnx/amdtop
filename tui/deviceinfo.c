#include "tabinfo.h"

static struct GpuDeviceInfo gpuDeviceInfo;
static struct GpuMemInfo vramInfo, visibleInfo, gttInfo;
static struct GpuCapInfo capInfo;
static struct Device *current_device = NULL;

static int getAllInfo(void)
{
    int ret = 0;
    struct Device * device = getAcitveDevice();

    if (device == current_device)
        return 0;

    if (!DeviceDriverisLoaded(device))
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
    ret = gpuQueryGpuCap(device->gpuDevice, &capInfo);
    if (ret)
        return ret;

    current_device = device;
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
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "Name", device->deviceName);
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "Famliy", gpuGetFamilyType(amdgpuinfo->family));
    mvwprintw2c(nwin, line++, x, "%-20s: %04x:%04x", "PCI", device->vendor_id, device->device_id);
    mvwprintw2c(nwin, line++, x, "%-20s: %04x:%04x", "PCI Sub", device->sub_vendor_id, device->sub_device_id);
    mvwprintw2c(nwin, line++, x, "%-20s: 0x%02x", "PCI Revsion", device->revision_id);
    mvwprintw2c(nwin, line++, x, "%-20s: 0x%02x", "Chip Rev", amdgpuinfo->chip_rev);
    mvwprintw2c(nwin, line++, x, "%-20s: 0x%02x", "Chip ExtID", amdgpuinfo->external_rev);
    mvwprintw2c(nwin, line++, x, "%-20s: %d", "IRQ No.", device->irq);
    mvwprintw2c(nwin, line++, x, "%-20s: %s", "Capability", capInfo.cap_str);
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
    .id = TabID_DEVICE,
    .name = "deviceInfo",
    .labelName = "Device",
    .init = tabDeviceInfoInit,
    .exit = tabDeviceInfoExit,
    .period = 0,
};

