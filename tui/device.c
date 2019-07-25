#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
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

    device->driverisLoaded = false;

    ret = DeviceGetDeviceName(device, device->deviceName);
    if (ret)
        return ret;
    strcpy(device->vendorName, pci_device_get_vendor_name(pdev));


    if (pci_device_has_kernel_driver(pdev)) {
        ret = DeviceGetDriverName(device, device->driverName, &size);
        if (ret) {
            device->driverisLoaded = false;
            return ret;
        }

        device->gpuDevice = gpuGetDeviceByBus(DEVICE_TYPE_RENDER,
                                              pdev->domain,
                                              pdev->bus,
                                              pdev->dev,
                                              pdev->func);
        device->gpuCardDevice = gpuGetDeviceByBus(DEVICE_TYPE_CARD,
                                                  pdev->domain,
                                                  pdev->bus,
                                                  pdev->dev,
                                                  pdev->func);

        if (!device->gpuDevice || !device->gpuCardDevice)
            return -EACCES;

        device->driverisLoaded = true;
    }

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

#define AMDGPU_IDS_PATH     "/opt/amdgpu/share/libdrm/amdgpu.ids"
int getDeviceNameFromAmdgpuIDS(struct Device* device, char *name)
{
    char buf[100] = {0};
    int a, b, c;
    int ret = 0;

    uint32_t device_id= device->device_id;
    uint32_t revision_id = device->revision_id;
    uint32_t did = 0, rid = 0;

    if (device->vendor_id != 0x1002)
        return -EINVAL;

    if (access(AMDGPU_IDS_PATH, O_RDONLY))
        return -EINVAL;

    FILE *fp = fopen(AMDGPU_IDS_PATH, "r");
    if (!fp)
        return -EINVAL;

    while(fgets(buf, sizeof(buf), fp)) {
        if(buf[0] == '#') continue;
        break;
    }

    if (fscanf(fp, "%d.%d.%d\n", &a, &b, &c) != 3) {
        ret = -EINVAL;
        goto failed;
    }

    ret = -EINVAL;
    while (!feof(fp) && fscanf(fp, "%4x, %x, %[^,'\n']", &did, &rid, buf) == 3) {
        if (did == device_id && rid == revision_id) {
            strcpy(name, buf);
            ret = 0;
            break;
        }
    }

failed:
    if (fp)
        fclose(fp);

    return ret;
}

int DeviceGetDeviceName(struct Device *device, char *deviceName)
{

    if (getDeviceNameFromAmdgpuIDS(device, deviceName)) {
        if (pci_device_get_device_name(device->pdev))
            strcpy(deviceName, pci_device_get_device_name(device->pdev));
        else
            sprintf(deviceName, "unknow device %08x:%08x.%02d",
                device->vendor_id,device->device_id, device->revision_id);
    }

    return 0;
}

bool DeviceDriverisLoaded(struct Device *device)
{
    if (!device)
        return false;

    if (device->driverisLoaded)
        return  true;

    return false;
}

