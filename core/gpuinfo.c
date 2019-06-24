#include "gpuinfo.h"
#include "drmhelper.h"
#include "utils/utils.h"
#include <errno.h>
#include <malloc.h>
#include <string.h>

int gpuQueryDriverInfo(struct GpuDevice *device, struct GpuDriverInfo *driverInfo)
{
    struct drm_version version = {0};
    int ret = 0;
    int fd = device->fd;
    if (!driverInfo)
        return  -EINVAL;

    ret = gpuIoctl(fd, DRM_IOCTL_VERSION, &version);
    if (ret)
        return ret;

    version.name = xAlloc(version.name_len);
    version.date = xAlloc(version.date_len);
    version.desc = xAlloc(version.desc_len);

    ret = gpuIoctl(fd, DRM_IOCTL_VERSION, &version);
    if (ret)
        return ret;

    driverInfo->version_major = version.version_major;
    driverInfo->version_minor = version.version_minor;
    driverInfo->driver_name = strdup(version.name);
    driverInfo->desc = strdup(version.desc);
    driverInfo->date = strdup(version.date);
    driverInfo->device = device;

    xFree(version.name);
    xFree(version.date);
    xFree(version.desc);

    return  ret;
}

static int queryAmdGpuInfo(struct GpuDevice *device, unsigned id, unsigned size, void *data)
{
    int ret = 0;

    struct drm_amdgpu_info request = {0};
    request.return_pointer = (__u64)data;
    request.return_size = size;
    request.query = id;

    ret = gpuIoctl(device->fd, DRM_IOCTL_AMDGPU_INFO, &request);
    if (ret)
        return ret;

    return ret;
}

int gpuQueryDeviceInfo(struct GpuDevice *device, struct GpuDeviceInfo *deviceInfo)
{
    int ret = 0;
    ret = queryAmdGpuInfo(device, AMDGPU_INFO_DEV_INFO, sizeof(deviceInfo->amdgpu), &deviceInfo->amdgpu);
    if (ret)
        return ret;
    deviceInfo->device = device;
    return ret;
}

int gpuQueryMemInfo(struct GpuDevice *device, enum GpuMemType type, struct GpuMemInfo *memInfo)
{
    int ret = 0;
    struct drm_amdgpu_info_vram_gtt vram_gtt = {0};

    ret = queryAmdGpuInfo(device, AMDGPU_INFO_VRAM_GTT, sizeof(vram_gtt), &vram_gtt);
    if (ret)
        return ret;

    switch (type) {
        case MemType_VRAM:
            memInfo->total = vram_gtt.vram_size;
            ret = queryAmdGpuInfo(device, AMDGPU_INFO_VRAM_USAGE, sizeof(memInfo->used), &memInfo->used);
            break;
        case MemType_VISIBLE:
            memInfo->total = vram_gtt.vram_cpu_accessible_size;
            ret = queryAmdGpuInfo(device, AMDGPU_INFO_VIS_VRAM_USAGE, sizeof(memInfo->used), &memInfo->used);
            break;
        case MemType_GTT:
            memInfo->total = vram_gtt.gtt_size;
            ret = queryAmdGpuInfo(device, AMDGPU_INFO_GTT_USAGE, sizeof(memInfo->used), &memInfo->used);
            break;
        default:
            return -EINVAL;
    }

    if (ret)
        return ret;

    memInfo->type = type;
    memInfo->device = device;

    return ret;
}

static int amdGpuFwTypeMap(enum GpuFwType type) {
    int fw_type = 0;
    static int fwtypeMap [] = {
        [FwType_VCE]  = AMDGPU_INFO_FW_VCE,
        [FwType_UVD]  = AMDGPU_INFO_FW_UVD,
        [FwType_MC]   = AMDGPU_INFO_FW_GFX_MEC,
        [FwType_ME]   = AMDGPU_INFO_FW_GFX_ME,
        [FwType_PFP]  = AMDGPU_INFO_FW_GFX_PFP,
        [FwType_CE]   = AMDGPU_INFO_FW_GFX_CE,
        [FwType_RLC]  = AMDGPU_INFO_FW_GFX_RLC,
        [FwType_SOS]  = AMDGPU_INFO_FW_SOS,
        [FwType_ASD]  = AMDGPU_INFO_FW_ASD,
        [FwType_SDMA] = AMDGPU_INFO_FW_ASD,
        [FwType_SMC]  = AMDGPU_INFO_FW_SMC,
    };
    if (type > FwType_COUNT)
        return -EINVAL;

    fw_type = fwtypeMap[type];
    if (fw_type == 0)
        return  -EINVAL;

    return fw_type;
}

int gpuQueryFWInfo(struct GpuDevice *device, enum GpuFwType type, uint32_t inst, uint32_t index, struct GpuFwInfo *fwInfo)
{
    struct drm_amdgpu_info_firmware firmware = {0};
    struct drm_amdgpu_info request = {0};
    uint32_t  fw_type;
    int ret = 0;

    fw_type = amdGpuFwTypeMap(type);
    if (fw_type < 0)
        return  fw_type;

    request.return_pointer = (__u64)&firmware;
    request.return_size = sizeof(firmware);
    request.query = AMDGPU_INFO_FW_VERSION;
    request.query_fw.fw_type = fw_type;
    request.query_fw.ip_instance = inst;
    request.query_fw.index = index;

    ret = gpuIoctl(device->fd, DRM_IOCTL_AMDGPU_INFO, &request);
    if (ret)
        return ret;

    fwInfo->feature = firmware.feature;
    fwInfo->version = firmware.ver;
    fwInfo->inst = inst;
    fwInfo->index = index;
    fwInfo->device = device;

    return ret;
}

static int amdGpuSensorTypeMap(enum GpuSensorType type)
{
    int sensor_type;
    static int sensorMap[] = {
        [SensorType_GFXClock] = AMDGPU_INFO_SENSOR_GFX_SCLK,
        [SensorType_MemClock] = AMDGPU_INFO_SENSOR_GFX_MCLK,
        [SensorType_GpuTemp ] = AMDGPU_INFO_SENSOR_GPU_TEMP,
        [SensorType_GpuLoad ] = AMDGPU_INFO_SENSOR_GPU_LOAD,
        [SensorType_GpuPower] = AMDGPU_INFO_SENSOR_GPU_AVG_POWER,
        [SensorType_VDDNB   ] = AMDGPU_INFO_SENSOR_VDDNB,
        [SensorType_VDDGFX  ] = AMDGPU_INFO_SENSOR_VDDGFX,
    };

    if (type > SensorType_COUNT)
        return -EINVAL;

    sensor_type = sensorMap[type];
    if (sensor_type == 0)
        return  -EINVAL;

    return sensor_type;
}

int gpuQuerySensorInfo(struct GpuDevice *device, enum GpuSensorType type, struct GpuSensorInfo* sensorInfo)
{
    struct drm_amdgpu_info request = {0};
    int ret = 0;
    int sensor_type = 0;

   sensor_type = amdGpuSensorTypeMap(type);

    if (sensor_type < 0)
        return sensor_type;

    request.return_pointer = (__u64)&sensorInfo->value;
    request.return_size = sizeof(sensorInfo->value);
    request.query = AMDGPU_INFO_SENSOR;
    request.sensor_info.type = sensor_type;

    ret = gpuIoctl(device->fd, DRM_IOCTL_AMDGPU_INFO, &request);
    if (ret)
        return ret;

    return ret;
}

static int sysfs_get_uevent(int32_t major, uint32_t minor, const char *key, char **value)
{
    char buf[100] = {0};
    char *line = NULL;
    int ret = -EINVAL, len = 0, size = 0;
    int keylen = strlen(key);
    *value = NULL;
    len = snprintf(buf, sizeof(buf), "/sys/dev/char/%d:%d/device/uevent", major, minor);
    buf[len] = '\0';

    FILE *fp = fopen(buf, "r");
    if (!fp)
        return -EIO;

    while ((len = getline(&line, &size, fp)) >= 0) {
        if (strncmp(line, key, keylen) == 0 && line[keylen] == '=') {
            /* find out */
            *value = strdup(line + keylen + 1);
            ret = 0;
            break;
        }
    }
    free(line);
    fclose(fp);
    return ret;
}

int gpuQueryPciInfo(struct GpuDevice *device,  struct GpuPciInfo *pciInfo)
{
    int ret = 0, num = 0;
    char *value = NULL;
    uint32_t domain, bus, dev, func, vid, did, sub_vid, sub_did;

    /* struct drm_irq_busid busid;
     * DRM_IOCTL_IRQ_BUSID need root permission
     * ret = gpuIoctl(fd, DRM_IOCTL_IRQ_BUSID, &busid);
     * if (ret)
     *   return ret;
     */
    value = NULL;
    ret = sysfs_get_uevent(device->major, device->minor, "PCI_SLOT_NAME", &value);
    if (ret)
        return ret;
    if (sscanf(value, "%04x:%02x:%02x.%1u", &domain, &bus, &dev, &func) == 4) {
        pciInfo->domain = domain;
        pciInfo->bus = bus;
        pciInfo->dev = dev;
        pciInfo->func = func;
    }
    free(value);

    value = NULL;
    ret = sysfs_get_uevent(device->major, device->minor, "PCI_ID", &value);
    if (ret)
        return ret;
    if (sscanf(value, "%04x:%04x", &vid, &did) == 2) {
        pciInfo->vid = vid;
        pciInfo->did = did;
    }
    free(value);


    value = NULL;
    ret = sysfs_get_uevent(device->major, device->minor, "PCI_SUBSYS_ID", &value);
    if (ret)
        return ret;

    if (sscanf(value, "%04x:%04x", &sub_vid, &sub_did) == 2) {
        pciInfo->sub_vid = sub_vid;
        pciInfo->sub_did = sub_did;
    }
    free(value);

    return ret;
}

