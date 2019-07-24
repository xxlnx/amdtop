#include <sys/utsname.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "tabinfo.h"

#define DMI_ID_INFO_COUNT   ARRAY_SIZE(id_infos)
#define DMI_ID_SYS_PATH "/sys/class/dmi/id"
#define OS_RELEASE_PATH "/etc/os-release"

static struct utsname utsname;
static char pretty_name[MAX_NAME_SIZE];

struct dmi_id_info {
    char *label;
    char *fname;
    char value[MAX_NAME_SIZE];
};

static struct dmi_id_info id_infos[] = {
    {"Board Name",      "board_name"   , "unknow" },
    {"Board Vendor",    "board_vendor" , "unknow" },
    {"BIOS Version",    "bios_version" , "unknow" },
    {"BIOS Date",       "bios_date"    , "unknow" },
/*    {"BIOS Vendor",     "bios_vendor"  , "unknow" },*/
};

static int get_dmi_info(struct dmi_id_info *idInfo)
{
    int ret = 0;
    size_t size = 0;
    char fname[MAX_NAME_SIZE] = {0};
    FILE *fp = NULL;
    char *p = NULL;

    snprintf(fname, MAX_NAME_SIZE, "%s/%s", DMI_ID_SYS_PATH, idInfo->fname);
    fp = fopen(fname, "r");
    if (!fp)
        return -EIO;
    size = fread(idInfo->value, 1, MAX_NAME_SIZE, fp);
    idInfo->value[size - 1] = '\0';
    fclose(fp);

    return ret;
}

static void getOSReleaseInfo(const char *type, char *name)
{
    char buf[MAX_NAME_SIZE] = {0};
    char type_name[MAX_NAME_SIZE] = {0};
    int ret = 0;
    size_t size = 0;
    int type_len = strlen(type);
    char *p = NULL, *q = NULL, *m = NULL;
    FILE *fp = fopen(OS_RELEASE_PATH, "r");
    if (!fp)
        return;

    while(!feof(fp) && fgets(buf, MAX_NAME_SIZE, fp)) {
        if(!strncmp(buf, type, type_len)) {
            p = strchr(buf, '=');
            p = p + 1;
            q = p;
            m = strchr(q, '\"');
            if (m) {
                m = m + 1;
                p = strchr(m, '\"');
                if (p)
                    *p = '\0';
                strcpy(name, m);
                return;
            } else {
                while(*p != 0x0a) p++;
                *p = '\0';
                strcpy(name, q);
                return;
            }
        }
    }

    strcpy(name, "unknow");
    fclose(fp);
}

static int getInfo()
{
    int ret = 0;
    static bool is_first = true;
    if (!is_first)
        return 0;
    is_first = false;

    ret = uname(&utsname);
    if (ret)
        return ret;

    getOSReleaseInfo("PRETTY_NAME", pretty_name);

    for (int i = 0; i < DMI_ID_INFO_COUNT; i++) {
        ret = get_dmi_info(&id_infos[i]);
        if (ret)
            return ret;
    }

    return ret;
}

static int tabSystemInfoInit(struct TabInfo *info, struct Window *win)
{
    WINDOW *nwin = win->nwin;
    int ret = 0;
    int line = 1;
    int x = 10;
    struct dmi_id_info *dmi_info= NULL;

    ret = getInfo();
    if (ret)
        return ret;

    mvwprintw2c(nwin, line++, x, "%-15s: %s", "Distro", pretty_name);
    mvwprintw2c(nwin, line++, x, "%-15s: %s", "OS", utsname.sysname);
    mvwprintw2c(nwin, line++, x, "%-15s: %s", "Release", utsname.release);
    mvwprintw2c(nwin, line++, x, "%-15s: %s", "HostName", utsname.nodename);
    mvwprintw2c(nwin, line++, x, "%-15s: %s", "Arch", utsname.machine);
    for (int i = 0; i < DMI_ID_INFO_COUNT; i++) {
        dmi_info = &id_infos[i];
        mvwprintw2c(nwin, line++, x, "%-15s: %s", dmi_info->label, dmi_info->value);
    }

    wrefresh(nwin);
    return ret;
}

static int tabSystemInfoExit(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

static int tabSystemInfoUpdate(struct TabInfo *info, struct Window *win)
{
    int ret = 0;
    return ret;
}

struct TabInfo systemInfo = {
    .name = "systemInfo",
    .labelName = "System",
    .init = tabSystemInfoInit,
    .exit = tabSystemInfoExit,
    .update = tabSystemInfoUpdate,
};

