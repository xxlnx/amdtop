#include <malloc.h>
#include <string.h>

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
