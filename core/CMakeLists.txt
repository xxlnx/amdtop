include_directories(../utils)
set(libsrc
    gpudevice.c
    drmhelper.c
    gpuinfo.c)
add_library(gpu SHARED ${libsrc})
target_link_libraries(gpu
    utils)

add_subdirectory(test)