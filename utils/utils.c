#include <malloc.h>

void* xAlloc(size_t size)
{
    return calloc(1, size);
}


void *xFree(void *ptr)
{
    free(ptr);
}

