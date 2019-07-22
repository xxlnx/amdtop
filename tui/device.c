#include <asm-generic/errno-base.h>
#include <unistd.h>
#include <string.h>
#include "device.h"
#include "utils/utils.h"

struct DeviceContext *AllocDeviceContext(void)
{
    return xAlloc(sizeof(struct DeviceContext));
}

void FreeDeviceContext(struct DeviceContext *dctx)
{
    for (int i = 0; i < dctx->deviceCount; i++) {
        if (dctx->devs[i])
            FreeDevice(dctx->devs[i]);
        dctx->devs[i] = NULL;
    }

    pci_system_cleanup();

    xFree(dctx);
}

int InitDevice(struct Device *device)
{
    int ret = 0;
    size_t size = 0;
    struct pci_device *pdev = device->pdev;

    switch (device->pdev->vendor_id) {
        case 0x1002:
            device->vendorType = GPU_DEVICE_AMD;
            break;
        default:
            device->vendorType = GPU_DEVICE_UNKNOW;
            break;
    }

    if (device->vendorType != GPU_DEVICE_AMD)
        return ret;

    device->domain = pdev->domain;
    device->bus = pdev->bus;
    device->dev = pdev->dev;
    device->func = pdev->func;
    device->func = pdev->func;

    device->vendor_id = pdev->vendor_id;
    device->device_id = pdev->device_id;
    device->sub_vendor_id = pdev->subvendor_id;
    device->sub_device_id = pdev->subdevice_id;
    device->revision_id = pdev->revision;

    device->irq = pdev->irq;
    device->pci_class = pdev->device_class;

    strcpy(device->deviceName, pci_device_get_device_name(pdev));
    strcpy(device->vendorName, pci_device_get_vendor_name(pdev));

    ret = DeviceGetDriverName(device, device->driverName, &size);
    if (ret) {
        device->driverisLoaded = false;
        MemClear(device->driverName, MAX_NAME_SIZE);
        return ret;
    }

    device->driverisLoaded = true;
    device->gpuDevice = gpuGetDeviceByBus(DEVICE_TYPE_RENDER,
                                          device->pdev->domain,
                                          device->pdev->bus,
                                          device->pdev->dev,
                                          device->pdev->func);
    device->gpuCardDevice = gpuGetDeviceByBus(DEVICE_TYPE_CARD,
                                              device->pdev->domain,
                                              device->pdev->bus,
                                              device->pdev->dev,
                                              device->pdev->func);
    if (!device->gpuDevice || !device->gpuCardDevice)
        return -EACCES;

    return ret;
}

struct Device *AllocDevice(struct DeviceContext *dctx, struct pci_device *pdev)
{
    int ret = 0;

    struct Device *dev = xAlloc(sizeof(struct Device));
    if (!dev)
        return NULL;

    dev->ctx = dctx;
    dev->pdev = pdev;
    dev->id = dctx->deviceCount;
    dctx->devs[dctx->deviceCount++] = dev;

    return dev;
}

void FreeDevice(struct Device *device)
{
    device->ctx->devs[device->ctx->deviceCount--] = NULL;
    xFree(device);
}

int InitDeviceContext(struct DeviceContext *dctx)
{
    int ret = 0;
    struct pci_device *pdev;
    struct pci_device_iterator *iter;
    struct Device *dev;

    dctx->deviceCount = 0;

    ret = pci_system_init();
    if (ret)
        return ret;

    iter = pci_slot_match_iterator_create(NULL);
    while ((pdev = pci_device_next(iter)) != NULL) {
        if (pdev->device_class >> 16 == 0x03) {
            dev = AllocDevice(dctx, pdev);
            if (!dev)
                return -EAGAIN;
            ret = InitDevice(dev);
            if (ret)
                return ret;
        }
    }

    if (ret)
        return ret;

    return ret;
}

struct Device *getDeviceByIndex(struct DeviceContext *dctx, uint32_t index)
{
    if (dctx->deviceCount == 0 || index >= dctx->deviceCount)
        return NULL;

    return dctx->devs[index];
}

#define DEFAULT_SYS_PATH "/sys/bus/pci"
int DeviceGetSysPath(struct Device *device, char *path, size_t *outsize)
{
    struct pci_device *pdev = device->pdev;
    size_t size = 0;

    if (!path)
        return -EINVAL;

    size = snprintf(path, MAX_NAME_SIZE, "%s/devices/%04x:%02x:%02x.%d",
                    DEFAULT_SYS_PATH, pdev->domain, pdev->bus, pdev->dev, pdev->func);

    if (outsize)
        *outsize = size;

    return 0;
}

int DeviceGetDriverName(struct Device *device, char *driverName, size_t *outsize)
{
    char path[MAX_NAME_SIZE] = {0};
    char buf[MAX_NAME_SIZE] = {0};
    char driver[MAX_NAME_SIZE] = {0};
    char *drv = NULL;
    size_t size = 0;
    int ret = 0;

    ret = DeviceGetSysPath(device, path, &size);
    if (ret)
        return ret;

    size = snprintf(buf, MAX_NAME_SIZE, "%s/driver", path);
    if (size <= 0)
        return -EINVAL;

    if (access(buf, R_OK))
        return -EACCES;

    size = readlink(buf, driver, MAX_NAME_SIZE);
    if (size <= 0)
        return -EINVAL;

    driver[size] = '\0';
    drv = strrchr(driver, '/');
    strncpy(driverName, drv + 1, MAX_NAME_SIZE);
    if (outsize)
        *outsize = strlen(drv + 1);

    return 0;
}
