#ifndef __GPU_INFO_H__
#define __GPU_INFO_H__

#include <libdrm/amdgpu_drm.h>
#include "gpudevice.h"

enum GpuMemType {
    MemType_VRAM,
    MemType_VISIBLE,
    MemType_GTT,
    MemType_COUNT,
};

enum GpuFwType {
    FwType_VCE,
    FwType_UVD,
    FwType_MC,
    FwType_ME,
    FwType_PFP,
    FwType_CE,
    FwType_RLC,
    FwType_SOS,
    FwType_ASD,
    FwType_SDMA,
    FwType_SMC,
    FwType_COUNT,
};

struct GpuMemInfo {
    struct GpuDevice *device;
    enum GpuMemType type;
    uint64_t total;
    uint64_t used;
};

struct GpuHardInfo {
    struct GpuDevice *device;
    int index;
};

struct GpuFwInfo {
    struct GpuDevice *device;
    uint32_t inst;
    uint32_t index;
    uint32_t version;
    uint32_t feature;
    char * hm_version;
};

struct GpuDriverInfo {
    struct GpuDevice *device;
    int32_t version_major;
    int32_t version_minor;
    char *driver_name;
    char *date;
    char *desc;
};


struct GpuDeviceInfo {
    struct GpuDevice *device;
    union {
        struct drm_amdgpu_info_device amdgpu;
    };
};

struct GpuPciInfo {
    struct GpuDevice *dveice;
    uint32_t domain;
    uint32_t bus;
    uint32_t dev;
    uint32_t func;
    uint32_t vid;
    uint32_t did;
    uint32_t sub_vid;
    uint32_t sub_did;
};

enum  GpuSensorType {
    SensorType_GFXClock,
    SensorType_MemClock,
    SensorType_GpuLoad,
    SensorType_GpuTemp,
    SensorType_GpuPower,
    SensorType_VDDNB,
    SensorType_VDDGFX,
    SensorType_COUNT,
};
struct GpuSensorInfo {
    struct GpuDevice *device;
    enum GpuSensorType type;
    uint32_t value;
};

int gpuQueryPciInfo(struct GpuDevice *device,  struct GpuPciInfo *pciInfo);
int gpuQueryDriverInfo(struct GpuDevice *device, struct GpuDriverInfo *driverInfo);
int gpuQueryDeviceInfo(struct GpuDevice *device, struct GpuDeviceInfo *deviceInfo);
int gpuQueryFWInfo(struct GpuDevice *device, enum GpuFwType type, uint32_t inst, uint32_t index, struct GpuFwInfo *fwInfo);
int gpuQueryMemInfo(struct GpuDevice *device, enum GpuMemType type, struct GpuMemInfo *memInfo);
int gpuQuerySensorInfo(struct GpuDevice *device, enum GpuSensorType type, struct GpuSensorInfo* sensorInfo);

#endif
