#include "utils/utils.h"
#include "core/gpuinfo.h"
#include "core/gpudridebug.c"

int main(int argc, char *argv[])
{
    struct GpuDevice *devices;
    struct GpuDevice *dev;
    struct GpuDeviceInfo deviceInfo;
    struct AmdGpuRing amdGpuRing[20];
    struct AmdGpuFenceInfo fenceInfo;
    uint32_t ring_count = 0;
    uint32_t count = 0;
    int ret = 0;
    INFO("git rev = %s\n", BUILD_GIT_VERSION);
    ret = gpuGetDevices(&devices, &count, DEVICE_TYPE_CARD);
    INFO("count = %d\n", count);
    for(int i = 0; i < count; i++) {
        INFO("%d, %d:%d\n", devices[i].index, devices[i].major, devices[i].minor);
    }
    dev = &devices[0];
    ret = amdGpuRingInfo(dev, amdGpuRing, &ring_count);
    if (ret)
        return ret;
    INFO("title:\t\t\t  signaled,\temitted,\trail,\temitted_trail,\tpreempt,\trest,\tboth\n");
    for (int i = 0 ; i < ring_count; i++) {
        ret = amdGpuQueryFenceInfo(&amdGpuRing[i], &fenceInfo);
        if (ret)
            return ret;
        INFO("ring = %-20s, 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
            &amdGpuRing[i].shortname,
            fenceInfo.emitted, fenceInfo.signaled, fenceInfo.trailing_fence, fenceInfo.emitted_trial, fenceInfo.both, fenceInfo.reset, fenceInfo.preempted);
    }
    ret = gpuFreeDevices(&devices);
    if (ret)
        return ret;
    return 0;
}
