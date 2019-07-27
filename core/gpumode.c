#include <errno.h>
#include <string.h>
#include "gpumode.h"
#include "core/drmhelper.h"
#include "utils/utils.h"

#define U642VOID(x) ((void *)(unsigned long)(x))
#define VOID2U64(x) ((uint64_t)(unsigned long)(x))

int gpuGetModeResouce(struct GpuDevice *gpuDevice, struct GpuModeResource **modeResource)
{
    int ret = 0;
    struct drm_mode_card_res cardRes, tmpCardRes;
    struct GpuModeResource *res = NULL;

retry:
    MemClear(&cardRes, sizeof(cardRes));
    ret = gpuIoctl(gpuDevice->fd, DRM_IOCTL_MODE_GETRESOURCES, &cardRes);
    if (ret)
        return ret;

    tmpCardRes = cardRes;

    if (cardRes.count_connectors) {
        cardRes.connector_id_ptr = VOID2U64(xAlloc(sizeof(uint32_t) * cardRes.count_connectors));
        if (!cardRes.connector_id_ptr)
            goto error;
    }
    if (cardRes.count_encoders) {
        cardRes.encoder_id_ptr = VOID2U64(xAlloc(sizeof(uint32_t) * cardRes.count_encoders));
        if (!cardRes.encoder_id_ptr)
            goto error;
    }
    if (cardRes.count_crtcs) {
        cardRes.crtc_id_ptr = VOID2U64(xAlloc(sizeof(uint32_t) * cardRes.count_crtcs));
        if (!cardRes.crtc_id_ptr)
            goto error;
    }
    if (cardRes.count_fbs) {
        cardRes.fb_id_ptr = VOID2U64(xAlloc(sizeof(uint32_t) * cardRes.count_fbs));
        if (!cardRes.fb_id_ptr)
            goto error;
    }

    ret = gpuIoctl(gpuDevice->fd, DRM_IOCTL_MODE_GETRESOURCES, &cardRes);
    if (ret)
        return ret;

    if (tmpCardRes.count_fbs != cardRes.count_fbs ||
        tmpCardRes.count_crtcs != cardRes.count_crtcs ||
        tmpCardRes.count_encoders != cardRes.count_encoders ||
        tmpCardRes.count_connectors != cardRes.count_connectors) {
        xFree(U642VOID(cardRes.fb_id_ptr));
        xFree(U642VOID(cardRes.crtc_id_ptr));
        xFree(U642VOID(cardRes.encoder_id_ptr));
        xFree(U642VOID(cardRes.connector_id_ptr));
        goto retry;
    }

    res = xAlloc(sizeof(*res));
    if (!res)
        goto error;

    res->connector_count = cardRes.count_connectors;
    res->encoder_count = cardRes.count_encoders;
    res->crtcs_count = cardRes.count_crtcs;
    res->fb_count = cardRes.count_fbs;

    if (res->connector_count) {
        res->connector_ids= xAlloc(sizeof(uint32_t) * cardRes.count_connectors);
        if (res->connector_ids)
            memcpy(res->connector_ids, U642VOID(cardRes.connector_id_ptr), sizeof(uint32_t) * cardRes.count_connectors);
    }

    if (res->encoder_count) {
        res->encoder_ids = xAlloc(sizeof(uint32_t) * cardRes.count_encoders);
        if (res->encoder_ids)
            memcpy(res->encoder_ids, U642VOID(cardRes.encoder_id_ptr), sizeof(uint32_t) * cardRes.count_encoders);
    }

    if (res->crtcs_count) {
        res->crtc_ids= xAlloc(sizeof(uint32_t) * cardRes.count_crtcs);
        if (res->crtc_ids)
            memcpy(res->crtc_ids, U642VOID(cardRes.crtc_id_ptr), sizeof(uint32_t) * cardRes.count_crtcs);
    }

    if (res->fb_count) {
        res->fb_ids= xAlloc(sizeof(uint32_t) * cardRes.count_fbs);
        if (res->fb_ids)
            memcpy(res->fb_ids, U642VOID(cardRes.fb_id_ptr), sizeof(uint32_t) * cardRes.count_fbs);
    }

    res->device = gpuDevice;
    *modeResource = res;

    if (res->fb_count && !res->fb_ids ||
    res->connector_count && !res->connector_ids ||
    res->crtcs_count && !res->crtc_ids ||
    res->encoder_count && !res->encoder_ids){
        xFree(res->connector_ids);
        xFree(res->encoder_ids);
        xFree(res->crtc_ids);
        xFree(res->fb_ids);
        return -ENOMEM;
    }

 error:
    xFree(U642VOID(cardRes.fb_id_ptr));
    xFree(U642VOID(cardRes.crtc_id_ptr));
    xFree(U642VOID(cardRes.encoder_id_ptr));
    xFree(U642VOID(cardRes.connector_id_ptr));

    return ret;
}

int gpuFreeModeResource(struct GpuModeResource *modeResource)
{
    xFree(modeResource->connector_ids);
    xFree(modeResource->encoder_ids);
    xFree(modeResource->crtc_ids);
    xFree(modeResource->fb_ids);
    xFree(modeResource);
    modeResource = NULL;
    return 0;
}

static void copyModeInfo(struct GpuModeInfo *modeInfo, struct drm_mode_modeinfo *drm_modeinfo)
{
    if (!modeInfo || !drm_modeinfo)
        return;

    modeInfo->clock = drm_modeinfo->clock;
    modeInfo->hdisplay = drm_modeinfo->hdisplay;
    modeInfo->hsync_start = drm_modeinfo->hsync_start;
    modeInfo->hsync_end = drm_modeinfo->hsync_end;
    modeInfo->htotal = drm_modeinfo->htotal;
    modeInfo->hskew = drm_modeinfo->hskew;

    modeInfo->vdisplay = drm_modeinfo->vdisplay;
    modeInfo->vsync_start = drm_modeinfo->vsync_start;
    modeInfo->vsync_end = drm_modeinfo->vsync_end;
    modeInfo->vtotal = drm_modeinfo->vtotal;
    modeInfo->vscan = drm_modeinfo->vscan;
    modeInfo->vrefresh = drm_modeinfo->vrefresh;
    modeInfo->flags = drm_modeinfo->flags;
    modeInfo->type = drm_modeinfo->type;
    memcpy(modeInfo->name, drm_modeinfo->name, 32);
}

int gpuGetConnector(struct GpuModeResource *res, uint32_t conn_id, struct GpuConnector **connector)
{
    int ret = 0;
    struct GpuConnector *conn = NULL;
    struct drm_mode_get_connector drm_conn;
    struct GpuDevice *device = res->device;

    MemClear(&drm_conn, sizeof(drm_conn));
    drm_conn.connector_id  = conn_id;

    ret = gpuIoctl(device->fd, DRM_IOCTL_MODE_GETCONNECTOR, &drm_conn);
    if (ret)
        return ret;

    if (drm_conn.count_encoders) {
        drm_conn.encoders_ptr = VOID2U64(xAlloc(sizeof(uint32_t) * drm_conn.count_encoders));
        if (!drm_conn.encoders_ptr)
            goto error;
    }

    if (drm_conn.count_modes) {
        drm_conn.modes_ptr = VOID2U64(xAlloc(sizeof(struct drm_mode_modeinfo) * drm_conn.count_modes));
        if (!drm_conn.modes_ptr)
            goto error;
    }

    if (drm_conn.count_props) {
        drm_conn.props_ptr = VOID2U64(xAlloc(sizeof(uint32_t) * drm_conn.count_props));
        if (!drm_conn.props_ptr)
            goto error;
        drm_conn.prop_values_ptr = VOID2U64(xAlloc(sizeof(uint64_t) * drm_conn.count_props));
        if (!drm_conn.prop_values_ptr)
            goto error;
    }

    ret = gpuIoctl(device->fd, DRM_IOCTL_MODE_GETCONNECTOR, &drm_conn);
    if (ret)
        return ret;

    conn = xAlloc(sizeof(*conn));
    if (!conn) {
        goto error;
    }

    if (drm_conn.count_encoders) {
        conn->encoder_ids = xAlloc(sizeof(uint32_t) * drm_conn.count_encoders);
        if (conn->encoder_ids)
            memcpy(conn->encoder_ids, U642VOID(drm_conn.encoders_ptr), sizeof(uint32_t) * drm_conn.count_encoders);
    }
    if (drm_conn.count_props) {
        conn->props= xAlloc(sizeof(uint32_t) * drm_conn.count_props);
        if (conn->props)
            memcpy(conn->props, U642VOID(drm_conn.props_ptr), sizeof(uint32_t) * drm_conn.count_props);
        conn->prop_values = xAlloc(sizeof(uint64_t) * drm_conn.count_props);
        if (conn->prop_values)
            memcpy(conn->prop_values, U642VOID(drm_conn.prop_values_ptr), sizeof(uint64_t) * drm_conn.count_props);
    }
    if (drm_conn.count_modes) {
        conn->modeInfos = xAlloc(sizeof(struct GpuModeInfo) * drm_conn.count_modes);
        if (conn->modeInfos) {
            for (int i = 0 ; i < drm_conn.count_modes; i++) {
                struct GpuModeInfo *modeInfo = conn->modeInfos;
                struct drm_mode_modeinfo *drmmodeInfo = U642VOID(drm_conn.modes_ptr);
                copyModeInfo(&modeInfo[i], &drmmodeInfo[i]);
            }
        }
    }

    if (drm_conn.count_encoders && !conn->encoder_ids ||
    drm_conn.count_modes && !conn->modeInfos ||
    drm_conn.count_props && !conn->prop_values ||
    drm_conn.count_props && !conn->props) {
        xFree(conn->modeInfos);
        xFree(conn->encoder_ids);
        xFree(conn->props);
        xFree(conn->prop_values);
        xFree(conn);
        conn = NULL;
        return -ENOMEM;
    }

    conn->mode_count = drm_conn.count_modes;
    conn->prop_count = drm_conn.count_props;
    conn->encoder_count = drm_conn.count_encoders;
    conn->encoder_id = drm_conn.encoder_id;
    conn->connector_id = drm_conn.connector_id;
    conn->connector_type = drm_conn.connector_type;
    conn->connector_type_id = drm_conn.connector_type_id;
    conn->mm_height = drm_conn.mm_height;
    conn->mm_width = drm_conn.mm_width;
    conn->connection = drm_conn.connection;
    conn->subpixel = drm_conn.subpixel;

    *connector = conn;

error:
    xFree(U642VOID(drm_conn.encoders_ptr));
    xFree(U642VOID(drm_conn.modes_ptr));
    xFree(U642VOID(drm_conn.prop_values_ptr));
    xFree(U642VOID(drm_conn.props_ptr));

    return ret;
}

int gpuFreeConnector(struct GpuConnector *connector)
{
    xFree(connector->modeInfos);
    xFree(connector->encoder_ids);
    xFree(connector->props);
    xFree(connector->prop_values);
    xFree(connector);
    connector = NULL;
    return 0;
}

int gpuGetProperty(struct GpuModeResource *modeResources, uint32_t prop_id, struct GpuProp **gpuProp)
{
    int ret = 0;
    struct GpuDevice *device = modeResources->device;
    struct drm_mode_get_property drm_prop;
    struct GpuProp *prop = NULL;

    MemClear(&drm_prop, sizeof(drm_prop));
    drm_prop.prop_id = prop_id;
    ret = gpuIoctl(device->fd, DRM_IOCTL_MODE_GETPROPERTY, &drm_prop);
    if (ret)
        return ret;

    if (drm_prop.count_values) {
        drm_prop.values_ptr = VOID2U64(xAlloc(sizeof(uint64_t) * drm_prop.count_values));
        if (!drm_prop.values_ptr)
            goto error;
    }
    if (drm_prop.count_enum_blobs && (drm_prop.flags & (DRM_MODE_PROP_BITMASK | DRM_MODE_PROP_ENUM))) {
        drm_prop.enum_blob_ptr = VOID2U64(xAlloc(sizeof(struct drm_mode_property_enum) * drm_prop.count_enum_blobs));
        if (!drm_prop.enum_blob_ptr)
            goto error;
    }
    if (drm_prop.count_enum_blobs && (drm_prop.flags & (DRM_MODE_PROP_BLOB))) {
        drm_prop.values_ptr = VOID2U64(xAlloc(sizeof(uint32_t) * drm_prop.count_enum_blobs));
        drm_prop.enum_blob_ptr = VOID2U64(xAlloc(sizeof(uint32_t) * drm_prop.count_enum_blobs));
        if (!drm_prop.values_ptr || !drm_prop.enum_blob_ptr)
            goto error;
    }

    ret = gpuIoctl(device->fd, DRM_IOCTL_MODE_GETPROPERTY, &drm_prop);
    if (ret)
        return ret;

    prop = xAlloc(sizeof(*prop));
    if (!prop)
        goto error;

    prop->prop_id = drm_prop.prop_id;
    prop->flags = drm_prop.flags;

    if (drm_prop.count_values) {
        prop->count_values = drm_prop.count_values;
        prop->values = xAlloc(sizeof(uint64_t) * drm_prop.count_values);
        if (prop->values)
            memcpy(prop->values, U642VOID(drm_prop.values_ptr), drm_prop.count_values * sizeof(uint64_t));
    }

    if (drm_prop.flags & (DRM_MODE_PROP_ENUM | DRM_MODE_PROP_BITMASK)) {
        prop->count_enums = drm_prop.count_enum_blobs;
        prop->enums = xAlloc(sizeof(struct GpuPropEnum) * drm_prop.count_enum_blobs);
        if (prop->enums)
            memcpy(prop->enums, U642VOID(drm_prop.enum_blob_ptr), drm_prop.count_enum_blobs * sizeof(struct drm_mode_property_enum));
    } else if (drm_prop.flags & DRM_MODE_PROP_BLOB) {
        prop->count_blobs = drm_prop.count_enum_blobs;
        prop->values = xAlloc(sizeof(uint32_t) * drm_prop.count_enum_blobs);
        if (prop->values)
            memcpy(prop->values, U642VOID(drm_prop.values_ptr), drm_prop.count_enum_blobs * sizeof(uint32_t));
        prop->blob_ids= xAlloc(sizeof(uint32_t) * drm_prop.count_enum_blobs);
        if (prop->blob_ids)
            memcpy(prop->blob_ids, U642VOID(drm_prop.enum_blob_ptr), drm_prop.count_enum_blobs * sizeof(uint32_t));
    }

    if (drm_prop.count_values && !prop->values ||
    drm_prop.flags & (DRM_MODE_PROP_ENUM | DRM_MODE_PROP_BITMASK) && !prop->enums ||
    drm_prop.flags & DRM_MODE_PROP_BLOB && !prop->values ||
    drm_prop.flags & DRM_MODE_PROP_BLOB && !prop->blob_ids) {
        xFree(prop->values);
        xFree(prop->enums);
        xFree(prop->blob_ids);
        xFree(prop);
        prop = NULL;
        return -ENOMEM;
    }

    strncpy(prop->name, drm_prop.name, DRM_PROP_NAME_LEN);
    prop->name[DRM_PROP_NAME_LEN - 1] = 0;
    *gpuProp = prop;

error:
    xFree(U642VOID(drm_prop.values_ptr));
    xFree(U642VOID(drm_prop.enum_blob_ptr));

    return 0;
}

int gpuFreeProperty(struct GpuProp *prop)
{
    int ret = 0;
    xFree(prop->values);
    xFree(prop->enums);
    xFree(prop->blob_ids);
    xFree(prop);
    prop = NULL;
    return 0;
}

int gpuGetPropBlob(struct GpuModeResource *modeResource, uint32_t blob_id, struct GpuPropBlob **gpuBlob)
{
    int ret = 0;
    struct GpuDevice *device = modeResource->device;
    struct drm_mode_get_blob drm_blob;
    struct GpuPropBlob *blob =  NULL;

    MemClear(&drm_blob, sizeof(drm_blob));
    drm_blob.blob_id = blob_id;

    ret = gpuIoctl(device->fd, DRM_IOCTL_MODE_GETPROPBLOB, &drm_blob);
    if (ret)
        return ret;
    if (drm_blob.length) {
        drm_blob.data = VOID2U64(xAlloc(drm_blob.length));
        if (!drm_blob.data)
            goto error;
    }

    ret = gpuIoctl(device->fd, DRM_IOCTL_MODE_GETPROPBLOB, &drm_blob);
    if (ret)
        return ret;

    blob = xAlloc(sizeof(*blob));
    if (!blob)
        goto error;

    blob->id = drm_blob.blob_id;
    blob->length = drm_blob.length;
    if (drm_blob.length) {
        blob->data = xAlloc(drm_blob.length);
        if (blob->data)
            memcpy(blob->data, U642VOID(drm_blob.data), drm_blob.length);
    }

    if (drm_blob.length && !blob->data) {
        xFree(blob->data);
        xFree(blob);
        blob = NULL;
        return -ENOMEM;
    }

    *gpuBlob = blob;

error:
    xFree(U642VOID(drm_blob.data));
    return ret;
}

int gpuFreePropBlob(struct GpuPropBlob *blob)
{
    int ret = 0;
    xFree(blob->data);
    xFree(blob);
    blob = NULL;
    return ret;
}

int gpuGetEncoder(struct GpuModeResource *modeResource, uint32_t id, struct GpuEncoder *encoder)
{
    int ret = 0;
    struct drm_mode_get_encoder drm_encoder;
    struct GpuDevice *device = modeResource->device;

    MemClear(&drm_encoder, sizeof(drm_encoder));
    drm_encoder.encoder_id = id;

    ret = gpuIoctl(device->fd, DRM_IOCTL_MODE_GETENCODER, &drm_encoder);
    if (ret)
        return ret;

    encoder->id = drm_encoder.encoder_id;
    encoder->encoder_type = drm_encoder.encoder_type;
    encoder->crtc_id = drm_encoder.crtc_id;
    encoder->possible_crtcs = drm_encoder.possible_crtcs;
    encoder->possible_clones = drm_encoder.possible_clones;

    return ret;
}

int gpuGetCrtc(struct GpuModeResource *modeResource, uint32_t id, struct GpuCrtc *crtc)
{
    int ret = 0;
    struct drm_mode_crtc drm_crtc;
    struct GpuDevice *device = modeResource->device;
    MemClear(&drm_crtc, sizeof(drm_crtc));

    drm_crtc.crtc_id = id;

    ret = gpuIoctl(device->fd, DRM_IOCTL_MODE_GETCRTC, &drm_crtc);
    if (ret)
        return ret;

    crtc->id = drm_crtc.crtc_id;
    crtc->fb_id = drm_crtc.fb_id;
    crtc->count_connectors = drm_crtc.count_connectors;
    crtc->set_connectors_ptr = drm_crtc.set_connectors_ptr;
    crtc->x = drm_crtc.x;
    crtc->y = drm_crtc.y;
    copyModeInfo(&crtc->modeInfo, &drm_crtc.mode);
    crtc->width = drm_crtc.mode.hdisplay;
    crtc->height = drm_crtc.mode.vdisplay;
    crtc->gamma_size = crtc->gamma_size;
    crtc->mode_valid = drm_crtc.mode_valid;

    return 0;
}

#define CONNECTOR_2_NAME(type)  case DRM_MODE_CONNECTOR_##type: return #type
const char *getConnectorNameByType(uint32_t type)
{
    switch (type) {
           CONNECTOR_2_NAME(Unknown);
           CONNECTOR_2_NAME(VGA);
           CONNECTOR_2_NAME(DVII);
           CONNECTOR_2_NAME(DVID);
           CONNECTOR_2_NAME(DVIA);
           CONNECTOR_2_NAME(Composite);
           CONNECTOR_2_NAME(SVIDEO);
           CONNECTOR_2_NAME(LVDS);
           CONNECTOR_2_NAME(Component);
           CONNECTOR_2_NAME(9PinDIN);
           CONNECTOR_2_NAME(DisplayPort);
           CONNECTOR_2_NAME(HDMIA);
           CONNECTOR_2_NAME(HDMIB);
           CONNECTOR_2_NAME(TV);
           CONNECTOR_2_NAME(eDP);
           CONNECTOR_2_NAME(VIRTUAL);
           CONNECTOR_2_NAME(DSI);
           CONNECTOR_2_NAME(DPI);
        default:
            return "unknow";
    }
}

#define ENCODER_2_NAME(type)  case DRM_MODE_ENCODER_##type: return #type
const char *getEncoderNameByType(uint32_t type)
{
    switch (type) {
        ENCODER_2_NAME(NONE);
        ENCODER_2_NAME(DAC);
        ENCODER_2_NAME(TMDS);
        ENCODER_2_NAME(LVDS);
        ENCODER_2_NAME(TVDAC);
        ENCODER_2_NAME(VIRTUAL);
        ENCODER_2_NAME(DSI);
        ENCODER_2_NAME(DPMST);
        ENCODER_2_NAME(DPI);
        default:
            return "unknow";
    }
}
