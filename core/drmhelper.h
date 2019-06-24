#ifndef __DRMHELPER_H__
#define __DRMHELPER_H__

#include "libdrm/drm.h"
#include "libdrm/amdgpu_drm.h"

int gpuIoctl(int fd, unsigned long request, void *arg);

#endif
