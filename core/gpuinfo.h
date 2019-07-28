#ifndef __GPU_INFO_H__
#define __GPU_INFO_H__

#include <stdbool.h>
#include "gpudevice.h"
#include "import/drm.h"
#include "import/amdgpu_drm.h"

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
    FwType_VCN,
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
    enum GpuFwType type;
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

struct GpuVBiosInfo {
    struct GpuDevice *device;
    char vbios_version[100];
    uint8_t  *image;
    uint32_t imagelen;
    uint32_t  offset; /* not used */
};


enum GpuHwIpType {
    GPU_HW_IP_GFX = 0,
    GPU_HW_IP_COMPUTE,
    GPU_HW_IP_DMA,
    GPU_HW_IP_UVD,
    GPU_HW_IP_VCE,
    GPU_HW_IP_UVD_ENC,
    GPU_HW_IP_VCN_DEC,
    GPU_HW_IP_VCN_ENC,
    GPU_HW_IP_VCN_JPEG,
    GPU_HW_IP_NUM
};

struct GpuHwIPInfo {
    struct GpuDevice *device;
    enum GpuHwIpType iptype;
    uint32_t inst;
    uint32_t version_major, version_minor;
    uint64_t  capabilities;
    uint32_t ib_start_aligment;
    uint32_t ib_size_alignment;
    uint32_t avaiable_rings;
};

#define GPUCAP_STRING_SIZE  (100)
struct GpuCapInfo {
    struct GpuDevice *device;
    bool dgma_support;
    uint32_t dgma_size;
    uint32_t flags;
    char cap_str[GPUCAP_STRING_SIZE];
};

int gpuQueryPciInfo(struct GpuDevice *device,  struct GpuPciInfo *pciInfo);
int gpuQueryDriverInfo(struct GpuDevice *device, struct GpuDriverInfo *driverInfo);
int gpuQueryDeviceInfo(struct GpuDevice *device, struct GpuDeviceInfo *deviceInfo);
int gpuQueryFWInfo(struct GpuDevice *device, enum GpuFwType type, uint32_t inst, uint32_t index, struct GpuFwInfo *fwInfo);
int gpuQueryMemInfo(struct GpuDevice *device, enum GpuMemType type, struct GpuMemInfo *memInfo);
int gpuQuerySensorInfo(struct GpuDevice *device, enum GpuSensorType type, struct GpuSensorInfo* sensorInfo);
int gpuQueryVbiosVersion(struct GpuDevice *device, char *vbios_version);
int gpuQueryVBiosInfo(struct GpuDevice *device, struct GpuVBiosInfo* vBiosInfo);
int gpuQueryDeviceName(struct GpuDevice *device, char *name);
int gpuQueryHwIpCount(struct GpuDevice *device, enum GpuHwIpType hwIpType, uint32_t *count);
int gpuQueryHwIpInfo(struct GpuDevice *device, enum GpuHwIpType hwIpType, uint32_t inst, struct GpuHwIPInfo *hwIpInfo);
int gpuQueryGpuCap(struct GpuDevice *device, struct GpuCapInfo *capInfo);
char* gpuGetFamilyType(int32_t familyType);
char *gpuGetVramType(int32_t type);

#endif
