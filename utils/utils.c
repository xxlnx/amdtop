#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


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
