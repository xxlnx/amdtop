#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <pci/pci.h>
#include "core/gpudevice.h"
#include "core/gpuinfo.h"
#include <stdbool.h>

#define MAX_NAME_SIZE  (100)

enum DeviceVendorType {
    GPU_DEVICE_AMD,
    GPU_DEVICE_UNKNOW,
};

struct DeviceContext;
struct Device {
    uint32_t id;
    struct DeviceContext *ctx;
    struct pci_dev *pdev;
    struct GpuDevice *gpuDevice;
    enum DeviceVendorType vendorType;
    bool driverisLoaded;
    char deviceName[MAX_NAME_SIZE];
    char driverName[MAX_NAME_SIZE];
    uint8_t domain, bus, dev, func;
};

struct DeviceContext {
    struct pci_access *pacc;
    struct Device *devs[GPU_MAX_CARD_SUPPORT];
    uint32_t  deviceCount;
};

struct DeviceContext* AllocDeviceContext(void);
void FreeDeviceContext(struct DeviceContext *dctx);
int InitDeviceContext(struct DeviceContext *dctx);
struct Device* AllocDevice(struct DeviceContext *dctx, struct pci_dev *pdev);
void FreeDevice(struct Device *device);
struct Device *getDeviceByIndex(struct DeviceContext *dctx, uint32_t index);
int DeviceGetSysPath(struct Device *device, char* path, size_t *outsize);
int DeviceGetDeviceName(struct Device *device, char *deviceName, size_t *outsize);
int DeviceGetDriverName(struct Device *device, char *driverName, size_t *outsize);
#endif