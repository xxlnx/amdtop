#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include "utils.h"


void* xAlloc(size_t size)
{
    return calloc(1, size);
}


void *xFree(void *ptr)
{
    free(ptr);
}


void MemClear(void *buf, size_t size)
{
    memset(buf, 0, size);
}

#define PROC_MEMINFO_PATH   "/proc/meminfo"
uint32_t getProcMemInfo(const char *name)
{
    uint32_t value = 0;
    char *p;
    char buf[100] = {0};
    FILE *fp = fopen(PROC_MEMINFO_PATH, "r");
    if (!fp)
        return 0;

    while (!feof(fp) && fgets(buf, 100, fp)) {
        p = strstr(buf, name);
        if (!p) continue;
        p = p + strlen(name);
        value = atoi(p + 1);
    }

    fclose(fp);
    return value;
}

uint32_t getTotalMem(void)
{
    return getProcMemInfo("MemTotal");
}

uint32_t getFreeMem(void)
{
    return getProcMemInfo("MemAvailable");
}

uint64_t getcurrent_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t )((uint64_t )ts.tv_sec * 1000 * 1000 * 1000 + (uint64_t )ts.tv_nsec);
}

#define CPU_INFO_PATH   "/proc/cpuinfo"

int parseCpuInfo(char *item, char *name)
{
    int ret = 0;
    FILE *fp = NULL;
    char buf[1024];
    char value[1024];
    size_t size = 0;
    char *p = NULL, *q = NULL;
    uint32_t item_len = 0;

    fp = fopen(CPU_INFO_PATH, "r");
    if (!fp)
        return -EINVAL;

    item_len = strlen(item);

    while(!feof(fp)) {
        fgets(buf, 1024, fp);
        if (!strncmp(buf, item, item_len)) {
            p = strchr(buf, ':');
            if (p) {
                p = p + 2; /* item\t : value'0a' */
                q = p;
                while(*q != 0x0a) q++;
                *q = '\0';
                strncpy(name, p, q - p);
                return 0;
            }
        }
    }

    fclose(fp);

    return -EINVAL;
}

int getCpuName(char *name)
{
    int ret = 0;

    ret = parseCpuInfo("model name", name);

    return ret;
}

int getCpuCores(uint32_t *hw_core, uint32_t *threads)
{
    int ret = 0;
    char value[1024];

    if (hw_core) {
        ret = parseCpuInfo("cpu cores", value);
        if (ret)
            return ret;
        *hw_core = atoi(value);
    }

    if (threads) {
        ret = parseCpuInfo("siblings", value);
        if (ret)
            return ret;
        *threads = atoi(value);
    }

    return ret;
}

