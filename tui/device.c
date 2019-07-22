#include <asm-generic/errno-base.h>
#include <unistd.h>
#include <string.h>
#include "device.h"
#include "utils/utils.h"

struct DeviceContext* AllocDeviceContext(void)
{
    return xAlloc(sizeof(struct DeviceContext));
}

void FreeDeviceContext(struct DeviceContext *dctx)
{
    for(int i = 0; i < dctx->deviceCount; i++) {
        if (dctx->devs[i])
            FreeDevice(dctx->devs[i]);
        dctx->devs[i] = NULL;
    }

    if (dctx->pacc) {
        xFree(dctx->pacc);
    }

    xFree(dctx);
}

int InitDevice(struct Device *device)
{
   int ret = 0;
   size_t size = 0;

   switch (device->pdev->vendor_id) {
       case 0x1002:
           device->vendorType = GPU_DEVICE_AMD;
           break;
       default:
           device->vendorType = GPU_DEVICE_UNKNOW;
           break;
   }

   device->domain = device->pdev->domain;
   device->bus= device->pdev->bus;
   device->dev = device->pdev->dev;
   device->func = device->pdev->func;

   if (device->vendorType != GPU_DEVICE_AMD)
       return ret;

   ret = DeviceGetDeviceName(device, device->deviceName, &size);
   if (ret)
       return ret;

   ret = DeviceGetDriverName(device, device->driverName, &size);
   if (ret) {
       device->driverisLoaded = false;
       MemClear(device->driverName, MAX_NAME_SIZE);
   } else {
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
   }

   return ret;
}

struct Device* AllocDevice(struct DeviceContext *dctx, struct pci_dev *pdev)
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
    if (device->pdev)
        pci_free_dev(device->pdev);
   device->ctx->devs[device->ctx->deviceCount--] = NULL;
   xFree(device);
}

int InitDeviceContext(struct DeviceContext *dctx)
{
    int ret = 0;
    struct pci_dev *pdev;
    struct pci_access *pacc;
    struct Device *dev;

    dctx->deviceCount = 0;
    pacc = pci_alloc();
    dctx->pacc = pacc;
    pci_init(pacc);
    pci_scan_bus(pacc);
    for (pdev = pacc->devices; pdev != NULL; pdev = pdev->next) {
        pci_fill_info(pdev, PCI_FILL_IDENT | PCI_FILL_CLASS | PCI_FILL_BASES);
        if ((pdev->device_class >> 8) == PCI_BASE_CLASS_DISPLAY) {
            dev = AllocDevice(dctx, pdev);
            if (!dev)
                return -1;
            ret = InitDevice(dev);
            if (ret)
                return ret;
        }
    }

    return ret;
}

struct Device *getDeviceByIndex(struct DeviceContext *dctx, uint32_t index)
{
    if (dctx->deviceCount == 0 || index >= dctx->deviceCount)
        return NULL;

    return dctx->devs[index];
}

int DeviceGetDeviceName(struct Device *device, char *deviceName, size_t *outsize)
{
    char *name = NULL;

    if (!deviceName)
        return -EINVAL;

    name = pci_lookup_name(device->pdev->access, deviceName, MAX_NAME_SIZE,
        PCI_LOOKUP_DEVICE, device->pdev->vendor_id, device->pdev->device_id);

    if (!name)
        return -EINVAL;

    if (outsize)
        *outsize = strlen(deviceName);

    return 0;
}

#define DEFAULT_SYS_PATH "/sys/bus/pci"

int DeviceGetSysPath(struct Device *device, char* path, size_t *outsize)
{
    struct pci_dev *pdev = device->pdev;
    size_t size = 0;
    char *sysfs = NULL, *defaut_sys_path = DEFAULT_SYS_PATH;

    if (!path)
        return -EINVAL;

    sysfs = pci_get_param(pdev->access, "sysfs.path");
    if (sysfs == NULL)
        sysfs = defaut_sys_path;

    size = snprintf(path, MAX_NAME_SIZE, "%s/devices/%04x:%02x:%02x.%d",
             sysfs, pdev->domain, pdev->bus, pdev->dev, pdev->func);

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

    if (access(buf,R_OK))
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
