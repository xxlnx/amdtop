#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>

#define _LOG(prefix, fmt, ...) \
    fprintf(stdout, prefix fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) \
    _LOG("[INFO]:", fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) \
    _LOG("[WARN]:", fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR]:" fmt, ##__VA_ARGS__)

#ifdef __DEBUG__
#define DEBUG_TRACE() \
    _LOG("[TRACE]:", "%s:%d ret = %d\n", __func__, __LINE__, ret)
#define DEBUG(fmt, ...) \
    _LOG("[DEBUG]:", "%s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_TRACE()
#define DEBUG(...)
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void* xAlloc(size_t size);
void *xFree(void *ptr);
void MemClear(void *buf, size_t size);


#endif
