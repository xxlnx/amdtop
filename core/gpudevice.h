#ifndef __GPUDEVICE_H__
#define __GPUDEVICE_H__

#include <stdint.h>

#define GPU_MAX_CARD_SUPPORT (8)

#define DRM_DIR_NAME "/dev/dri"

#define DRM_MAJOR       (226)

enum DeviceType {
    DEVICE_TYPE_UNKNOW = 0,
    DEVICE_TYPE_CARD,
    DEVICE_TYPE_RENDER,
    DEVICE_TYPE_CONTROL,
};

struct GpuInfo
{
    char *deviceName;
};

struct GpuDevice
{
    int fd;
    int index;
    int major;
    int minor;
    enum DeviceType type;
    struct GpuInfo gpuInfo;
};


int gpuOpenDevice(struct GpuDevice *device, const char *name);
int gpuCloseDevice(struct GpuDevice *device);
int gpuGetDevices(struct GpuDevice **gpuDevices, int *deviceCount, enum DeviceType type);
int gpuGetDeviceCount(int type);
int gpuFreeDevices(struct GpuDevice **devices);
struct GpuDevice * gpuGetDeviceByBus(uint8_t domain, uint8_t bus, uint8_t dev, uint8_t func);

#endif