#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <pciaccess.h>
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
    uint32_t vendor_id, device_id;
    uint32_t sub_vendor_id, sub_device_id;
    uint32_t revision_id;
    struct DeviceContext *ctx;
    struct pci_device *pdev;
    struct GpuDevice *gpuDevice;
    enum DeviceVendorType vendorType;
    bool driverisLoaded;
    uint32_t  irq;
    uint32_t  pci_class;
    char deviceName[MAX_NAME_SIZE];
    char vendorName[MAX_NAME_SIZE];
    char driverName[MAX_NAME_SIZE];
    uint8_t domain, bus, dev, func;
};

struct DeviceContext {
    struct Device *devs[GPU_MAX_CARD_SUPPORT];
    uint32_t  deviceCount;
};

struct PciLinkInfo {
    float current_link_speed;
    float current_link_width;
    float max_link_speed;
    float max_link_width;
};

struct IRQInfo {
    uint32_t irq;
    uint32_t hw_irq;
    char type[MAX_NAME_SIZE];
    char irq_chip[MAX_NAME_SIZE];
};


struct DeviceContext* AllocDeviceContext(void);
void FreeDeviceContext(struct DeviceContext *dctx);
int InitDeviceContext(struct DeviceContext *dctx);
struct Device* AllocDevice(struct DeviceContext *dctx, struct pci_device *pdev);
void FreeDevice(struct Device *device);
struct Device *getDeviceByIndex(struct DeviceContext *dctx, uint32_t index);
int DeviceGetSysPath(struct Device *device, char* path, size_t *outsize);
int DeviceGetDeviceName(struct Device *device, char *deviceName);
int DeviceGetDriverName(struct Device *device, char *driverName, size_t *outsize);
int getDeviceNameFromAmdgpuIDS(struct Device* device, char *name);
bool DeviceDriverisLoaded(struct Device *device);
int UpdateDeviceInfo(struct Device *device);
int DeviceGetPciInfo(struct Device *device, struct PciLinkInfo *linkInfo);
int DeviceGetIrqNumber(struct Device *device, uint32_t *irq_number);
int getIRQInfo(uint32_t irq, struct IRQInfo *irqInfo);
int getIrqCount(uint32_t irq, uint64_t *irq_count);

#endif