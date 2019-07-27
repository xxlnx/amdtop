#ifndef __GPU_DRI_DEBUG_H__
#define __GPU_DRI_DEBUG_H__

#include <stdint.h>
#include "core/gpuinfo.h"

#define RING_NAME_SIZE  (50)
#define AMDGPU_FENCE_INFO_NAME   "amdgpu_fence_info"

enum RingType{
    RingType_UNKNOW = -1,
    RingType_GFX = 0,
    RingType_COMP,
    RingType_KIQ,
    RingType_SDMA,
    RingType_VCE,
    RingType_UVD,
    RingType_CONT,
};

struct AmdGpuRing {
    struct GpuDevice *device;
    enum RingType type;
    char name[RING_NAME_SIZE];
    char shortname[RING_NAME_SIZE];
    int32_t values[3];
};

struct AmdGpuFenceInfo {
    uint32_t ringid;
    uint32_t signaled;
    uint32_t emitted;
    uint32_t trailing_fence;
    uint32_t emitted_trial;
    uint32_t preempted;
    uint32_t reset;
    uint32_t both;
};

#define CLIENT_NAME_SIZE    (100)
struct AmdGpuClientInfo {
    char command[CLIENT_NAME_SIZE];
    uint32_t pid;
    uint32_t dev;
    char master;
    char a;
    uint32_t uid;
    uint32_t magic;
};


int amdGpuRingInfo(struct GpuDevice *device, struct AmdGpuRing *gpuRing, uint32_t *count);
int amdGpuQueryFenceInfo(struct AmdGpuRing *ring, struct AmdGpuFenceInfo *fenceInfo);
int amdGpuQueryClientInfo(struct GpuDevice *device, struct AmdGpuClientInfo *clientInfo, uint32_t *count);

#endif