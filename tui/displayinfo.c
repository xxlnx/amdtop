#include <string.h>
#include <errno.h>
#include <math.h>
#include "tabinfo.h"
#include "core/gpumode.h"
#include "import/edid.h"

#define IN_UNIT (0.0393701f)
#define MAX_DISPLAY_COUNT    (8)

struct DisplayInfo {
    bool present;
    uint32_t connector_id;
    uint32_t connectorType;
    uint32_t connectorType_id;
    char connector_name[MAX_NAME_SIZE];
    uint32_t mm_width;
    uint32_t mm_height;
    double in_size;
    uint32_t mode_count;

    uint8_t edid[512];  /* edid 256 bytes */
    uint32_t edid_len;

    char dpms_state[MAX_NAME_SIZE];
    uint32_t dpms_value;

    uint32_t crtc_id;
    uint32_t encoder_type ;
    char encoder_name[MAX_NAME_SIZE];

    uint32_t startx, starty, width, height;
    uint32_t vrefresh;

    /* edid information */
    char monitor_name[MAX_NAME_SIZE];
    char serial_numer[MAX_NAME_SIZE];
    char manufacturer[4];
    uint32_t year, week;
};

struct DisplayState {
    bool valid;
    uint32_t alive_connector_count;
    struct DisplayInfo displayinfo[MAX_DISPLAY_COUNT];
    uint32_t connector_count;
    uint32_t encoder_count;
    uint32_t crtc_count;
    uint32_t fb_count;
};

static struct DisplayState display_state, current_display_state;
static struct Device *current_device = NULL;

static int edidGetSerialNumber(void *raw_edid, char name[MAX_NAME_SIZE])
{
    const struct edid const * edid = raw_edid;
    int ret = -EINVAL;

    for (int i = 0; i < ARRAY_SIZE(edid->detailed_timings); i++) {
        const struct edid_monitor_descriptor * mon = &edid->detailed_timings[i].monitor;
        if (!edid_detailed_timing_is_monitor_descriptor(edid, i))
            continue;
        if (mon->tag == EDID_MONITOR_DESCRIPTOR_MONITOR_SERIAL_NUMBER) {
                strncpy(name, (char *)mon->data, sizeof(edid_monitor_descriptor_string));
                char *p = strchr(name, '\n');
                if (p)
                    *p = '\0';
                else
                    name[sizeof(edid_monitor_descriptor_string) - 1] = '\0';
            return 0;
        }
    }

    return ret;
}

static int edidGetMonitorName(void *raw_edid, char name[MAX_NAME_SIZE])
{
    const struct edid const * edid = raw_edid;
    int ret = -EINVAL;

    for (int i = 0; i < ARRAY_SIZE(edid->detailed_timings); i++) {
        const struct edid_monitor_descriptor * mon = &edid->detailed_timings[i].monitor;
        if (!edid_detailed_timing_is_monitor_descriptor(edid, i))
            continue;
        if (mon->tag == EDID_MONITOR_DESCRIPTOR_MONITOR_NAME) {
            strncpy(name, (char *)mon->data, sizeof(edid_monitor_descriptor_string));
            char *p = strchr(name, '\n');
            if (p)
                *p = '\0';
            else
                name[sizeof(edid_monitor_descriptor_string) - 1] = '\0';
            return 0;
        }
    }

    return ret;
}

static int edidGetDate(void *raw_edid, uint32_t *year ,uint32_t *week)
{
    int ret = 0;

    const struct edid const * edid = raw_edid;

    if (year)
        *year = edid->manufacture_year + 1990;
    if (week)
        *week = edid->manufacture_week;

    return ret;
}

static int edidGetManufacturer(void *raw_edid, char manufacturer[4])
{
    const struct edid const * edid = raw_edid;
    edid_manufacturer(edid, manufacturer);
    return 0;
}

static double calc_display_size(uint32_t width, uint32_t height)
{
    double diagonal = width * width + height * height;

    diagonal = sqrt(diagonal);
    diagonal *= IN_UNIT;

    return diagonal;
}

static int getConnectorPropBlob(struct GpuModeResource *res, struct GpuConnector *connector,
    const char *name, uint8_t *data, uint32_t *length)
{
    int ret = 0;
    struct GpuProp *prop = NULL;
    struct GpuPropBlob *propBlob = NULL;;
    uint64_t prop_value = 0;
    uint32_t prop_id = 0;

    for (int i = 0; i < connector->prop_count; i++) {
        prop_id = connector->props[i];
        prop_value =  connector->prop_values[i];
        ret = gpuGetProperty(res, prop_id, &prop);
        if (ret)
            return ret;

        if (prop->flags & DRM_MODE_PROP_BLOB && !strncmp(prop->name, name, strlen(name))) {
            ret = gpuGetPropBlob(res, prop_value, &propBlob);
            if (ret)
                goto blobfailed;
            memcpy(data, propBlob->data, propBlob->length);
            if (length)
                *length = propBlob->length;
            gpuFreePropBlob(propBlob);
        }
blobfailed:
        gpuFreeProperty(prop);
    }

    return ret;
}

static int getConnectorPropEnum(struct GpuModeResource *res, struct GpuConnector *connector,
    char *name, char *enum_name, uint32_t *enum_value)
{
    int ret = 0;
    struct GpuProp *prop = NULL;
    struct GpuPropEnum *propEnum = NULL;
    uint64_t prop_value = 0;
    uint32_t prop_id = 0;
    for (int i = 0; i < connector->prop_count; i++) {
        prop_id = connector->props[i];
        prop_value =  connector->prop_values[i];
        ret = gpuGetProperty(res, prop_id, &prop);
        if (ret)
            return ret;

        if (prop->flags & DRM_MODE_PROP_ENUM && !strncmp(prop->name, name, strlen(name))) {
            for (int j = 0; j < prop->count_enums; j++) {
                propEnum = &prop->enums[prop_value];
                strcpy(enum_name, propEnum->name);
                if (enum_value)
                    *enum_value = prop_value;
            }
        }
        gpuFreeProperty(prop);
    }
    return ret;

}

static int parseEDIDInfo(struct DisplayInfo *info)
{
    int ret = 0;
    void *edid = info->edid;

    if (info->edid_len == 0)
        return 0;

    ret = edidGetDate(edid, &info->year, &info->week);
    if (ret) {
        info->year = 9999;
        info->year = 9999;
    }

    ret = edidGetManufacturer(edid, info->manufacturer);
    if (ret)
        strncpy(info->manufacturer, "xxx", 3);

    ret = edidGetMonitorName(edid, info->monitor_name);
    if (ret)
        strcpy(info->monitor_name, "unknow");

    ret = edidGetSerialNumber(edid, info->serial_numer);
    if (ret)
        strcpy(info->serial_numer, "unknow");

    return ret;
}

static int query_display_info(void)
{
    int ret = 0;
    static struct GpuModeResource *resource = NULL;
    struct GpuConnector *connector = NULL;
    struct GpuProp *prop = NULL;
    struct DisplayInfo *dispInfo = NULL;
    struct GpuEncoder encoder;
    struct GpuCrtc crtc;

    struct Device *device = getAcitveDevice();

    MemClear(&display_state, sizeof(display_state));
    display_state.alive_connector_count = 0;
    display_state.valid = true;

    ret = gpuGetModeResouce(device->gpuCardDevice, &resource);
    if (ret)
        return ret;

    for (int i = 0; i < resource->connector_count; i ++) {

        ret = gpuGetConnector(resource, resource->connector_ids[i], &connector);
        if (ret)
            return ret;

        if (connector->connection != GPU_MODE_CONNECTED) {
            ret = gpuFreeConnector(connector);
            if (ret)
                return ret;
            continue;
        }

        display_state.connector_count = resource->connector_count;
        display_state.crtc_count = resource->crtcs_count;
        display_state.encoder_count = resource->encoder_count;
        display_state.fb_count = resource->fb_count;

        dispInfo  = &display_state.displayinfo[display_state.alive_connector_count];
        MemClear(dispInfo, sizeof(*dispInfo));

        dispInfo->mode_count = connector->mode_count;
        dispInfo->mm_height = connector->mm_height;
        dispInfo->mm_width = connector->mm_width;
        dispInfo->connector_id = connector->connector_id;
        dispInfo->in_size = calc_display_size(dispInfo->mm_width, dispInfo->mm_height);

        dispInfo->connectorType = connector->connector_type;
        dispInfo->connectorType_id = connector->connector_type_id;

        ret = getConnectorPropBlob(resource, connector, "EDID", dispInfo->edid, &dispInfo->edid_len);
        if (ret)
            return ret;

        ret = getConnectorPropEnum(resource, connector, "DPMS", dispInfo->dpms_state, &dispInfo->dpms_value);
        if (ret)
            return ret;

        dispInfo->present = connector->encoder_id != 0 ? true : false;
        if (dispInfo->present) {
            ret = gpuGetEncoder(resource, connector->encoder_id, &encoder);
            if (ret)
                return ret;

            dispInfo->encoder_type = encoder.encoder_type;
            dispInfo->crtc_id = encoder.crtc_id;

            ret = gpuGetCrtc(resource, encoder.crtc_id, &crtc);
            if (ret)
                return ret;

            dispInfo->starty = crtc.y;
            dispInfo->startx = crtc.x;
            dispInfo->width = crtc.width;
            dispInfo->height = crtc.height;
            dispInfo->present = true;
        }

        display_state.alive_connector_count++;

        ret = gpuFreeConnector(connector);
        if (ret)
            return ret;
    }

    return ret;
}

static int update_display_info(void)
{
    int ret = 0;
    size_t size = 0;

    struct DisplayInfo *dispInfo = NULL;
    memcpy(&current_display_state, &display_state, sizeof(display_state));

    for (int i = 0; i < current_display_state.alive_connector_count; i++) {
        dispInfo = &current_display_state.displayinfo[i];
        ret = parseEDIDInfo(&current_display_state.displayinfo[i]);
        if (ret)
            return ret;

        size = snprintf(dispInfo->connector_name, MAX_NAME_SIZE, "%s-%d",
                        getConnectorNameByType(dispInfo->connectorType), dispInfo->connectorType_id);
        dispInfo->connector_name[size] = '\0';

        if (dispInfo->present) {

            strncpy(dispInfo->encoder_name, getEncoderNameByType(dispInfo->encoder_type), MAX_NAME_SIZE);
        }
    }


    return ret;
}

static int refresh_display_ui(struct Window *win)
{
    int ret = 0;
    int line = 1;
    int x = win->layout.width / 20;
    int starty = 0;
    struct DisplayInfo *dispinfo = NULL;
    WINDOW *nwin = win->nwin;

    starty = line++;
    mvwprintw2c(nwin, line++, x, "%-10s: %2d", "connector", current_display_state.connector_count);
    mvwprintw2c(nwin, line++, x, "%-10s: %2d", "encoder", current_display_state.encoder_count);
    mvwprintw2c(nwin, line++, x, "%-10s: %2d", "crtc", current_display_state.crtc_count);
    mvwprintw2c(nwin, line++, x, "%-10s: %2d", "Display", current_display_state.alive_connector_count);
    winframe(nwin, starty, 2, line, win->layout.width - 2, "Display Info");

    line += 2;

    for (int i = 0; i < current_display_state.alive_connector_count; i++) {
        dispinfo = &current_display_state.displayinfo[i];
        starty = line++;
        mvwprintw2c(nwin, line++, x, "%-10s: %s", "Name", dispinfo->monitor_name);
        mvwprintw2c(nwin, line++, x, "%-10s: %s", "Serial", dispinfo->serial_numer);
        mvwprintw2c(nwin, line++, x, "%-10s: %s", "Vendor", dispinfo->manufacturer);
        mvwprintw2c(nwin, line++, x, "%-10s: %d year %d weeks ", "Date", dispinfo->year, dispinfo->week);
        mvwprintw2c(nwin, line++, x, "%-10s: %.2f' %3d x %3d (mm)", "Size",
                    dispinfo->in_size, dispinfo->mm_width, dispinfo->mm_height);
        mvwprintw2c(nwin, line++, x, "%-10s: %s", "DPMS", dispinfo->dpms_state);
        mvwprintw2c(nwin, line++, x, "%-10s: %s", "Present", dispinfo->present ? "Alive" : "Sleep");

        if (dispinfo->present) {
            mvwprintw2c(nwin, line++, x, "%-10s: %s", "Encoder", dispinfo->encoder_name);
            mvwprintw2c(nwin, line++, x, "%-10s: %d x %d [%d x %d]", "Resolution",
                        dispinfo->width, dispinfo->height,
                        dispinfo->startx, dispinfo->starty);
        }

        winframe(nwin, starty, 2, line , win->layout.width - 2, dispinfo->connector_name);

        line += 2;
    }

    return ret;
}


static bool needUpdateDisplayInfo(void)
{

    if (current_display_state.valid == false)
        return true;

    if (current_display_state.connector_count != display_state.connector_count)
        return true;

    if (current_display_state.alive_connector_count != display_state.alive_connector_count)
        return true;

    for (int i = 0; i < current_display_state.alive_connector_count; i++) {
        struct DisplayInfo * curr_dispinfo = &current_display_state.displayinfo[i];
        struct DisplayInfo * dispinfo = &display_state.displayinfo[i];

        if (curr_dispinfo->dpms_value != dispinfo->dpms_value)
            return true;

        if (curr_dispinfo->present != dispinfo->present)
            return true;

        if (curr_dispinfo->starty != dispinfo->starty
            || curr_dispinfo->startx != dispinfo->startx
            || curr_dispinfo->width != dispinfo->width
            || curr_dispinfo->height != dispinfo->height)
            return true;
    }

    return false;
}


static int getAllInfo()
{
    int ret = 0;
    struct Device *device = getAcitveDevice();

    if (device == current_device)
        return 0;

    current_device = device;

    ret = query_display_info();
    if (ret)
        return ret;

    if (needUpdateDisplayInfo()) {
        ret = update_display_info();
        if (ret)
            return ret;
    }

    return 0;

}


static int tabDisplayInfoInit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    WINDOW *nwin = win->nwin;

    ret = getAllInfo();
    if (ret)
        return ret;

    ret = refresh_display_ui(win);
    if (ret)
        return ret;

    wrefresh(nwin);
    return ret;
}

static int tabDisplayInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabDisplayInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;

    ret = query_display_info();
    if (ret)
        return ret;

    if (needUpdateDisplayInfo()) {
        ret = update_display_info();
        if (ret)
            return ret;

        WindowClear(win);

        ret = refresh_display_ui(win);
        if (ret)
            return ret;

        wrefresh(win->nwin);
    }

    return ret;
}

struct TabInfo displayInfo = {
    .id = TabID_Display,
    .name = "displayInfo",
    .labelName = "Display",
    .init = tabDisplayInfoInit,
    .exit = tabDisplayInfoExit,
    .update = tabDisplayInfoUpdate,
    .period = MS_2_NS(500),
};

