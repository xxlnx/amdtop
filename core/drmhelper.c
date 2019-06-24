#include "drmhelper.h"
#include <errno.h>
#include <sys/ioctl.h>

int gpuIoctl(int fd, unsigned long request, void *arg)
{
    int ret ;
    do {
        ret = ioctl(fd, request, arg);
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));
    return ret;
}

