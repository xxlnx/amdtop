#include "gpuinfo.h"
#include "drmhelper.h"
#include "utils/utils.h"
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <libdrm/amdgpu_drm.h>

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
int gpuQueryVbiosVersion(struct GpuDevice *device, char *vbios_version)
{
    int ret = 0, len = 0;
    FILE *fp = NULL;
    char buf[100] = {0};

    snprintf(buf, sizeof(buf), "/sys/dev/char/%d:%d/device/vbios_version", device->major, device->minor);
    fp = fopen(buf, "r");
    if (!fp)
        return -EIO;

    memset(buf, 0, sizeof(buf));
    len = fread(buf, 1, sizeof(buf) - 1, fp);
    if (len > 0) {
        buf[len - 1] = '\0';
        strncpy(vbios_version, buf, len);
    }
    else {
        ret = -EIO;
    }
    fclose(fp);

    return ret;
}

int gpuQueryVBiosInfo(struct GpuDevice *device, struct GpuVBiosInfo* vBiosInfo)
{
    struct drm_amdgpu_info request = {0};
    int ret = 0;

    request.return_pointer = (__u64)&vBiosInfo->imagelen;
    request.return_size = sizeof(vBiosInfo->imagelen);
    request.query = AMDGPU_INFO_VBIOS;
    request.vbios_info.type = AMDGPU_INFO_VBIOS_SIZE;

    ret = gpuIoctl(device->fd, DRM_IOCTL_AMDGPU_INFO, &request);
    if (ret)
        return ret;

    vBiosInfo->image = xAlloc(vBiosInfo->imagelen);
    if (!vBiosInfo->image)
        return -ENOMEM;

    request.return_pointer = (__u64)vBiosInfo->image;
    request.return_size = vBiosInfo->imagelen;
    request.query = AMDGPU_INFO_VBIOS;
    request.vbios_info.type = AMDGPU_INFO_VBIOS_IMAGE;
    request.vbios_info.offset = 0;

    ret = gpuIoctl(device->fd, DRM_IOCTL_AMDGPU_INFO, &request);
    if (ret)
        goto failed;

    vBiosInfo->device = device;

    ret = gpuQueryVbiosVersion(device, vBiosInfo->vbios_version);
    if (ret)
        return ret;

    return 0;

failed:
    if (vBiosInfo->image)
        xFree(vBiosInfo->image);
    return ret;
}

#define AMDGPU_IDS_PATH     "../data/amdgpu.ids"
int gpuQueryDeviceName(struct GpuDevice *device, char *name)
{
    char buf[100] = {0};
    int a, b, c, len = 0, ret = 0;
    uint32_t vid, rid;
    struct GpuDeviceInfo deviceInfo;

    ret = gpuQueryDeviceInfo(device, &deviceInfo);
    if (ret)
        return ret;

    FILE *fp = fopen(AMDGPU_IDS_PATH, "rb+");
    if (!fp)
	    goto fallback;

    while(fgets(buf, sizeof(buf), fp)) {
        if(buf[0] == '#') continue;
        break;
    }

    if (fscanf(fp, "%d.%d.%d\n", &a, &b, &c) != 3) {
	    goto fallback;
    }

    while (!feof(fp) && fscanf(fp, "%4x, %x, %[^,'\n']", &vid, &rid, buf) == 3) {
        if(deviceInfo.amdgpu.device_id == vid && deviceInfo.amdgpu.chip_rev == rid) {
            strcpy(name, buf);
            break;
        }
    }
fallback:
    /* failed get name by amdgpu ids file */
    if (*name == NULL) {
        memset(buf, 0, sizeof(buf));
        len = snprintf(buf, sizeof(buf), "%04x:%02x", deviceInfo.amdgpu.device_id, deviceInfo.amdgpu.chip_rev);
        buf[len] = '\0';
        strcpy(name, buf);
    }

    if (fp)
	    fclose(fp);

    return ret;
}

int gpuQueryHwIpCount(struct GpuDevice *device, enum GpuHwIpType hwIpType, uint32_t *count)
{
    int ret = 0;

    struct drm_amdgpu_info request = {0};

    request.return_pointer = (uint64_t)count;
    request.return_size = sizeof(*count);
    request.query = AMDGPU_INFO_HW_IP_COUNT;
    request.query_hw_ip.type = hwIpType;

    ret = gpuIoctl(device->fd, DRM_IOCTL_AMDGPU_INFO, &request);

    return ret;
}

int gpuQueryHwIpInfo(struct GpuDevice *device, enum GpuHwIpType hwIpType, uint32_t inst, struct GpuHwIPInfo *hwIpInfo)
{
    struct drm_amdgpu_info request = {0};
    struct drm_amdgpu_info_hw_ip info_hw_ip = {0};
    int ret = 0;

    request.return_pointer = &info_hw_ip;
    request.return_size = sizeof(info_hw_ip);
    request.query = AMDGPU_INFO_HW_IP_INFO;
    request.query_hw_ip.type = hwIpType;
    request.query_hw_ip.ip_instance = inst;

    ret = gpuIoctl(device->fd, DRM_IOCTL_AMDGPU_INFO, &request);
    if (ret)
        return ret;

    hwIpInfo->device = device;
    hwIpInfo->iptype = hwIpType;
    hwIpInfo->inst = inst;
    hwIpInfo->version_major = info_hw_ip.hw_ip_version_major;
    hwIpInfo->version_minor = info_hw_ip.hw_ip_version_minor;
    hwIpInfo->avaiable_rings = info_hw_ip.available_rings;
    hwIpInfo->capabilities = info_hw_ip.capabilities_flags;
    hwIpInfo->ib_size_alignment = info_hw_ip.ib_size_alignment;
    hwIpInfo->ib_start_aligment = info_hw_ip.ib_start_alignment;

    return ret;
}

char *gpuGetVramType(int32_t type)
{
    char *name = NULL;
    switch (type){
        case AMDGPU_VRAM_TYPE_UNKNOWN:
            name = "unknow";
            break;
        case AMDGPU_VRAM_TYPE_GDDR1:
            name = "GDDR1";
            break;
        case AMDGPU_VRAM_TYPE_DDR2:
            name = "GDDR2";
            break;
        case AMDGPU_VRAM_TYPE_GDDR3:
            name = "GDDR2";
            break;
        case AMDGPU_VRAM_TYPE_GDDR4:
            name = "GDDR4";
            break;
        case AMDGPU_VRAM_TYPE_GDDR5:
            name ="GDDR5";
            break;
        case AMDGPU_VRAM_TYPE_HBM:
            name = "HBM";
            break;
        case AMDGPU_VRAM_TYPE_DDR3:
            name = "DDR3";
            break;
        case 8:
            name = "GDDR6";
            break;
        default:
            name = "unknow";
            break;
    }
    return name;
}

char* gpuGetFamilyType(int32_t familyType)
{
    char *name = NULL;
    switch (familyType) {
        case AMDGPU_FAMILY_SI:
            name = "SI (Hainan, etc)";
            break;
        case AMDGPU_FAMILY_CI:
            name = "CI (Bonaire, Hawaii)";
            break;
        case AMDGPU_FAMILY_VI:
            name = "VI (Iceland, Tonga)";
            break;
        case AMDGPU_FAMILY_CZ:
            name = "CZ (Carrizo, Stoney)";
            break;
        case AMDGPU_FAMILY_AI:
            name = "AI (Vega10)";
            break;
        case AMDGPU_FAMILY_RV:
            name = "RV (Raven)";
            break;
        default:
            name = "unknow";
            break;
    }
    return name;
}

