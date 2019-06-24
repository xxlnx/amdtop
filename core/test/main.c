#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include "utils/utils.h"
#include "core/gpudevice.h"
#include "core/gpuinfo.h"

int main(int argc, char *argv[])
{
    struct GpuDevice *devices;
    struct GpuPciInfo pciInfo;
    struct GpuDriverInfo driverInfo;
    struct GpuDeviceInfo deviceInfo;
    struct GpuFwInfo fwInfo;
    struct GpuMemInfo memInfo;
    struct GpuSensorInfo sensorInfo;
    struct GpuVBiosInfo vbiosInfo;
    int count = 2;
    int ret = 0;
    INFO("git rev = %s\n", BUILD_GIT_VERSION);
    ret = gpuGetDevices(&devices, &count, DEVICE_TYPE_RENDER);
    INFO("count = %d\n", count);
    for(int i = 0; i < count; i++) {
        INFO("%d, %d:%d\n", devices[i].index, devices[i].minor, devices[i].minor);
    }
    ret = gpuQueryDriverInfo(&devices[0], &driverInfo);
    INFO("ret = %d, name = %s , date %s, desc %s\n", ret , driverInfo.driver_name, driverInfo.date, driverInfo.desc);
    if (ret)
        return ret;
    ret = gpuQueryDeviceInfo(&devices[0], &deviceInfo);
    INFO("ret = %d, family = %d, deviceid = 0x%x, family_id = %d num_shader_engines %d, max eclk %ld, mclk %ld, vram_widht = %ld\n",
        ret, deviceInfo.amdgpu.family, deviceInfo.amdgpu.device_id, deviceInfo.amdgpu.family, deviceInfo.amdgpu.num_shader_engines, deviceInfo.amdgpu.max_engine_clock,
        deviceInfo.amdgpu.max_memory_clock, deviceInfo.amdgpu.vram_bit_width, deviceInfo.amdgpu);
    ret = gpuQueryFWInfo(&devices[0], FwType_SMC, 0, 0, &fwInfo);
    INFO("ret = %d, version = 0x%x (%d), feature = 0x%x \n", ret, fwInfo.version, fwInfo.version, fwInfo.feature);
    ret = gpuQueryMemInfo(&devices[0], MemType_VRAM, &memInfo);
    if (ret)
        return ret;
    INFO("ret = %d, VRAM %ld / %ld\n", ret, memInfo.used >> 20, memInfo.total >> 20);
    ret = gpuQueryMemInfo(&devices[0], MemType_VISIBLE, &memInfo);
    if (ret)
        return ret;
    INFO("ret = %d, VISIBLE %ld / %ld\n", ret, memInfo.used >> 20, memInfo.total >> 20);
    ret = gpuQueryMemInfo(&devices[0], MemType_GTT, &memInfo);
    if (ret)
        return ret;
    INFO("ret = %d, GTT %ld / %ld\n", ret, memInfo.used >> 20, memInfo.total >> 20);
    ret = gpuQuerySensorInfo(&devices[0], SensorType_GFXClock, &sensorInfo);
    if (ret)
        return ret;
    INFO("ret = %d, GFXClock = %d\n", ret, sensorInfo.value);
    ret = gpuQuerySensorInfo(&devices[0], SensorType_MemClock, &sensorInfo);
    if (ret)
        return ret;
    INFO("ret = %d, MemClock = %d\n", ret, sensorInfo.value);
    ret = gpuQuerySensorInfo(&devices[0], SensorType_GpuLoad, &sensorInfo);
    if (ret)
        return ret;
    INFO("ret = %d, Load = %d\n", ret, sensorInfo.value);
    ret = gpuQuerySensorInfo(&devices[0], SensorType_GpuTemp, &sensorInfo);
    if (ret)
        return ret;
    INFO("ret = %d, Temp = %d\n", ret, sensorInfo.value);
    ret = gpuQueryPciInfo(&devices[0], &pciInfo);
    if (ret)
        return ret;
    INFO("ret = %d, domain = %d, bus = %d, dev = %d, func = %d\n",
        ret, pciInfo.domain, pciInfo.bus, pciInfo.dev, pciInfo.func);
    INFO("ret = %d, vid = %04x, did = %04x, svid = %04x, sdid = %04x\n",
        ret, pciInfo.vid, pciInfo.did, pciInfo.sub_vid, pciInfo.sub_did);
    char *vbios_version = NULL;
    ret = gpuQueryVbiosVersion(&devices[0], &vbios_version);
    INFO("ret = %d, vbios version = %s\n", ret, vbios_version);
    if (ret)
        return ret;
    xFree(vbios_version);
    ret = gpuQueryVBiosInfo(&devices[0], &vbiosInfo);
    INFO("ret = %d, vbios version = %s, imagelen = %d, header = %02x %02x\n",
        ret, vbiosInfo.vbios_version, vbiosInfo.imagelen, vbiosInfo.image[0], vbiosInfo.image[1]);
    if (ret)
        return ret;
    xFree(vbiosInfo.image);

    char *name = NULL;
    ret = gpuQueryDeviceName(&devices[0], &name);
    INFO("ret = %d, name = %s\n", ret, name);
    if (ret)
        return ret;
    ret = gpuFreeDevices(&devices);
    if (ret)
        return ret;
    return 0;
}