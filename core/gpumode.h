#ifndef __GPU_MODE_H__
#define __GPU_MODE_H__

#include "core/gpuinfo.h"

struct GpuModeInfo {
    uint32_t clock;
    uint16_t hdisplay;
    uint16_t hsync_start;
    uint16_t hsync_end;
    uint16_t htotal;
    uint16_t hskew;
    uint16_t vdisplay;
    uint16_t vsync_start;
    uint16_t vsync_end;
    uint16_t vtotal;
    uint16_t vscan;
    uint32_t vrefresh;
    uint32_t flags;
    uint32_t type;
    char name[32];
};

enum GpuConnection {
    GPU_MODE_CONNECTED = 1,
    GPU_MODE_DISCONNECTED = 2,
    GPU_MODE_UNKNOW = 3,
};

struct GpuConnector {
    uint32_t prop_count;
    uint32_t *props;
    uint64_t *prop_values;
    uint32_t encoder_count;
    uint32_t *encoder_ids;
    uint32_t mode_count;
    struct GpuModeInfo *modeInfos;
    uint32_t encoder_id;
    uint32_t connector_id;
    uint32_t connector_type;
    uint32_t connector_type_id;
    uint32_t mm_width;
    uint32_t mm_height;
    uint32_t connection;
    uint32_t subpixel;
};

struct GpuPropEnum {
    uint64_t value;
    char name[DRM_PROP_NAME_LEN];
};

struct GpuProp {
    uint32_t prop_id;
    uint32_t flags;
    uint32_t count_values;
    uint64_t *values; /* store the blob lengths */
    uint32_t count_enums;
    uint32_t count_blobs;
    uint32_t *blob_ids; /* store the blob IDs */
    struct GpuPropEnum *enums;
    char name[DRM_PROP_NAME_LEN];
};

struct GpuPropBlob {
    uint32_t id;
    uint32_t length;
    uint8_t *data;
};

struct GpuEncoder {
    uint32_t id;
    uint32_t encoder_type;
    uint32_t crtc_id;
    uint32_t possible_crtcs;
    uint32_t possible_clones;
};

struct GpuCrtc {
    uint32_t id;
    uint64_t set_connectors_ptr;
    uint32_t count_connectors;
    uint32_t crtc_id; /**< Id */
    uint32_t fb_id; /**< Id of framebuffer */
    uint32_t x; /**< x Position on the framebuffer */
    uint32_t y; /**< y Position on the framebuffer */
    uint32_t width;
    uint32_t height;
    uint32_t vrefresh;
    uint32_t gamma_size;
    uint32_t mode_valid;
    struct GpuModeInfo modeInfo;
};

struct GpuFrameBuffer {
    uint32_t id;
};

struct GpuModeResource {
    struct GpuDevice *device;
    uint32_t connector_count;
    uint32_t *connector_ids;
    uint32_t encoder_count;
    uint32_t *encoder_ids;
    uint32_t crtcs_count;
    uint32_t *crtc_ids;
    uint32_t fb_count;
    uint32_t *fb_ids;
};

int gpuGetModeResouce(struct GpuDevice *gpuDevice, struct GpuModeResource **modeResource);
int gpuFreeModeResource(struct GpuModeResource *modeResource);
int gpuGetConnector(struct GpuModeResource *res, uint32_t conn_id, struct GpuConnector **connector);
int gpuFreeConnector(struct GpuConnector *connector);
int gpuGetProperty(struct GpuModeResource *modeResources, uint32_t prop_id, struct GpuProp **gpuProp);
int gpuFreeProperty(struct GpuProp *prop);
int gpuGetPropBlob(struct GpuModeResource *modeResource, uint32_t blob_id, struct GpuPropBlob **gpuBlob);
int gpuFreePropBlob(struct GpuPropBlob *blob);
int gpuGetEncoder(struct GpuModeResource *modeResource, uint32_t id, struct GpuEncoder *encoder);
int gpuGetCrtc(struct GpuModeResource *modeResource, uint32_t id, struct GpuCrtc *crtc);
const char *getConnectorNameByType(uint32_t type);
const char *getEncoderNameByType(uint32_t type);

#endif
