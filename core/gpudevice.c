#include <sys/types.h>
#include <stdint.h>
#include <drm/drm.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "gpudevice.h"
#include "drmhelper.h"
#include "utils/utils.h"
#include "gpuinfo.h"

int gpuOpenDevice(struct GpuDevice *device, const char *name)
{
    int ret = 0;
    char buf[80];
    struct stat st;

    if(!device || !name)
        return  -EINVAL;

    snprintf(buf, sizeof(buf), DRM_DIR_NAME"/%s", name);
    int fd = 0;

    fd = open(buf, O_RDWR | O_CLOEXEC);
    if (fd < 0)
        return fd;

    ret = fstat(fd, &st);
    if (ret < 0)
        return ret;

    if (!S_ISCHR(st.st_mode))
        return -EIO;
    if (DRM_MAJOR != major(st.st_rdev))
        return -EIO;

    device->fd = fd;
    device->major = major(st.st_rdev);
    device->minor = minor(st.st_rdev);

    return ret;
}

int gpuCloseDevice(struct GpuDevice *device)
{
    int ret = 0;
    if (device && device->fd > 0)
        close(device->fd);
    return ret;
}

static int getDeviceTypebyName(const char *name)
{
    if (!strncmp(name, "render", 6))
        return DEVICE_TYPE_RENDER;
    if (!strncmp(name, "card", 4))
        return DEVICE_TYPE_CARD;
    return DEVICE_TYPE_UNKNOW;
}


int gpuGetDevices(struct GpuDevice **gpuDevices, int *deviceCount, enum DeviceType type)
{
    struct GpuDevice devices[GPU_MAX_CARD_SUPPORT];
    struct GpuDevice *device;
    int ret = 0;
    int index = 0;
    int fd = 0;
    DIR *dir;
    struct dirent *dent;
    dir = opendir(DRM_DIR_NAME);
    while(dent = readdir(dir)) {
        device = &devices[index];
        if (dent->d_name[0] == '.')
            continue;
        if (type != getDeviceTypebyName(dent->d_name))
            continue;
        ret = gpuOpenDevice(device, dent->d_name);
        if (ret < 0)
            continue;
        device->index = index;
        device->type = type;
        index++;
    }
    closedir(dir);

    if (deviceCount)
        *deviceCount = index;
    if (gpuDevices) {
        *gpuDevices = xAlloc(index * sizeof(struct GpuDevice));
        if (!*gpuDevices)
            return -ENOMEM;
        memcpy(*gpuDevices, devices, sizeof(struct GpuDevice) * index);
    }
    return ret;
}

int gpuFreeDevices(struct GpuDevice **devices)
{
    int ret = 0;

    if (!devices)
        return -EINVAL;
    xFree(*devices);
    *devices = NULL;

    return ret;
}

int gpuGetDeviceCount(int type)
{
    int ret = 0, count = 0;
    ret = gpuGetDevices(NULL, &count, type);
    if (ret)
        return ret;
    return count;
}

struct GpuDevice * gpuGetDeviceByBus(enum DeviceType deviceType, uint8_t domain, uint8_t bus, uint8_t dev, uint8_t func)
{
    struct GpuDevice *device= NULL, *devices = NULL, *foundDevice = NULL;
    struct GpuPciInfo pciInfo = {0};
    int deviceCount = 0;
    int ret = 0;

    ret = gpuGetDevices(&devices, &deviceCount, deviceType);
    if (ret)
        return NULL;

    for (int i = 0; i < deviceCount; i++) {
        device = &devices[i];
        ret = gpuQueryPciInfo(device, &pciInfo);
        if (ret)
            return NULL;
        if (pciInfo.domain == domain &&
            pciInfo.bus    == bus &&
            pciInfo.dev    == dev &&
            pciInfo.func   == func) {
            foundDevice = device;
        } else {
            ret = gpuCloseDevice(device);
            if (ret)
                return NULL;
        }
    }
    return foundDevice;
}
