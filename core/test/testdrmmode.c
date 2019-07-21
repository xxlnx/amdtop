#include <stdio.h>
#include "core/gpuinfo.h"
#include "core/gpumode.h"
#include "utils/utils.h"


int dump_resource(struct GpuModeResource *res)
{
    int ret = 0;
    struct GpuConnector *connector = NULL;
    struct GpuProp *prop = NULL;
    struct GpuPropBlob *propBlob = NULL;
    INFO("connector count = %d\n", res->connector_count);
    for (int i = 0; i < res->connector_count; i++) {
        ret = gpuGetConnector(res, res->connector_ids[i], &connector);
        if (ret)
            return ret;
        if (connector->connection == GPU_MODE_CONNECTED) {
            INFO("connector %d, %d x %d\n", connector->connector_id, connector->mm_width, connector->mm_height);
            INFO("connector %d, modes = %d, props = %d, encoders = %d\n",
                 connector->connector_id, connector->mode_count, connector->prop_count, connector->encoder_count);
            INFO("connector %d, type = %d type id = %d encoder id = %d\n",
                 connector->connector_id,
                 connector->connector_type,
                 connector->connector_type_id,
                 connector->encoder_id);
/*            for (int j = 0; j < connector->mode_count; j++) {
                INFO("mode %d %d x %d @ %d\n", j, connector->modeInfos[j].hdisplay, connector->modeInfos[j].vdisplay, connector->modeInfos[j].vrefresh);
            }*/
            for (int j = 0; j < connector->prop_count; j++) {
                uint32_t prop_id = 0;
                uint64_t prop_value = 0;
                prop_id = connector->props[j];
                prop_value = connector->prop_values[j];
                ret = gpuGetProperty(res, prop_id, &prop);
                if (ret)
                    return ret;
                if (prop->flags & DRM_MODE_PROP_ENUM) {
                    INFO("prop id = %d\n", prop->prop_id);
                    for (int k = 0; k < prop->count_enums; k ++) {
                        INFO("prop enum %s %d %s %s\n",prop->name, prop->enums[k].value, prop->enums[k].name,
                            prop->enums[k].value == prop_value ? "*" : "");
                    }
                }
                if (prop->flags & DRM_MODE_PROP_BLOB) {
                    ret = gpuGetPropBlob(res, prop_value, &propBlob);
                    INFO("prop id = %d, blob id = %d, length = %d\n", prop_id, prop_value, propBlob->length);
                    if (ret)
                        continue;
                    for (int k = 0; k < propBlob->length; k++) {
                        if (k  % 16 == 0)
                            printf("\n");
                        printf("%02x ", propBlob->data[k]);
                    }
                    printf("\n");
                    ret = gpuFreePropBlob(propBlob);
                    if (ret)
                        return ret;
                }
                ret = gpuFreeProperty(prop);
                if (ret)
                    return ret;
            }
        }
        ret = gpuFreeConnector(connector);
        if (ret)
            return ret;
    }
    printf("\n");
    INFO("encoder count = %d\n", res->encoder_count);
    for (int i = 0; i < res->encoder_count; i++) {
        printf("%d ", res->encoder_ids[i]);
    }
    printf("\n");
    INFO("crtc count = %d\n", res->crtcs_count);
    for (int i = 0; i < res->crtcs_count; i++) {
        printf("%d ", res->crtc_ids[i]);
    }
    printf("\n");
    INFO("fb count = %d\n", res->fb_count);
    for (int i = 0; i < res->fb_count; i++) {
        printf("%d ", res->fb_ids[i]);
    }
    printf("\n");

    return 0;
}

int main(int argc, char *argv[])
{
    struct GpuDevice *devices;
    struct GpuDevice *dev;
    struct GpuDeviceInfo deviceInfo;
    struct GpuModeResource *modeResource;
    uint32_t count = 0;
    int ret = 0;
    INFO("git rev = %s\n", BUILD_GIT_VERSION);
    ret = gpuGetDevices(&devices, &count, DEVICE_TYPE_CARD);
    INFO("count = %d\n", count);
    for (int i = 0; i < count; i++) {
        INFO("%d, %d:%d\n", devices[i].index, devices[i].major, devices[i].minor);
    }
    dev = &devices[0];
    ret = gpuGetModeResouce(dev, &modeResource);
    if (ret)
        return ret;

    ret = dump_resource(modeResource);
    if (ret)
        return ret;

    ret = gpuFreeModeResource(modeResource);
    if (ret)
        return ret;

    ret = gpuFreeDevices(&devices);
    if (ret)
        return ret;

    return ret;
}
